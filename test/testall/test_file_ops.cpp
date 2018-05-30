#include <flogfs.h>
#include <flogfs_linux_mmap.h>

#include "test_file_ops.h"

FileOpsSuite::FileOpsSuite() {
}

FileOpsSuite::~FileOpsSuite() {};

void FileOpsSuite::SetUp() {};

void FileOpsSuite::TearDown() {};

TEST_F(FileOpsSuite, Nothing) {
    flog_initialize_params_t params { 16, 16 };

    EXPECT_TRUE(flogfs_linux_open("tests.bin", true, &params));

    EXPECT_TRUE(flogfs_initialize(&params));
}
