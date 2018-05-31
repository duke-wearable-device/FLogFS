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
    EXPECT_TRUE(flush_and_close());
};

TEST_F(FileOpsSuite, InitializeAndFormat) {
    flog_initialize_params_t params { 16, 16 };

    EXPECT_TRUE(flogfs_linux_open("tests.bin", true, &params));
    EXPECT_TRUE(flogfs_initialize(&params));
    EXPECT_TRUE(flogfs_format());
}

TEST_F(FileOpsSuite, FormatAndMount) {
    EXPECT_TRUE(initialize_and_open());
}

TEST_F(FileOpsSuite, FileCreateAndExists) {
    EXPECT_TRUE(initialize_and_open());

    flog_write_file_t file;
    EXPECT_FALSE(flogfs_check_exists("file.bin"));
    EXPECT_TRUE(flogfs_open_write(&file, "file.bin"));
    EXPECT_TRUE(flogfs_check_exists("file.bin"));
    EXPECT_TRUE(flogfs_close_write(&file));
    EXPECT_TRUE(flogfs_check_exists("file.bin"));
    EXPECT_TRUE(flogfs_rm("file.bin"));
    EXPECT_FALSE(flogfs_check_exists("file.bin"));
}

TEST_F(FileOpsSuite, WriteFile) {
    uint8_t pattern[256];

    EXPECT_TRUE(initialize_and_open());

    flog_write_file_t fwrite;
    EXPECT_TRUE(flogfs_open_write(&fwrite, "file.bin"));

    for (auto i = 0; i < 4; ++i) {
        EXPECT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }

    EXPECT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);

    EXPECT_TRUE(flogfs_close_write(&fwrite));

    flog_read_file_t fread;
    EXPECT_TRUE(flogfs_open_read(&fread, "file.bin"));
    EXPECT_EQ(flogfs_read_file_size(&fread), sizeof(pattern) * 4);
    EXPECT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, FileSizeAfterMount) {
    uint8_t pattern[256];

    EXPECT_TRUE(initialize_and_open());

    flog_write_file_t fwrite;
    EXPECT_TRUE(flogfs_open_write(&fwrite, "file.bin"));

    for (auto i = 0; i < 4; ++i) {
        EXPECT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }

    EXPECT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);

    EXPECT_TRUE(flogfs_close_write(&fwrite));

    EXPECT_TRUE(flush_and_close());

    EXPECT_TRUE(initialize_and_open(false));

    flog_read_file_t fread;
    EXPECT_TRUE(flogfs_open_read(&fread, "file.bin"));
    EXPECT_EQ(flogfs_read_file_size(&fread), sizeof(pattern) * 4);
    EXPECT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, ReadFile) {
    uint8_t pattern[256];
    uint8_t temporary[64];

    EXPECT_TRUE(initialize_and_open());

    flog_write_file_t fwrite;
    EXPECT_TRUE(flogfs_open_write(&fwrite, "file.bin"));
    for (auto i = 0; i < 4; ++i) {
        EXPECT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }
    EXPECT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);
    EXPECT_TRUE(flogfs_close_write(&fwrite));

    flog_read_file_t fread;
    EXPECT_TRUE(flogfs_open_read(&fread, "file.bin"));

    for (auto i = 0; i < sizeof(pattern) * 4 / sizeof(temporary); ++i) {
        EXPECT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 64);
    }

    EXPECT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 0);

    EXPECT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, SeekFile) {
    uint8_t pattern[256];
    uint8_t temporary[64];

    EXPECT_TRUE(initialize_and_open());

    flog_write_file_t fwrite;
    EXPECT_TRUE(flogfs_open_write(&fwrite, "file.bin"));
    for (auto i = 0; i < 4; ++i) {
        EXPECT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
    }
    EXPECT_EQ(flogfs_write_file_size(&fwrite), sizeof(pattern) * 4);
    EXPECT_TRUE(flogfs_close_write(&fwrite));

    flog_read_file_t fread;
    EXPECT_TRUE(flogfs_open_read(&fread, "file.bin"));

    EXPECT_TRUE(flogfs_read_seek(&fread, sizeof(pattern)));

    for (auto i = 0; i < sizeof(pattern) * 3 / sizeof(temporary); ++i) {
        EXPECT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 64);
    }

    EXPECT_EQ(flogfs_read(&fread, temporary, sizeof(temporary)), 0);

    EXPECT_TRUE(flogfs_close_read(&fread));
}

TEST_F(FileOpsSuite, ListFiles) {
    uint8_t pattern[256];

    EXPECT_TRUE(initialize_and_open());

    auto expected_names = generate_random_file_names(10);
    for (auto name : expected_names) {
        flog_write_file_t fwrite;
        EXPECT_TRUE(flogfs_open_write(&fwrite, name.c_str()));
        EXPECT_EQ(flogfs_write(&fwrite, pattern, sizeof(pattern)), sizeof(pattern));
        EXPECT_TRUE(flogfs_close_write(&fwrite));
    }

    std::vector<std::string> actual_names = get_file_listing();

    EXPECT_EQ(expected_names, actual_names);
}
