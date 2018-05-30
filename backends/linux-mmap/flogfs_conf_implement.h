#ifndef __FLOGFS_LINUX_MMAP_IMPLEMENT_H_
#define __FLOGFS_LINUX_MMAP_IMPLEMENT_H_

typedef void *fs_lock_t;

void fs_lock_initialize(fs_lock_t *lock);

void fs_lock(fs_lock_t *lock);

void fs_unlock(fs_lock_t *lock);

flog_result_t flash_initialize();

void flash_lock();

void flash_unlock();

flog_result_t flash_open_page(flog_block_idx_t block, flog_page_index_t page);

void flash_close_page();

flog_result_t flash_erase_block(flog_block_idx_t block);

flog_result_t flash_block_is_bad();

void flash_set_bad_block();

void flash_commit();

flog_result_t flash_read_sector(uint8_t *dst, flog_sector_idx_t sector, uint16_t offset, uint16_t n);

flog_result_t flash_read_spare(uint8_t *dst, flog_sector_idx_t sector);

void flash_write_sector(uint8_t const *src, flog_sector_idx_t sector, uint16_t offset, uint16_t n);

void flash_write_spare(uint8_t const *src, flog_sector_idx_t sector);

void flash_debug_warn(char const *f, ...);

void flash_debug_error(char const *f, ...);

void flash_debug_panic();

uint32_t flash_random(uint32_t max);

void flash_high_level(flog_high_level_event_t hle);

#endif
