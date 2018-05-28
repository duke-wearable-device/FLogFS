#include <cstring>
#include <cassert>
#include <cstdarg>
#include <cstdio>

#include <flogfs.h>
#include <flogfs_private.h>

#include "sd_raw.h"

#include "flogfs_arduino_sd.h"

extern "C" void debugfln(const char *f, ...);

static sd_raw_t sd;
static uint16_t open_block{ 0 };
static uint16_t open_page{ 0 };

#define fslog_trace(f, ...) flash_debug_warn(f, ##__VA_ARGS__)

#define fslog_debug(f, ...) flash_debug_warn(f, ##__VA_ARGS__)

static inline uint32_t get_sd_block(uint16_t block, uint16_t page, uint8_t sector) {
    return (block * FS_SECTORS_PER_BLOCK_INTERNAL) +
           (page * FS_SECTORS_PER_PAGE_INTERNAL) +
           (sector);
}

static inline uint32_t get_sd_block_in_open_page(uint8_t sector) {
    return get_sd_block(open_block, open_page, sector);
}

flog_result_t flogfs_arduino_sd_open(uint8_t cs) {
    return FLOG_RESULT(sd_raw_initialize(&sd, cs));
}

flog_result_t flogfs_arduino_sd_close() {
    return FLOG_FAILURE;
}

void fs_lock_init(fs_lock_t *lock) {
}

void fs_lock(fs_lock_t *lock) {
}

void fs_unlock(fs_lock_t *lock) {
}

flog_result_t flash_init() {
    return FLOG_SUCCESS;
}

void flash_lock() {
}

void flash_unlock() {
}

flog_result_t flash_open_page(uint16_t block, uint16_t page) {
    fslog_debug("flash_open_page(%d, %d)", block, page);
    open_block = block;
    open_page = page;
    return FLOG_SUCCESS;
}

void flash_close_page() {
    fslog_debug("flash_close_page\n");
}

flog_result_t flash_erase_block(uint16_t block) {
    auto first_sd_block = get_sd_block(block, 0, 0);
    auto last_sd_block = get_sd_block(block + 1, 0, 0);
    fslog_debug("flash_erase_block(%d, %d -> %d)", block, first_sd_block, last_sd_block);
    return FLOG_RESULT(sd_raw_erase(&sd, first_sd_block, last_sd_block));
}

flog_result_t flash_block_is_bad() {
    return FLOG_RESULT(0);
}

void flash_set_bad_block() {
}

void flash_commit() {
}

flog_result_t flash_read_sector(uint8_t *dst, uint8_t sector, uint16_t offset, uint16_t n) {
    fslog_debug("flash_read_sector(%p, %d, %d, %d)", dst, sector, offset, n);
    sector = sector % FS_SECTORS_PER_PAGE;
    return FLOG_RESULT(sd_raw_read_data(&sd, get_sd_block_in_open_page(sector), offset, n, dst));
}

flog_result_t flash_read_spare(uint8_t *dst, uint8_t sector) {
    fslog_debug("flash_read_spare(%p, %d)", dst, sector);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto sd_block = get_sd_block_in_open_page(FS_SECTORS_PER_PAGE);
    return FLOG_RESULT(sd_raw_read_data(&sd, sd_block, sector * 0x10, sizeof(flog_file_sector_spare_t), dst));
}

void flash_write_sector(uint8_t const *src, uint8_t sector, uint16_t offset, uint16_t n) {
    fslog_debug("flash_write_sector(%p, %d, %d, %d)", src, sector, offset, n);
    sector = sector % FS_SECTORS_PER_PAGE;
    sd_raw_write_data(&sd, get_sd_block_in_open_page(sector), offset, n, src);
}

void flash_write_spare(uint8_t const *src, uint8_t sector) {
    fslog_debug("flash_write_spare(%p, %d)", src, sector);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto sd_block = get_sd_block_in_open_page(FS_SECTORS_PER_PAGE);
    sd_raw_write_data(&sd, sd_block, sector * 0x10, sizeof(flog_file_sector_spare_t), src);
}

constexpr uint16_t DebugLineMax = 256;

void flash_debug(const char *f, va_list args) {
    char buffer[DebugLineMax];
    auto w = vsnprintf(buffer, DebugLineMax, f, args);
    debugfln("%s", buffer);
}

void flash_debug_warn(char const *f, ...) {
    #ifdef FLOGFS_VERBOSE_LOGGING
    va_list args;
    va_start(args, f);
    flash_debug(f, args);
    va_end(args);
    #endif
}

void flash_debug_error(char const *f, ...) {
    #ifdef FLOGFS_VERBOSE_LOGGING
    va_list args;
    va_start(args, f);
    flash_debug(f, args);
    va_end(args);
    #endif
}
