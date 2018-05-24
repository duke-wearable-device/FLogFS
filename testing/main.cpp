#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>

#include <flogfs.h>

#include "flogfs_linux.h"

void flog_check(flog_result_t fr) {
    if (fr != FLOG_SUCCESS) {
        fprintf(stderr, "Failed\n");
        exit(2);
    }
}

void write_file(const char *path) {
    if (!flogfs_check_exists(path)) {
        flog_write_file_t file;
        std::cout << "Creating " << path << std::endl;
        flog_check(flogfs_open_write(&file, path));
        std::cout << "Writing " << path << std::endl;
        for (auto i = 0; i < 128 * 3; ++i) {
            flogfs_write(&file, (uint8_t *)"Jacob", 5);
        }
        std::cout << "Closing " << path << std::endl;
        flog_check(flogfs_close_write(&file));
    }
}

int32_t main(int argc, char *argv[]) {
    std::cout << "Starting" << std::endl;

    flog_check(flogfs_linux_open());

    flog_check(flogfs_init());

    std::cout << "Formatting" << std::endl;

    flog_check(flogfs_format());

    std::cout << "Mounting" << std::endl;

    flog_check(flogfs_mount());

    write_file("data-1.bin");
    write_file("data-2.bin");
    write_file("data-3.bin");

    {
        flogfs_ls_iterator_t iter;
        char fname[256];

        std::cout << "Listing: " << std::endl;

        flogfs_start_ls(&iter);

        while (flogfs_ls_iterate(&iter, fname)) {
            std::cout << "LS: '" << fname << "'" << std::endl;
        }

        flogfs_stop_ls(&iter);
    }

    std::cout << "Done" << std::endl;

    flog_check(flogfs_linux_close());

    return 0;
}
