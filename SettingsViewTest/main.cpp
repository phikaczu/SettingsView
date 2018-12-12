#include "pch.h"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    //::testing::FLAGS_gtest_death_test_style = "fast";
    ::testing::GTEST_FLAG(filter) = "MonitorTest.*";

    return RUN_ALL_TESTS();
}
