#ifndef __FLOGFS_LINUX_MMAP_H_
#define __FLOGFS_LINUX_MMAP_H_

#include <flogfs.h>

#include "debug_log.h"

extern "C" {

#include "flogfs_conf_implement.h"

flog_result_t flogfs_linux_open(const char *path, bool truncate, flog_initialize_params_t *params);

flog_result_t flogfs_linux_close();

Log &flogfs_linux_get_log();

}

#endif
