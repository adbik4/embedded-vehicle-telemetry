#include <gtest/gtest.h>

//#include "gui.hpp"

class GUITest: public ::testing::Test {
protected:
    // Do Sth
};

TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}