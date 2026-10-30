#pragma once

#include "utilities.hpp"
#include "io.hpp"
#include <array>
#include <functional>
#include <map>
#include <optional>

namespace gui {

class ScreenState;

// TODO: Detect button presses

class GUI : public Singleton<GUI> {
private:
    ScreenState& currentScreen;
    ButtonState button1;
    ButtonState button2;
    ButtonState button3;

public:
    GUI();

    inline ScreenState& getCurrentScreen() const { return currentScreen; };
    std::optional<const ButtonState> getButtonState(const Button id) const;
    void switchScreen();
    void setScreen(ScreenState& nextScreen);
    void refreshScreen();
};

class ScreenState {
public:
    virtual ~ScreenState() {}

    /* Draws the Screen on entry */
    virtual void init(GUI* gui) = 0;

    /* Refreshes the data on the screen */
    virtual void refresh(GUI* gui) = 0;
    
    /* Points to the next screen */
    virtual void switchScreen(GUI* gui) = 0;
};

/* ----- Concrete Screens ----- */

class DebugScreen : public Singleton<DebugScreen>, public ScreenState {
private:
    const std::array<const char*, 3> bottom_bar = {"", "", "Back"};

public:
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void switchScreen(GUI* gui) override;
};

class MainScreen : public Singleton<MainScreen>, public ScreenState {
private:
    const std::array<const char*, 3> bottom_bar = {USERNAME_A, USERNAME_B, "Stats"};

public:
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void switchScreen(GUI* gui) override;
};

} // namespace gui