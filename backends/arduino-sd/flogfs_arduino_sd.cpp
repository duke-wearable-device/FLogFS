#include <cstring>
#include <cassert>

#include <flogfs.h>
#include <flogfs_private.h>

#include "sd_raw.h"

#include "flogfs_arduino_sd.h"

static sd_raw_t sd;
static uint16_t open_block{ 0 };
static uint16_t open_page{ 0 };

#define fslog_trace(f, ...)

#define fslog_debug(f, ...) // printf("flogfs: " f, ##__VA_ARGS__)

static inline uint32_t get_sd_block(uint16_t block, uint16_t page, uint8_t sector) {
    return (block * FS_SECTORS_PER_BLOCK_INTERNAL * FS_SECTOR_SIZE) +
           (page * FS_SECTORS_PER_PAGE_INTERNAL * FS_SECTOR_SIZE) +
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
    return FLOG_FAILURE;
}

void flash_lock() {
}

void flash_unlock() {
}

flog_result_t flash_open_page(uint16_t block, uint16_t page) {
    fslog_debug("flash_open_page(%d, %d)\n", block, page);
    open_block = block;
    open_page = page;
    return FLOG_SUCCESS;
}

void flash_close_page() {
    fslog_debug("flash_close_page\n");
}

flog_result_t flash_erase_block(uint16_t block) {
    fslog_debug("flash_erase_block(%d)\n", block);
    auto first_sd_block = get_sd_block(block, 0, 0);
    auto last_sd_block = get_sd_block(block + 1, 0, 0);
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
    fslog_debug("flash_read_sector(%p, %d, %d, %d)\n", dst, sector, offset, n);
    sector = sector % FS_SECTORS_PER_PAGE;
    sd_raw_read_block(&sd, get_sd_block_in_open_page(sector), dst);
    return FLOG_SUCCESS;
}

flog_result_t flash_read_spare(uint8_t *dst, uint8_t sector) {
    fslog_debug("flash_read_spare(%p, %d)\n", dst, sector);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto sd_block = get_sd_block_in_open_page(FS_SECTORS_PER_PAGE);
    return FLOG_SUCCESS;
}

void flash_write_sector(uint8_t const *src, uint8_t sector, uint16_t offset, uint16_t n) {
    fslog_debug("flash_write_sector(%p, %d, %d, %d)\n", src, sector, offset, n);
    sector = sector % FS_SECTORS_PER_PAGE;
    sd_raw_write_block(&sd, get_sd_block_in_open_page(sector), src);
}

void flash_write_spare(uint8_t const *src, uint8_t sector) {
    fslog_debug("flash_write_spare(%p, %d)\n", src, sector);
    sector = sector % FS_SECTORS_PER_PAGE;
    auto sd_block = get_sd_block_in_open_page(FS_SECTORS_PER_PAGE);
}

void flash_debug_warn(char const *msg) {
    fslog_debug("debug: %s\n", msg);
}

void flash_debug_error(char const *msg) {
    fslog_debug("error: %s\n", msg);
}
