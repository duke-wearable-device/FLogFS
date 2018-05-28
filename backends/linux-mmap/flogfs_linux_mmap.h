#ifndef __FLOGFS_LINUX_MMAP_H_
#define __FLOGFS_LINUX_MMAP_H_

#include <flogfs.h>

extern "C" {

#include "flogfs_conf_implement.h"

flog_result_t flogfs_linux_open(const char *path, bool truncate, flog_init_params_t *params);

flog_result_t flogfs_linux_close();

}

#endif
