#include <flogfs.h>
#include <flogfs_linux_mmap.h>

#include "test_long_running.h"

#include "utilities.h"

LongRunningOpsSuite::LongRunningOpsSuite() {
}

LongRunningOpsSuite::~LongRunningOpsSuite() {
};

void LongRunningOpsSuite::SetUp() {
};

void LongRunningOpsSuite::TearDown() {
    flush_and_close();
};

TEST_F(LongRunningOpsSuite, GenerateRandomFiles) {
    initialize_and_open();

    auto names = generate_random_file_names(10);

    write_files_randomly(names, 20, 4096, 65536);
}

TEST_F(LongRunningOpsSuite, RepeatedFormatting) {
    initialize_and_open();

    auto names = generate_random_file_names(10);

    write_files_randomly(names, 20, 4096, 65536);

    for (auto &name : names) {
        EXPECT_TRUE(flogfs_rm(name.c_str()));
    }

    flush_and_close();

    initialize_and_open(false, true);

    write_files_randomly(names, 20, 4096, 65536);
}
