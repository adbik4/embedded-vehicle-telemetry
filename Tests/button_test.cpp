#include <gtest/gtest.h>
#include "io.hpp"

TEST(ButtonTest, AllButtonTransitions) {
    Button b;

    ASSERT_EQ(b.getState(), ButtonState::idle);
    b.advanceState();
    ASSERT_EQ(b.getState(), ButtonState::debouncing);
    b.advanceState();
    ASSERT_EQ(b.getState(), ButtonState::pressed);
    b.advanceState();
    ASSERT_EQ(b.getState(), ButtonState::held);
}

TEST(ButtonTest, GoesIdle) {
    Button b;
    
    b.advanceState();
    b.advanceState();
    ASSERT_NE(b.getState(), ButtonState::idle);
    b.goIdle();
    ASSERT_EQ(b.getState(), ButtonState::idle);
}