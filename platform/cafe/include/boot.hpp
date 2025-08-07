#pragma once

#include "common/luax.hpp"

namespace love
{
    std::string getApplicationPath(const std::string& argv0 = "");

    int preInit();

    bool mainLoop(lua_State* L, int argc, int* nres);

    void onExit();
    
    // Crash screen function for Wii U
    void drawCrashScreen(const char* error, const char* errorType, const char* functionInfo, const char* tb1, const char* tb2, const char* tb3);
} // namespace love
