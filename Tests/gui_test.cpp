#include "gui.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "io.hpp"

using namespace gui;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Exactly;
using ::testing::Return;

ButtonMock button1(ButtonId::B1);
ButtonMock button2(ButtonId::B2);
ButtonMock button3(ButtonId::B3);

/* TEST PLAN:
   1) when the GUI starts, it is on the right screen
   2) wrong presses don't switch the screen
   3) all of the state transitions are correct:
    - MainScreen <-> StatsScreen
    - MainScreen || StatsScreen -> DebugScreen
    - no other transitions occur
    4) The inits are called and are called in the right order
*/

// class MockState : public StateIf {
// public:
//     MOCK_METHOD((const sensor_hub_data_t&), getIMUdata, (), (const, override));
//     MOCK_METHOD((GPSdata_t&), getGNSSdata, (), (override));
//     MOCK_METHOD((const GPSdata_t&), getGNSSdata, (), (const, override));
// };

class GUITest: public ::testing::Test {
protected:
    GUI& gui = GUI::getInstance();
};

template <class T>
void assertCurrStateIs(GUI& gui) {
    gui.processScreen();
    ScreenState& screen = gui.getCurrentScreen();
    ScreenState& target = T::getInstance();

    ASSERT_EQ(&screen , &target);
}

TEST_F(GUITest, InitialState) {
    assertCurrStateIs<DebugScreen>(gui);
}

TEST_F(GUITest, WrongActionsDontChangeState) {
    button1.setState(ButtonState::pressed);
    button2.setState(ButtonState::pressed);
    assertCurrStateIs<DebugScreen>(gui);
}

TEST_F(GUITest, StateTransitions) {
    button3.setState(ButtonState::pressed);
    assertCurrStateIs<MainScreen>(gui);

    button3.setState(ButtonState::pressed);
    assertCurrStateIs<StatsScreen>(gui);

    button3.setState(ButtonState::pressed);
    assertCurrStateIs<DebugScreen>(gui);
}