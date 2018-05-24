#include <cassert>
#include <cstdio>

#include "flogfs.h"
#include "flogfs_linux.h"

typedef void *fs_lock_t;

#define fslog_linux_trace(f, ...)

#define fslog_linux_debug(f, ...) // printf("flogfs: " f, ##__VA_ARGS__)

static inline void memzero(void *ptr, size_t size) {
    memset(ptr, 0, size);
}

static inline void fs_lock_init(fs_lock_t *lock) {
    fslog_linux_trace("fs_lock_init\n");
}

static inline void fs_lock(fs_lock_t *lock) {
    fslog_linux_trace("fs_lock\n");
}

static inline void fs_unlock(fs_lock_t *lock) {
    fslog_linux_trace("fs_unlock\n");
}

static void *mapped{ nullptr };
static uint16_t flash_block{ 0 };
static uint16_t flash_page{ 0 };

static inline flog_result_t flash_init() {
    mapped = flogfs_linux_get();

    return FLOG_RESULT(mapped != nullptr);
}

static inline uint32_t get_offset(uint16_t block, uint16_t page, uint8_t sector, uint16_t offset) {
    return (block * FS_SECTORS_PER_BLOCK_INTERNAL * FS_SECTOR_SIZE) +
           (page * FS_SECTORS_PER_PAGE_INTERNAL * FS_SECTOR_SIZE) +
           (sector * FS_SECTOR_SIZE) + offset;
}

static inline uint32_t get_offset_in_block(uint8_t sector, uint16_t offset) {
    return get_offset(flash_block, flash_page, sector, offset);
}

static inline void *mapped_sector_absolute_ptr(uint16_t block, uint16_t page, uint8_t sector, uint16_t offset) {
    return ((uint8_t *)mapped) + get_offset(block, page, sector, offset);
}

static inline void *mapped_sector_ptr(uint8_t sector, uint16_t offset) {
    return mapped_sector_absolute_ptr(flash_block, flash_page, sector, offset);
}

static inline void flash_lock() {
}

static inline void flash_unlock() {
}

static inline flog_result_t flash_open_page(uint16_t block, uint16_t page) {
    flash_block = block;
    flash_page = page;
    fslog_linux_debug("flash_open_page(%d, %d)\n", block, page);
    return FLOG_RESULT(FLOG_SUCCESS);
}

static inline void flash_close_page() {
    fslog_linux_debug("flash_close_page\n");
}

static inline flog_result_t flash_erase_block(uint16_t block) {
    fslog_linux_debug("flash_erase_block(%d)\n", block);
    memset(mapped_sector_absolute_ptr(block, 0, 0, 0), 0xff, FS_SECTORS_PER_BLOCK_INTERNAL * FS_SECTOR_SIZE);
    return FLOG_RESULT(FLOG_SUCCESS);
}

static inline flog_result_t flash_block_is_bad() {
    fslog_linux_trace("flash_block_is_bad\n");
    return FLOG_RESULT(0);
}

static inline void flash_set_bad_block() {
    fslog_linux_trace("flash_set_bad_block\n");
}

static inline void flash_commit() {
    fslog_linux_trace("flash_commit\n");
}

static inline bool verified_memcpy(void *dst, const void *src, size_t size) {
    auto unerased = false;
    for (auto i = 0; i < size; ++i) {
        if (((uint8_t *)dst)[i] != 0xff) {
            unerased = true;
            fprintf(stderr, "UNERASED: %p (%ld)\n", (uint8_t *)dst + i, size);
            assert(false);
            break;
        }
    }
    memcpy(dst, src, size);
    return !unerased;
}

static inline flog_result_t flash_read_sector(uint8_t *dst, uint8_t sector, uint16_t offset, uint16_t n) {
    fslog_linux_debug("flash_read_sector(%p, %d, %d, %d)\n", dst, sector, offset, n);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto src = mapped_sector_ptr(sector, offset);
    memcpy(dst, src, n);
    return FLOG_SUCCESS;
}

static inline flog_result_t flash_read_spare(uint8_t *dst, uint8_t sector) {
    fslog_linux_debug("flash_read_spare(%p, %d)\n", dst, sector);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto src = mapped_sector_ptr(0, 0x804 + sector * 0x10);
    memcpy(dst, src, sizeof(flog_file_sector_spare_t));
    return FLOG_SUCCESS;
}

static inline void flash_write_sector(uint8_t const *src, uint8_t sector, uint16_t offset, uint16_t n) {
    fslog_linux_debug("flash_write_sector(%p, %d, %d, %d)\n", src, sector, offset, n);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto dst = mapped_sector_ptr(sector, offset);
    verified_memcpy(dst, src, n);
}

static inline void flash_write_spare(uint8_t const *src, uint8_t sector) {
    fslog_linux_debug("flash_write_spare(%p, %d)\n", src, sector);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto dst = mapped_sector_ptr(0, 0x804 + sector * 0x10);
    verified_memcpy(dst, src, sizeof(flog_file_sector_spare_t));
}

static inline void flash_debug_warn(char const *msg) {
    fslog_linux_debug("debug: %s\n", msg);
}

static inline void flash_debug_error(char const *msg) {
    fslog_linux_debug("error: %s\n", msg);
}
