#pragma once

#include "settings.h"
#include "utilities.hpp"
#include <etl/string.h>
#include <etl/tuple.h>
#include <array>
#include <functional>
#include <map>
#include <optional>

namespace gui {

class ScreenState;

class GUI : public Singleton<GUI> {
private:
    ScreenState* currentScreen;

public:
    GUI();

    inline ScreenState& getCurrentScreen() const { return *currentScreen; };
    void processInput();
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
    virtual void processInput(GUI* gui) = 0;
};

/* ----- DATA TYPES ----- */

template <typename T>
struct Line {
    static constexpr size_t MAX_NAME_SIZE = 16;
    etl::string<MAX_NAME_SIZE> name;
    T value;
};

template <typename T, size_t N>
struct Paragraph {
    static constexpr size_t MAX_HEADER_SIZE = 32;
    etl::string<MAX_HEADER_SIZE> header;
    std::array<Line<T>, N> contents;
};

/* ----- Concrete Screens ----- */
/* ----- DEBUG SCREEN ----- */

using DebugPage1 = etl::tuple<
    Paragraph<uint64_t, 1>,
    Paragraph<float_t, 3>
>;

using DebugPage2 = etl::tuple<
    Paragraph<float_t, 3>,
    Paragraph<float_t, 3>
>;

using DebugPage3 = etl::tuple<
    Paragraph<float_t, 4>,
    Paragraph<etl::string<DEBUG_MSG_SIZE>, 1>
>;

class DebugScreen : public Singleton<DebugScreen>, public ScreenState {
private:
    static constexpr std::array<const char*, 3> bottom_bar = {
        "Prev", "Next", "Main"
    };

    static constexpr size_t PAGE_COUNT = 2;
    uint8_t _curr_page;
    etl::tuple<DebugPage1, DebugPage2> _debug_page;


    void update_pages();
    void increment_page();
    void decrement_page();

    void draw_current_page();

public:
    DebugScreen() : ScreenState(DebugScreen::bottom_bar), _curr_page(0) {
        static_assert(_debug_page.size() == PAGE_COUNT);
        static_assert(PAGE_COUNT > 0);
        update_pages();
    };

    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void processInput(GUI* gui) override;
};

/* ----- MAIN SCREEN ----- */

class MainScreen : public Singleton<MainScreen>, public ScreenState {
private:
    static constexpr std::array<const char*, 3> bottom_bar = {
        USERNAME_A, USERNAME_B, "Stats"
    };

public:
    MainScreen() : ScreenState(MainScreen::bottom_bar) {};
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void processInput(GUI* gui) override;
};

/* ----- STATS SCREEN ----- */

class StatsScreen : public Singleton<StatsScreen>, public ScreenState {
private:
    static constexpr std::array<const char*, 3> bottom_bar = {
        "Prev", "Next", "Debug"
    };

public:
    StatsScreen() : ScreenState(StatsScreen::bottom_bar) {};
    void init(GUI* gui) override;
    void refresh(GUI* gui) override;
    void processInput(GUI* gui) override;
};

} // namespace gui