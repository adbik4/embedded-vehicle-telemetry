#include "gui.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "io.hpp"
#include "system_state.hpp"

using namespace gui;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Exactly;
using ::testing::Return;

/* TEST PLAN:
   1) when the GUI starts, it is on the right screen
   2) all of the state transitions are correct:
    - MainScreen <-> StatsScreen
    - MainScreen || StatsScreen -> DebugScreen
    - no other transitions occur
    3) The inits are called and are called in the right order
*/

class MockState : public StateIf {
public:
    MOCK_METHOD((const sensor_hub_data_t&), getIMUdata, (), (const, override));
    MOCK_METHOD(const ButtonState&, getButtonState, (const Button id), (const, override));
    MOCK_METHOD((GPSdata_t&), getGNSSdata, (), (override));
    MOCK_METHOD((const GPSdata_t&), getGNSSdata, (), (const, override));
};

class GUITest: public ::testing::Test {
protected:
    MockState system_state;
    GUI& gui = GUI::getInstance();
};

template <class T>
void assertCurrStateIs(GUI& gui) {
    ScreenState& screen = gui.getCurrentScreen();
    ScreenState& target = T::getInstance();

    ASSERT_EQ(&screen , &target);
}

TEST_F(GUITest, InitialState) {
    assertCurrStateIs<MainScreen>(gui);
}

// TEST_F(GUITest, StateTransitions) {
//     static const ButtonState button_pressed = {._isPressed = true, ._isHeld = false};

//     EXPECT_CALL(system_state, getButtonState(Button::B2))
//         .Times(Exactly(2))
//         .WillRepeatedly(Return(button_pressed));
    
//     gui.switchScreen();
//     assertCurrStateIs<StatsScreen>(gui);

//     gui.switchScreen();
//     assertCurrStateIs<MainScreen>(gui);
// }