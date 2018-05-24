#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include <flogfs.h>
#include <flogfs_private.h>

#include "flogfs_linux.h"

void flog_check(flog_result_t fr) {
    if (fr != FLOG_SUCCESS) {
        fprintf(stderr, "Failed\n");
        exit(2);
    }
}

constexpr char *Pattern = "abcdefgh";

size_t write_file(const char *path) {
    if (!flogfs_check_exists(path)) {
        size_t size = 0;
        flog_write_file_t file;
        std::cout << "Creating " << path << std::endl;
        flog_check(flogfs_open_write(&file, path));
        std::cout << "Writing " << path << std::endl;
        // for (auto i = 0; i < 1024 * 26; ++i) {
        while (size < 4*512*64) {
            size += flogfs_write(&file, (uint8_t *)Pattern, strlen(Pattern));
        }
        if (size != file.file_size) {
            std::cout << "Size is wrong! " << size << " != " << file.file_size << std::endl;
        }
        std::cout << "Closing " << path << " size=" << file.file_size << " block=" << file.block << " sector=" << file.sector << std::endl;
        flog_check(flogfs_close_write(&file));

        return size;
    }

    return 0;
}

void reopen_file(const char *path, size_t expected_size) {
    flog_write_file_t file;
    flog_check(flogfs_open_write(&file, path));
    std::cout << "Opened " << path << " size=" << file.file_size << " block=" << file.block << " sector=" << file.sector << std::endl;
    if (expected_size != file.file_size) {
        std::cout << "Size is wrong! (exepcted != actual) " << expected_size << " != " << file.file_size << " diff=" << (file.file_size - expected_size) << std::endl;
    }
    flog_check(flogfs_close_write(&file));
}

void read_file(const char *path, size_t expected_size) {
    flog_read_file_t file;
    std::cout << "Reading " << path << std::endl;
    flog_check(flogfs_open_read(&file, path));
    if (expected_size != file.file_size) {
        std::cout << "Size is wrong! " << expected_size << " != " << file.file_size << " diff=" << (file.file_size - expected_size) << std::endl;
    }

    while (true) {
        uint8_t buffer[strlen(Pattern) + 1];
        if (flogfs_read(&file, buffer, sizeof(buffer) - 1) != sizeof(buffer) - 1) {
            break;
        }

        buffer[sizeof(buffer) - 1] = 0;

        if (strcmp((const char *)buffer, Pattern) != 0) {
            std::cout << buffer << std::endl;
        }
    }

    flog_check(flogfs_close_read(&file));
}

int32_t main(int argc, char *argv[]) {
    flog_check(flogfs_linux_open());

    flog_check(flogfs_init());

    flog_check(flogfs_format());

    flog_check(flogfs_mount());

    auto size1 = write_file("data-1.bin");
    auto size2 = write_file("data-2.bin");
    auto size3 = write_file("data-3.bin");

    read_file("data-1.bin", size1);
    read_file("data-2.bin", size2);
    read_file("data-3.bin", size3);

    reopen_file("data-1.bin", size1);
    reopen_file("data-2.bin", size2);
    reopen_file("data-3.bin", size3);

    {
        flogfs_ls_iterator_t iter;
        char fname[256];

        flogfs_start_ls(&iter);

        while (flogfs_ls_iterate(&iter, fname)) {
            std::cout << "LS: '" << fname << "'" << std::endl;
        }

        flogfs_stop_ls(&iter);
    }

    flog_check(flogfs_linux_close());

    return 0;
}
