#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "flogfs_linux.h"

static const char *path = "flash.bin";
static void *mapped{ nullptr };
static int32_t fd{ -1 };

flog_result_t flogfs_linux_open() {
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return FLOG_FAILURE;
    }

    if (lseek(fd, MappedSize, SEEK_SET) == -1) {
        return FLOG_FAILURE;
    }

    if (write(fd, "", 1) == -1) {
        return FLOG_FAILURE;
    }

    mapped = mmap(nullptr, MappedSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        mapped = nullptr;
        return FLOG_FAILURE;
    }

    return FLOG_SUCCESS;
}

void *flogfs_linux_get() {
    return mapped;
}

flog_result_t flogfs_linux_close() {
    if (mapped != nullptr) {
        munmap(mapped, MappedSize);
        mapped = nullptr;
    }
    if (fd < 0) {
        close(fd);
        fd = -1;
    }

    return FLOG_SUCCESS;
}
