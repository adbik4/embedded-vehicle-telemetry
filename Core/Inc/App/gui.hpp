#pragma once

#include "utilities.hpp"
#include <array>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>

namespace gui {

class ScreenState;

class GUI : public Singleton<GUI> {
private:
    ScreenState* currentScreen;

public:
    GUI();

    inline ScreenState& getCurrentScreen() const { return *currentScreen; };
    void processScreen();
    void setScreen(ScreenState* nextScreen);
    void refreshScreen();
};

class ScreenState {
public:
    const std::array<const char*, 3> bottom_bar;

    ScreenState(const std::array<const char*, 3> arr) : bottom_bar(arr) {};

    /* Draws the Screen on entry */
    virtual void init(GUI* gui) = 0;

    /* Refreshes the data on the screen */
    virtual void refresh(GUI* gui) = 0;
    
    /* Points to the next screen */
    virtual void processScreen(GUI* gui) = 0;
};

/* ----- DATA TYPES ----- */

template <typename T>
struct Line {
    std::string name;
    T value;
};

template <typename T, size_t N>
struct Paragraph {
    std::string header;
    std::array<Line<T>, N> contents;
};

using ParagraphVariant = std::variant<
    Paragraph<uint64_t, 4>,
    Paragraph<float, 4>
>;

using DispList = std::array<ParagraphVariant, 3>;

/* ----- Concrete Screens ----- */

class DebugScreen : public Singleton<DebugScreen>, public ScreenState {
private:
    static constexpr std::array<const char*, 3> bottom_bar = {
        "", "", "Main"
    };

    std::array<DispList, 2> disp_lists;

    void updateDispLists();

public:
    DebugScreen() : ScreenState(DebugScreen::bottom_bar) {};
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void processScreen(GUI* gui) override;
};

class MainScreen : public Singleton<MainScreen>, public ScreenState {
private:
    static constexpr std::array<const char*, 3> bottom_bar = {
        USERNAME_A, USERNAME_B, "Stats"
    };

public:
    MainScreen() : ScreenState(MainScreen::bottom_bar) {};
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void processScreen(GUI* gui) override;
};

class StatsScreen : public Singleton<StatsScreen>, public ScreenState {
private:
    static constexpr std::array<const char*, 3> bottom_bar = {
        "Next", "Prev", "Debug"
    };

public:
    StatsScreen() : ScreenState(StatsScreen::bottom_bar) {};
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void processScreen(GUI* gui) override;
};

} // namespace gui