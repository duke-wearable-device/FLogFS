#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cassert>

#include <flogfs.h>
#include <flogfs_private.h>

#include "flogfs_linux_mmap.h"

#ifdef FLOGFS_VERBOSE_LOGGING
#define fslog_trace(f, ...) flash_debug_warn(f, ##__VA_ARGS__)
#else
#define fslog_trace(f, ...)
#endif

constexpr uint32_t FS_SECTORS_PER_PAGE_INTERNAL = (FS_SECTORS_PER_PAGE + 1);

static void *mapped{ nullptr };
static int32_t fd{ -1 };
static uint32_t mapped_size{ 0 };
static flog_block_idx_t open_block = FLOG_BLOCK_IDX_INVALID;
static flog_page_index_t open_page{ 0 };
static uint16_t pages_per_block{ 0 };
static Log log;

static inline uint32_t sectors_per_block() {
    return FS_SECTORS_PER_PAGE_INTERNAL * pages_per_block;
}

flog_result_t flogfs_linux_open(const char *path, bool truncate, flog_initialize_params_t *params) {
    assert(fd == -1);
    assert(mapped == nullptr);

    fd = open(path, O_RDWR | O_CREAT | (truncate ? O_TRUNC : 0), 0644);
    if (fd < 0) {
        return FLOG_FAILURE;
    }

    pages_per_block = params->pages_per_block;

    mapped_size = FS_SECTOR_SIZE * sectors_per_block() * params->number_of_blocks;
    if (lseek(fd, mapped_size, SEEK_SET) == -1) {
        return FLOG_FAILURE;
    }

    if (write(fd, "", 1) == -1) {
        return FLOG_FAILURE;
    }

    mapped = mmap(nullptr, mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        mapped = nullptr;
        return FLOG_FAILURE;
    }

    log.clear();
    log.append(LogEntry{ OperationType::Opened });

    return FLOG_SUCCESS;
}

Log &flogfs_linux_get_log() {
    return log;
}

flog_result_t flogfs_linux_close() {
    if (mapped != nullptr) {
        munmap(mapped, mapped_size);
        mapped = nullptr;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }

    return FLOG_SUCCESS;
}

static inline uint32_t get_offset(flog_block_idx_t block, flog_page_index_t page, flog_sector_idx_t sector, uint16_t offset) {
    return (block * sectors_per_block() * FS_SECTOR_SIZE) +
        (page * FS_SECTORS_PER_PAGE_INTERNAL * FS_SECTOR_SIZE) +
        (sector * FS_SECTOR_SIZE) + offset;
}

static inline uint32_t get_offset_in_block(flog_sector_idx_t sector, uint16_t offset) {
    return get_offset(open_block, open_page, sector, offset);
}

static inline void *mapped_sector_absolute_ptr(flog_block_idx_t block, flog_page_index_t page, flog_sector_idx_t sector, uint16_t offset) {
    return ((uint8_t *)mapped) + get_offset(block, page, sector, offset);
}

static inline void *mapped_sector_ptr(flog_sector_idx_t sector, uint16_t offset) {
    return mapped_sector_absolute_ptr(open_block, open_page, sector, offset);
}

void fs_lock_initialize(fs_lock_t *lock) {
}

void fs_lock(fs_lock_t *lock) {
}

void fs_unlock(fs_lock_t *lock) {
}

flog_result_t flash_initialize() {
    return FLOG_RESULT(mapped != nullptr);
}

void flash_lock() {
}

void flash_unlock() {
}

flog_result_t flash_open_page(flog_block_idx_t block, flog_page_index_t page) {
    // fslog_trace("flash_open_page(%d, %d)", block, page);
    open_block = block;
    open_page = page;
    return FLOG_RESULT(FLOG_SUCCESS);
}

void flash_close_page() {
    // fslog_trace("flash_close_page");
}

