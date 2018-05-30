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

constexpr const char *Pattern = "abcdefgh";
#ifdef FLOG_ERASE_ZERO
constexpr const char *Path = "flash-00.bin";
#else
constexpr const char *Path = "flash-ff.bin";
#endif

void flog_assertion_fail(const char *expression, const char *file, uint32_t line) {
    fprintf(stderr, "Failed: %s:%d : %s\n", file, line, expression);
    exit(2);
}

#define FLOG_CHECK(EX) (void)((EX) || (flog_assertion_fail(#EX, __FILE__, __LINE__), 0))

size_t write_file(const char *path) {
    if (flogfs_check_exists(path)) {
        std::cout << "Deleting " << path << std::endl;
        FLOG_CHECK(flogfs_rm(path));
    }

    size_t size = 0;
    flog_write_file_t file;
    std::cout << "Creating " << path << std::endl;
    FLOG_CHECK(flogfs_open_write(&file, path));
    std::cout << "Writing " << path << std::endl;
    for (auto i = 0; i < 256 * 1024; ++i) {
        auto written = flogfs_write(&file, (uint8_t *)Pattern, strlen(Pattern));
        FLOG_CHECK(written == strlen(Pattern));
        size += written;
    }
    if (size != file.file_size) {
        std::cout << "Size is wrong! " << size << " != " << file.file_size << std::endl;
    }
    std::cout << "Closing " << path << " size=" << file.file_size << " block=" << file.block << " sector=" << file.sector << std::endl;
    FLOG_CHECK(flogfs_close_write(&file));

    return size;
}

void reopen_file(const char *path, size_t expected_size) {
    flog_write_file_t file;
    FLOG_CHECK(flogfs_open_write(&file, path));
    std::cout << "Opened " << path << " size=" << file.file_size << " block=" << file.block << " sector=" << file.sector << std::endl;
    if (expected_size != file.file_size) {
        std::cout << "Size is wrong! " << expected_size << " != " << file.file_size << " diff=" << ((int32_t)file.file_size - (int32_t)expected_size) << std::endl;
    }
    FLOG_CHECK(flogfs_close_write(&file));
}

void read_records(flog_read_file_t *file, size_t expected_size) {
    size_t actually_read = 0;

    while (true) {
        uint8_t buffer[strlen(Pattern) + 1];
        uint32_t bytes_read = flogfs_read(file, buffer, sizeof(buffer) - 1);
        if (bytes_read == 0) {
            break;
        }
        actually_read += bytes_read;
        buffer[sizeof(buffer) - 1] = 0;
        if (strcmp((const char *)buffer, Pattern) != 0) {
            std::cout << buffer << std::endl;
        }
    }

    if (actually_read != expected_size) {
        std::cout << "Read failed! " << expected_size << " != " << actually_read << std::endl;
    }
}

void read_file(const char *path, size_t expected_size) {
    flog_read_file_t file;
    std::cout << "Reading " << path << std::endl;
    FLOG_CHECK(flogfs_open_read(&file, path));
    if (expected_size != file.file_size) {
        std::cout << "Size is wrong! " << expected_size << " != " << file.file_size << " diff=" << ((int32_t)file.file_size - (int32_t)expected_size) << std::endl;
    }

    read_records(&file, expected_size);

    FLOG_CHECK(flogfs_close_read(&file));
}

void seek_file(const char *path, size_t expected_size) {
    flog_read_file_t file;
    std::cout << "Opening to test seek " << path << std::endl;
    FLOG_CHECK(flogfs_open_read(&file, path));

    size_t expected_bytes_to_be_read = strlen(Pattern) * 2;

    FLOG_CHECK(flogfs_read_seek(&file, expected_size - expected_bytes_to_be_read));

    read_records(&file, expected_bytes_to_be_read);

    FLOG_CHECK(flogfs_close_read(&file));
}

void ls_files() {
    flogfs_ls_iterator_t iter;
    char fname[256];

    flogfs_start_ls(&iter);

    while (flogfs_ls_iterate(&iter, fname)) {
        std::cout << "LS: '" << fname << "'" << std::endl;
    }

    flogfs_stop_ls(&iter);
}

