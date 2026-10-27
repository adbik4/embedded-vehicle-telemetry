#pragma once

#include "utilities.hpp"
#include <array>
#include <map>

class ScreenState;

class GUI {
private:
    ScreenState* currentScreen = nullptr;

public:
    GUI();

    inline ScreenState* getCurrentScreen() const { return currentScreen; }
    void switchScreen();
    void setScreen(ScreenState& nextScreen);
};

class ScreenState {
public:
    virtual ~ScreenState() {}

    /* Draws the Screen on entry */
    virtual void init(GUI* gui) = 0;

    /* Refreshes the data on the screen */
    virtual void refresh(GUI* gui) = 0;
    
    /* Points to the next screen */
    virtual ScreenState& switchScreen(GUI* gui) = 0;
};

class DebugScreen : public Singleton<DebugScreen>, public ScreenState {
private:
    const std::array<const char*, 3> bottom_bar = {"", "", "BACK"};

public:
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    ScreenState& switchScreen(GUI* gui) override;
};

static std::map<ScreenState*, ScreenState*> screenTransitions = {
    {&DebugScreen::getInstance(), &DebugScreen::getInstance()}
};