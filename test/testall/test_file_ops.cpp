#include <flogfs.h>
#include <flogfs_linux_mmap.h>

#include "test_file_ops.h"

#include "utilities.h"

FileOpsSuite::FileOpsSuite() {
}

FileOpsSuite::~FileOpsSuite() {
};

void FileOpsSuite::SetUp() {
};

void FileOpsSuite::TearDown() {
    flush_and_close();
};

TEST_F(FileOpsSuite, InitializeAndFormat) {
    flog_initialize_params_t params { 16, 16 };

    ASSERT_TRUE(flogfs_linux_open("tests.bin", true, &params));
    ASSERT_TRUE(flogfs_initialize(&params));
    ASSERT_TRUE(flogfs_format());
}

TEST_F(FileOpsSuite, FormatAndMount) {
    initialize_and_open();
}

TEST_F(FileOpsSuite, FileCreateAndExists) {
    initialize_and_open();

    flog_write_file_t file;
    ASSERT_FALSE(flogfs_check_exists("file.bin"));
    ASSERT_TRUE(flogfs_open_write(&file, "file.bin"));
    ASSERT_TRUE(flogfs_check_exists("file.bin"));
    ASSERT_TRUE(flogfs_close_write(&file));
    ASSERT_TRUE(flogfs_check_exists("file.bin"));
    ASSERT_TRUE(flogfs_rm("file.bin"));
    ASSERT_FALSE(flogfs_check_exists("file.bin"));
}

TEST_F(FileOpsSuite, WriteFile) {
    uint8_t pattern[256];

    initialize_and_open();

    flog_write_file_t fwrite;
    ASSERT_TRUE(flogfs_open_write(&fwrite, "file.bin"));

    for (auto i = 0; i < 4; ++i) {
        ASSERT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }

    ASSERT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);

    ASSERT_TRUE(flogfs_close_write(&fwrite));

    flog_read_file_t fread;
    ASSERT_TRUE(flogfs_open_read(&fread, "file.bin"));
    ASSERT_EQ(flogfs_read_file_size(&fread), sizeof(pattern) * 4);
    ASSERT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, FileSizeAfterMount) {
    uint8_t pattern[256];

    initialize_and_open();

    flog_write_file_t fwrite;
    ASSERT_TRUE(flogfs_open_write(&fwrite, "file.bin"));

    for (auto i = 0; i < 4; ++i) {
        ASSERT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }

    ASSERT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);

    ASSERT_TRUE(flogfs_close_write(&fwrite));

    flush_and_close();

    initialize_and_open(false, false);

    flog_read_file_t fread;
    ASSERT_TRUE(flogfs_open_read(&fread, "file.bin"));
    ASSERT_EQ(flogfs_read_file_size(&fread), sizeof(pattern) * 4);
    ASSERT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, ReadFile) {
    uint8_t pattern[256];
    uint8_t temporary[64];

    initialize_and_open();

    flog_write_file_t fwrite;
    ASSERT_TRUE(flogfs_open_write(&fwrite, "file.bin"));
    for (auto i = 0; i < 4; ++i) {
        ASSERT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }
    ASSERT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);
    ASSERT_TRUE(flogfs_close_write(&fwrite));

    flog_read_file_t fread;
    ASSERT_TRUE(flogfs_open_read(&fread, "file.bin"));

    for (auto i = 0; i < sizeof(pattern) * 4 / sizeof(temporary); ++i) {
        ASSERT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 64);
    }

    ASSERT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 0);

    ASSERT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, SeekFile) {
    uint8_t pattern[256];
    uint8_t temporary[64];

    initialize_and_open();

    flog_write_file_t fwrite;
    ASSERT_TRUE(flogfs_open_write(&fwrite, "file.bin"));
    for (auto i = 0; i < 4; ++i) {
        ASSERT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }
    ASSERT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);
    ASSERT_TRUE(flogfs_close_write(&fwrite));

    flog_read_file_t fread;
    ASSERT_TRUE(flogfs_open_read(&fread, "file.bin"));

    ASSERT_TRUE(flogfs_read_seek(&fread, sizeof(pattern)));

    for (auto i = 0; i < sizeof(pattern) * 3 / sizeof(temporary); ++i) {
        ASSERT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 64);
    }

    ASSERT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 0);

    ASSERT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, ListFiles) {
    uint8_t pattern[256];

    initialize_and_open();

    auto expected_names = generate_random_file_names(10);
    for (auto name : expected_names) {
        flog_write_file_t fwrite;
        ASSERT_TRUE(flogfs_open_write(&fwrite, name.c_str()));
        ASSERT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
        ASSERT_TRUE(flogfs_close_write(&fwrite));
    }

    std::vector<std::string> actual_names = get_file_listing();

    ASSERT_EQ(expected_names, actual_names);
}
