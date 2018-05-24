#pragma once

#include <cstdio>

#include <flogfs.h>

constexpr uint32_t FS_SECTORS_PER_PAGE_INTERNAL = (FS_SECTORS_PER_PAGE + 1);
constexpr uint32_t FS_SECTORS_PER_BLOCK_INTERNAL = FS_SECTORS_PER_PAGE_INTERNAL * FS_PAGES_PER_BLOCK;
constexpr uint32_t MappedSize = FS_SECTOR_SIZE * FS_SECTORS_PER_BLOCK_INTERNAL * FS_NUM_BLOCKS;

extern "C" {

flog_result_t flogfs_linux_open();

void *flogfs_linux_get();

flog_result_t flogfs_linux_close();
}