void generate_random_files(uint8_t number_of_files, uint8_t number_of_iterations, uint32_t min_size, uint32_t max_size) {
    std::vector<flog_write_file_t> files(number_of_files);
    std::vector<size_t> sizes(number_of_files);
    std::vector<size_t> written(number_of_files);
    std::vector<std::string> names(number_of_files);

    for (auto i = 0; i < number_of_files; ++i) {
        char temp[32];
        snprintf(temp, sizeof(temp), "file-%02d.bin", i);
        names[i] = temp;
    }

    for (auto j = 0; j < number_of_iterations; ++j) {
        if (j > 0) {
            for (auto i = 0; i < number_of_files; ++i) {
                FLOG_CHECK(flogfs_rm(names[i].c_str()));
            }
        }

        for (auto i = 0; i < number_of_files; ++i) {
            sizes[i] = (random() % (max_size - min_size)) + min_size;
            written[i] = 0;

            std::cout << "Creating " << names[i] << " " << sizes[i] << std::endl;
            FLOG_CHECK(flogfs_open_write(&files[i], names[i].c_str()));
        }

        std::cout << "Writing" << std::endl;

        while (true) {
            auto done = true;

            for (auto i = 0; i < number_of_files; ++i) {
                auto remaining = sizes[i] - written[i];
                if (remaining > 0) {
                    auto to_write = std::min((uint32_t)(random() % 4096) + 256, (uint32_t)remaining);
                    while (to_write > 0) {
                        auto writing = std::min((uint32_t)to_write, (uint32_t)strlen(Pattern));
                        auto wrote = flogfs_write(&files[i], (uint8_t *)Pattern, writing);
                        if (wrote != writing) {
                            std::cout << "Writing " << names[i] << " " << remaining << " " << files[i].block << " " << writing << std::endl;
                            assert(wrote == writing);
                        }
                        to_write -= wrote;
                        written[i] += wrote;
                        done = false;
                    }
                }
            }

            if (done) {
                break;
            }
        }

        for (auto i = 0; i < number_of_files; ++i) {
            std::cout << "Closing " << names[i] << " " << flogfs_write_file_size(&files[i]) << " " << files[i].block << std::endl;
            FLOG_CHECK(flogfs_close_write(&files[i]));
        }
    }
}

int32_t main(int argc, char *argv[]) {
    bool truncate = false;
    flog_initialize_params_t params {
        .number_of_blocks = 256,
        .pages_per_block = 64,
    };

    for (auto i = 0; i < argc; ++i) {
        auto arg = std::string{ argv[i] };
        if (arg == "--truncate") {
            truncate = true;
        }
    }

    FLOG_CHECK(flogfs_linux_open(Path, truncate, &params));

    FLOG_CHECK(flogfs_initialize(&params));

    std::cout << "Mounting" << std::endl;
    if (flogfs_mount() == FLOG_FAILURE) {
        std::cout << "Formatting" << std::endl;
        FLOG_CHECK(flogfs_format());
        std::cout << "Mounting" << std::endl;
        FLOG_CHECK(flogfs_mount());
    }

    srand(1);

    if (false) {
        write_file("data-1.bin");
        FLOG_CHECK(flogfs_rm("data-1.bin"));
    }

    else if (true) {
        for (auto i = 0; i < 10; ++i) {
            std::cout << "Mounting" << std::endl;
            if (i % 3 == 0) {
                FLOG_CHECK(flogfs_initialize(&params));
                FLOG_CHECK(flogfs_mount());
            }
            generate_random_files(10, 10, 16384, 16384 * 64);
        }
    }
    else {
        auto size1 = write_file("data-1.bin");
        auto size2 = write_file("data-2.bin");
        auto size3 = write_file("data-3.bin");

        read_file("data-1.bin", size1);
        read_file("data-2.bin", size2);
        read_file("data-3.bin", size3);

        reopen_file("data-1.bin", size1);
        reopen_file("data-2.bin", size2);
        reopen_file("data-3.bin", size3);

        seek_file("data-1.bin", size1);

        std::cout << "Deleting" << std::endl;

        FLOG_CHECK(flogfs_rm("data-1.bin"));

        ls_files();

        size1 = write_file("data-1.bin");

        read_file("data-1.bin", size1);

        ls_files();
    }

    FLOG_CHECK(flogfs_linux_close());

    return 0;
}
