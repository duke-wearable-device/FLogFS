#ifndef __FLOGFS_LINUX_MMAP_H_
#define __FLOGFS_LINUX_MMAP_H_

#include <flogfs.h>

extern "C" {

#include "flogfs_conf_implement.h"

constexpr uint32_t FS_SECTORS_PER_PAGE_INTERNAL = (FS_SECTORS_PER_PAGE + 1);
constexpr uint32_t FS_SECTORS_PER_BLOCK_INTERNAL = FS_SECTORS_PER_PAGE_INTERNAL * FS_PAGES_PER_BLOCK;
constexpr uint32_t MappedSize = FS_SECTOR_SIZE * FS_SECTORS_PER_BLOCK_INTERNAL * FS_NUM_BLOCKS;

flog_result_t flogfs_linux_open(const char *path);

flog_result_t flogfs_linux_close();

}

#endif
