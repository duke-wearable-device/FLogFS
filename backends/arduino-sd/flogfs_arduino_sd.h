#ifndef __FLOGFS_ARDUINO_SD_H_
#define __FLOGFS_ARDUINO_SD_H_

#include <flogfs.h>

extern "C" {

#include "flogfs_conf_implement.h"

constexpr uint32_t FS_SECTORS_PER_PAGE_INTERNAL = (FS_SECTORS_PER_PAGE + 1);
constexpr uint32_t FS_SECTORS_PER_BLOCK_INTERNAL = FS_SECTORS_PER_PAGE_INTERNAL * FS_PAGES_PER_BLOCK;

flog_result_t flogfs_arduino_sd_open(uint8_t cs, flog_init_params_t *params);

flog_result_t flogfs_arduino_sd_close();

}

#endif
