#pragma once

#include "EPD_DEV_Config.h"
#include "GUI_Paint.h"
#include "imagedata.h"
#include "GUI_Paint.h"

#include "utilities.h"
#include <stdlib.h>

class GUI {
private:

public:
    GUI();
};

class Screen {
private:
    const char* options[3];

public:
    Screen(const char* opt1, const char* opt2, const char* opt3) :
    options{opt1, opt2, opt3}
    {};

    virtual void init() = 0;
    virtual void refresh() = 0;
};

class DebugScreen : public Screen {
public:
    DebugScreen() : Screen("", "", "BACK") {};
};
