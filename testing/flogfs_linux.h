#pragma once

#include <cstdio>

#include <flogfs.h>

extern "C" {

flog_result_t flogfs_linux_open();

void *flogfs_linux_get();

flog_result_t flogfs_linux_close();
}
