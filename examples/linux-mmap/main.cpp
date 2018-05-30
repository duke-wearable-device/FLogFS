#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <iostream>
#include <algorithm>
#include <array>
#include <vector>
#include <string>

#include <flogfs.h>
#include <flogfs_linux_mmap.h>

void flog_assertion_fail(const char *expression, const char *file, uint32_t line) {
    fprintf(stderr, "Failed: %s:%d : %s\n", file, line, expression);
    abort();
}

#define FLOG_CHECK(EX) (void)((EX) || (flog_assertion_fail(#EX, __FILE__, __LINE__), 0))

int32_t main(int argc, char *argv[]) {
    std::string path;
    bool truncate = false;
    flog_initialize_params_t params { 256, 64 };

    for (auto i = 1; i < argc; ++i) {
        auto arg = std::string{ argv[i] };
        if (arg == "--truncate") {
            truncate = true;
        }
        else {
            path = arg;
        }
    }

    if (path.size() == 0) {
        fprintf(stderr, "Usage: %s [--truncate] <path>\n", argv[0]);
        exit(2);
    }

    srand(1);

    FLOG_CHECK(flogfs_linux_open(path.c_str(), truncate, &params));

    FLOG_CHECK(flogfs_initialize(&params));

    std::cout << "Mounting " << path << std::endl;
    if (flogfs_mount() == FLOG_FAILURE) {
        std::cout << "Formatting" << std::endl;
        FLOG_CHECK(flogfs_format());
        std::cout << "Mounting" << std::endl;
        FLOG_CHECK(flogfs_mount());
    }

    FLOG_CHECK(flogfs_linux_close());

    return 0;
}
