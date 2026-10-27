#include <gtest/gtest.h>

/* This file tests the Singleton Base class to prove that it
*  actually obeys the Singleton object contract. */

#include "utilities.hpp"

class TestSubject : public Singleton<TestSubject> {
public:
    int test_var;

    TestSubject() : test_var{} {};
};

class SingletonTest : public ::testing::Test {
protected:
    TestSubject& instance = TestSubject::getInstance();
};

TEST_F(SingletonTest, ReturnsSameInstance)
{
    TestSubject& first = TestSubject::getInstance();
    TestSubject& second = TestSubject::getInstance();

    EXPECT_EQ(&first, &second);
}

TEST_F(SingletonTest, PreservesStateBetweenAccesses)
{
    TestSubject& first = TestSubject::getInstance();
    first.test_var = 42;

    TestSubject& second = TestSubject::getInstance();

    EXPECT_EQ(second.test_var, 42);
}

TEST_F(SingletonTest, InstanceAddressRemainsStable)
{
    TestSubject* first = &TestSubject::getInstance();

    // Access the singleton multiple times.
    for (int i = 0; i < 100; ++i) {
        (void)TestSubject::getInstance();
    }

    TestSubject* last = &TestSubject::getInstance();

    EXPECT_EQ(first, last);
}