flog_result_t flash_erase_block(flog_block_idx_t block) {
    fslog_trace("flash_erase_block(%d)", block);
    log.append(LogEntry{ OperationType::EraseBlock, block });
    memset(mapped_sector_absolute_ptr(block, 0, 0, 0), FS_ERASE_CHAR, sectors_per_block() * FS_SECTOR_SIZE);
    return FLOG_RESULT(FLOG_SUCCESS);
}

flog_result_t flash_block_is_bad() {
    return FLOG_RESULT(0);
}

void flash_set_bad_block() {
}

void flash_commit() {
}

static void verified_memcpy(void *dst, const void *src, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (((uint8_t *)dst)[i] != FS_ERASE_CHAR) {
            printf("Write to un-erased memory: offset=%lu size=%lu (%d / %d)\n", (uint8_t *)dst - (uint8_t *)mapped, size, open_block, open_page);
            assert(false);
        }
    }
    memcpy(dst, src, size);
}

flog_result_t flash_read_sector(uint8_t *dst, flog_sector_idx_t sector, uint16_t offset, uint16_t n) {
    fslog_trace("flash_read_sector(%d/%d, %d, %d, %d)", open_block, open_page, sector, offset, n);
    auto src = mapped_sector_ptr(sector % FS_SECTORS_PER_PAGE, offset);
    memcpy(dst, src, n);
    return FLOG_SUCCESS;
}

flog_result_t flash_read_spare(uint8_t *dst, flog_sector_idx_t sector) {
    fslog_trace("flash_read_spare(%d/%d, %d)", open_block, open_page, sector);
    auto src = mapped_sector_ptr(0, 0x804 + (sector % FS_SECTORS_PER_PAGE) * 0x10);
    memcpy(dst, src, sizeof(flog_file_sector_spare_t));
    return FLOG_SUCCESS;
}

void flash_write_sector(uint8_t const *src, flog_sector_idx_t sector, uint16_t offset, uint16_t n) {
    fslog_trace("flash_write_sector(%d/%d, %d, %d, %d)", open_block, open_page, sector, offset, n);
    auto dst = mapped_sector_ptr(sector % FS_SECTORS_PER_PAGE, offset);
    log.append(LogEntry{ OperationType::WriteSector, open_block, open_page, sector, dst, offset, n });
    verified_memcpy(dst, src, n);
}

void flash_write_spare(uint8_t const *src, flog_sector_idx_t sector) {
    fslog_trace("flash_write_spare(%d/%d, %d)", open_block, open_page, sector);
    auto dst = mapped_sector_ptr(0, 0x804 + (sector % FS_SECTORS_PER_PAGE) * 0x10);
    log.append(LogEntry{ OperationType::WriteSpare, open_block, open_page, sector, dst });
    verified_memcpy(dst, src, sizeof(flog_file_sector_spare_t));
}

uint32_t flash_random(uint32_t max) {
    return random() % max;
}

void flash_high_level(flog_high_level_event_t hle) {
    switch (hle) {
    case FLOG_PRIME_BEGIN:
        log.append(LogEntry{ OperationType::PrimeBegin });
        break;
    case FLOG_PRIME_END:
        log.append(LogEntry{ OperationType::PrimeEnd });
        break;
    case FLOG_FORMAT_BEGIN:
        log.append(LogEntry{ OperationType::FormatBegin });
        break;
    case FLOG_FORMAT_END: {
        log.append(LogEntry{ OperationType::FormatEnd });
        break;
    }
    }
}

void flash_debug(const char *f, va_list args) {
    constexpr uint16_t DebugLineMax = 256;
    char buffer[DebugLineMax];
    auto w = vsnprintf(buffer, DebugLineMax, f, args);
    buffer[w] = '\r';
    buffer[w + 1] = '\n';
    buffer[w + 2] = 0;
    printf("%s", buffer);
}

void flash_debug_warn(char const *f, ...) {
    #ifdef FLOGFS_DEBUG
    va_list args;
    va_start(args, f);
    flash_debug(f, args);
    va_end(args);
    #endif
}

void flash_debug_error(char const *f, ...) {
    va_list args;
    va_start(args, f);
    flash_debug(f, args);
    va_end(args);
}

void flash_debug_panic() {
    exit(2);
}
