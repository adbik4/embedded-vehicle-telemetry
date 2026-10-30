#include "gui.hpp"
#include <gtest/gtest.h>

using namespace gui;

class GUITest: public ::testing::Test {
protected:
    GUI& gui = GUI::getInstance();
};

TEST_F(GUITest, InitialState) {
    ScreenState& state = gui.getCurrentScreen();
    ScreenState& target = DebugScreen::getInstance();

    ASSERT_EQ(&state, &target);
}