#include <iostream>

#include <flogfs.h>
#include <flogfs_linux_mmap.h>

#include <gtest/gtest.h>

#include "utilities.h"

constexpr const char *Pattern = "abcdefgh";

bool initialize_and_open(bool truncate) {
    flog_initialize_params_t params { 32, 16 };
    EXPECT_TRUE(flogfs_linux_open("tests.bin", truncate, &params));
    EXPECT_TRUE(flogfs_initialize(&params));
    if (truncate) {
        EXPECT_TRUE(flogfs_format());
    }
    EXPECT_TRUE(flogfs_mount());

    std::cout << "Theoretical capacity: " << (params.number_of_blocks - 1) * params.pages_per_block * FS_SECTORS_PER_PAGE * FS_SECTOR_SIZE << std::endl;

    return true;
}

std::vector<std::string> generate_random_file_names(int32_t number) {
    std::vector<std::string> names;
    for (auto i = 0; i < number; ++i) {
        char file_name[32];
        snprintf(file_name, sizeof(file_name), "file-%02d.bin", i);
        names.emplace_back(file_name);
    }
    return names;
}

std::vector<std::string> get_file_listing() {
    std::vector<std::string> names;
    flogfs_ls_iterator_t iter;
    char file_name[32];
    flogfs_start_ls(&iter);
    while (flogfs_ls_iterate(&iter, file_name)) {
        names.emplace_back(file_name);
    }
    flogfs_stop_ls(&iter);
    return names;
}

std::vector<GeneratedFile> write_files_randomly(std::vector<std::string> &names, uint8_t number_of_iterations, uint32_t min_size, uint32_t max_size) {
    std::vector<GeneratedFile> files(names.size());
    for (auto i = 0; i < names.size(); ++i) {
        files[i].name = names[i];
    }

    for (auto j = 0; j < number_of_iterations; ++j) {
        auto total_written = 0;

        if (j > 0) {
            for (auto &file : files) {
                EXPECT_TRUE(flogfs_rm(file.name.c_str()));
            }
        }

        for (auto &file : files) {
            file.size = (random() % (max_size - min_size)) + min_size;
            file.written = 0;

            EXPECT_TRUE(flogfs_open_write(&file.file, file.name.c_str()));
        }

        while (true) {
            auto done = true;

            for (auto &file : files) {
                auto remaining = file.size - file.written;
                if (remaining > 0) {
                    auto random_block_size = (uint32_t)(random() % 4096) + 256;
                    auto to_write = std::min(random_block_size, (uint32_t)remaining);
                    while (to_write > 0) {
                        auto writing = std::min((uint32_t)to_write, (uint32_t)strlen(Pattern));
                        auto wrote = flogfs_write(&file.file, (uint8_t *)Pattern, writing);
                        if (wrote != writing) {
                            assert(wrote == writing);
                        }
                        to_write -= wrote;
                        file.written += wrote;
                        total_written += wrote;
                        done = false;
                    }
                }
            }

            if (done) {
                break;
            }
        }

        for (auto &file : files) {
            EXPECT_TRUE(flogfs_close_write(&file.file));
        }

        std::cout << "Total Written: " << total_written << " bytes" << std::endl;
    }

    return files;
}
