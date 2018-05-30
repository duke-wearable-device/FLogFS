#include <gtest/gtest.h>

class LongRunningOpsSuite : public ::testing::Test {
protected:
    LongRunningOpsSuite();
    virtual ~LongRunningOpsSuite();

    virtual void SetUp();
    virtual void TearDown();

};
