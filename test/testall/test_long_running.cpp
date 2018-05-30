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
};

TEST_F(LongRunningOpsSuite, GenerateRandomFiles) {
    EXPECT_TRUE(initialize_and_open());

    auto names = generate_random_file_names(10);

    analyze_file_system();

    write_files_randomly(names, 20, 4096, 65536);

    analyze_file_system();
}
