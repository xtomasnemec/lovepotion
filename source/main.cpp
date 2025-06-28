#include "common/luax.hpp"
#include "common/version.hpp"

#include "boot.hpp"
#include "modules/love/love.hpp"

#ifdef __WIIU__
#include "DebugLogger.hpp"
#include <cstdio>
#include <ctime>
#endif

#include <filesystem>
#include <vector>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "common/Exception.hpp"

#ifdef __WIIU__
// Simple logging functions using basic C file I/O
void simpleLog(const char* message) {
    FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
    if (logFile == nullptr) {
        // Try without fs: prefix
        logFile = fopen("/vol/external01/simple_debug.log", "a");
    }
    if (logFile == nullptr) {
        // Try internal storage
        logFile = fopen("/vol/storage_mlc01/simple_debug.log", "a");
    }
    if (logFile == nullptr) {
        // Try simple path
        logFile = fopen("simple_debug.log", "a");
    }
    
    if (logFile != nullptr) {
        time_t rawtime;
        struct tm* timeinfo;
        char timeStr[80];
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
        
        fprintf(logFile, "[%s] %s\n", timeStr, message);
        fflush(logFile);
        fclose(logFile);
    }
    
    // Also log to DebugLogger (it has its own fallbacks)
    // love::DebugLogger::log("SIMPLE: %s", message);  // Temporarily disabled
}

void initSimpleLog() {
    FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "w");
    if (logFile == nullptr) {
        logFile = fopen("/vol/external01/simple_debug.log", "w");
    }
    if (logFile == nullptr) {
        logFile = fopen("/vol/storage_mlc01/simple_debug.log", "w");
    }
    if (logFile == nullptr) {
        logFile = fopen("simple_debug.log", "w");
    }
    
    if (logFile != nullptr) {
        fprintf(logFile, "=== LOVE Potion Simple Debug Log ===\n");
        fprintf(logFile, "Log started\n");
        fprintf(logFile, "=====================================\n");
        fflush(logFile);
        fclose(logFile);
    }
}
#endif

enum DoneAction
{
    DONE_QUIT,
    DONE_RESTART
};

static DoneAction runLove(char** argv, int argc, int& result, love::Variant& restartValue)
{
#ifdef __WIIU__
    char buffer[256];
    sprintf(buffer, "runLove() called with argc=%d", argc);
    simpleLog(buffer);
    
    simpleLog("Creating Lua state...");
#endif

    lua_State* L = luaL_newstate();
#ifdef __WIIU__
    simpleLog("Lua state created, opening libs...");
#endif
    luaL_openlibs(L);
    luaopen_bit(L);

#ifdef __WIIU__
    simpleLog("Libs opened, preloading love...");
#endif
    love::luax_preload(L, love_initialize, "love");

#ifdef __WIIU__
    simpleLog("Love preloaded, setting up args table...");
#endif

    {
        lua_newtable(L);

        if (argc > 0)
        {
            lua_pushstring(L, argv[0]);
            lua_rawseti(L, -2, -2);
        }

        // on 3DS and Switch, argv[0] is the binary path
        // on Wii U, argv[0] is "safe.rpx"
        // args[-1] is "embedded boot.lua"
        // args[1] is "game" for the game folder
        // this should allow "game" to be used in love.filesystem.setSource
        std::vector<const char*> args(argv, argv + argc);

        /* if the game directory exists, add the arg */
        std::filesystem::path filepath = love::getApplicationPath(argv[0]);
        if (std::filesystem::exists(filepath.parent_path().append("game")))
        {
#ifdef __WIIU__
            simpleLog("Found game directory, adding 'game' argument");
#endif
            args.push_back("game");
        }
#ifdef __WIIU__
        else
        {
            simpleLog("No game directory found, will use nogame screen");
        }
#endif

        lua_pushstring(L, "embedded boot.lua");
        lua_rawseti(L, -2, -1);

        for (int index = 1; index < (int)args.size(); index++)
        {
            lua_pushstring(L, args[index]);
            lua_rawseti(L, -2, index);
        }

        lua_setglobal(L, "arg");
    }

#ifdef __WIIU__
    simpleLog("Args table set, requiring love module...");
#endif

    lua_getglobal(L, "require");
    lua_pushstring(L, "love");
    lua_call(L, 1, 1); // leave the returned table on the stack.

#ifdef __WIIU__
    simpleLog("Love module required, setting _exe flag...");
#endif

    {
        lua_pushboolean(L, 1);
        lua_setfield(L, -2, "_exe");
    }

    love::luax_pushvariant(L, restartValue);
    lua_setfield(L, -2, "restart");
    restartValue = love::Variant();

    lua_pop(L, 1);

#ifdef __WIIU__
    simpleLog("Setting up love.boot...");
#endif

    lua_getglobal(L, "require");
    lua_pushstring(L, "love.boot");
    lua_call(L, 1, 1);

#ifdef __WIIU__
    simpleLog("love.boot loaded, creating new thread...");
#endif

    lua_newthread(L);
    lua_pushvalue(L, -2);

    int position = lua_gettop(L);
    int results  = 0;

#ifdef __WIIU__
    simpleLog("Starting main loop...");
    love::DebugLogger::log("About to enter main loop with position=%d", position);
    simpleLog("About to call first love::mainLoop()...");
#endif

    int loopCount = 0;
    bool mainLoopResult = false;
    
#ifdef __WIIU__
    // Use simple counter instead of time() which seems broken on Wii U
    int iterationCounter = 0;
    int lastLogIteration = 0;
#endif
    
    while (true)
    {
        try
        {
#ifdef __WIIU__
            // Log every 60 iterations or first few iterations
            if (loopCount < 3 || (loopCount % 60 == 0))
            {
                char counterBuffer[256];
                sprintf(counterBuffer, "Loop iteration %d", loopCount);
                simpleLog(counterBuffer);
            }
            
            // Log before every call for first few iterations
            if (loopCount < 3)
            {
                simpleLog("About to call love::mainLoop()...");
            }
            
            // Emergency break after 1000 iterations to prevent infinite loop
            if (loopCount > 1000)
            {
                simpleLog("ERROR: Too many iterations, breaking out of loop");
                break;
            }
#endif
            
            mainLoopResult = love::mainLoop(L, 0, &results);
            
#ifdef __WIIU__
            if (loopCount < 3)
            {
                simpleLog("love::mainLoop() returned successfully");
            }
#endif
            
#ifdef __WIIU__
            if (loopCount < 5) // Log first few iterations in detail
            {
                char buffer[256];
                sprintf(buffer, "mainLoop() returned %s, results=%d", 
                        mainLoopResult ? "true" : "false", results);
                simpleLog(buffer);
            }
#endif
            
            if (!mainLoopResult)
            {
#ifdef __WIIU__
                simpleLog("mainLoop() returned false, exiting...");
#endif
                break;
            }
            
#if LUA_VERSION_NUM >= 504
            lua_pop(L, results)
#else
            lua_pop(L, lua_gettop(L) - position);
#endif
            
            loopCount++;
        }
        catch (const love::Exception& e)
        {
#ifdef __WIIU__
            love::DebugLogger::log("LOVE Exception in main loop iteration: %s", e.what());
            simpleLog("LOVE Exception in main loop iteration");
#endif
            break; // Exit the main loop
        }
        catch (const std::exception& e)
        {
#ifdef __WIIU__
            love::DebugLogger::log("C++ Exception in main loop iteration: %s", e.what());
            simpleLog("C++ Exception in main loop iteration");
#endif
            break; // Exit the main loop
        }
        catch (...)
        {
#ifdef __WIIU__
            love::DebugLogger::log("Unknown exception in main loop iteration");
            simpleLog("Unknown exception in main loop iteration");
#endif
            break; // Exit the main loop
        }
    }

#ifdef __WIIU__
    simpleLog("Main loop finished, processing results...");
    love::DebugLogger::log("Main loop exited normally");
#endif

    result            = 0;
    DoneAction action = DONE_QUIT;

    const auto index = position;
    if (!lua_isnoneornil(L, index))
    {
        if (lua_type(L, index) == LUA_TSTRING && strcmp(lua_tostring(L, index), "restart") == 0)
            action = DONE_RESTART;

        if (lua_isnumber(L, index))
            result = lua_tonumber(L, index);

        if (index < lua_gettop(L))
            restartValue = love::luax_checkvariant(L, index + 1, false);
    }

#ifdef __WIIU__
    // Check if there are any Lua errors left on the stack
    if (lua_gettop(L) > 0 && lua_type(L, -1) == LUA_TSTRING)
    {
        const char* errorMsg = lua_tostring(L, -1);
        if (errorMsg)
        {
            love::DebugLogger::log("Final Lua error on stack: %s", errorMsg);
        }
    }
#endif

    lua_close(L);

#ifdef __WIIU__
    simpleLog("runLove() finished, returning action");
#endif

    return action;
}

int main(int argc, char** argv)
{
#ifdef __WIIU__
    // Initialize simple logging first
    initSimpleLog();
    simpleLog("=== MAIN() STARTED ===");
    
    // Initialize DebugLogger too
    love::DebugLogger::init();
    love::DebugLogger::log("=== MAIN() STARTED ===");
    
    char buffer[256];
    sprintf(buffer, "main() called with argc=%d", argc);
    simpleLog(buffer);
    
    for (int i = 0; i < argc; i++) {
        sprintf(buffer, "argv[%d] = %s", i, argv[i] ? argv[i] : "(null)");
        simpleLog(buffer);
    }
    
    simpleLog("About to call preInit()");
    
    // Also use DebugLogger
    love::DebugLogger::log("main() starting with argc=%d", argc);
    for (int i = 0; i < argc; i++)
        love::DebugLogger::log("argv[%d] = %s", i, argv[i]);
#endif

    try
    {
#ifdef __WIIU__
        simpleLog("About to call love::preInit()");
#endif
        int preInitResult = love::preInit();
        if (preInitResult != 0)
        {
#ifdef __WIIU__
            simpleLog("ERROR: preInit() failed!");
            love::DebugLogger::log("ERROR: preInit() failed with code %d", preInitResult);
#endif
            love::onExit();
            return 0;
        }
    }
    catch (const love::Exception& e)
    {
#ifdef __WIIU__
        love::DebugLogger::log("LOVE Exception in preInit(): %s", e.what());
#endif
        love::onExit();
        return 1;
    }
    catch (const std::exception& e)
    {
#ifdef __WIIU__
        love::DebugLogger::log("C++ Exception in preInit(): %s", e.what());
#endif
        love::onExit();
        return 1;
    }
    catch (...)
    {
#ifdef __WIIU__
        love::DebugLogger::log("Unknown exception in preInit()");
#endif
        love::onExit();
        return 1;
    }

    try
    {
#ifdef __WIIU__
        simpleLog("preInit() completed successfully");
#endif

        int result        = 0;
        DoneAction action = DONE_QUIT;
        love::Variant restartValue;

#ifdef __WIIU__
        simpleLog("Starting main game loop");
        love::DebugLogger::log("Starting main loop");
#endif

        do
        {
            try
            {
#ifdef __WIIU__
                love::DebugLogger::log("Calling runLove() iteration...");
#endif
                action = runLove(argv, argc, result, restartValue);
#ifdef __WIIU__
                love::DebugLogger::log("runLove completed with action=%d, result=%d", (int)action, result);
#endif
            }
            catch (const love::Exception& e)
            {
#ifdef __WIIU__
                simpleLog("LOVE Exception caught in main loop");
                love::DebugLogger::log("LOVE Exception caught in main loop: %s", e.what());
#endif
                result = 1;
                action = DONE_QUIT;
            }
            catch (const std::exception& e)
            {
#ifdef __WIIU__
                simpleLog("C++ Exception caught in main loop");
                love::DebugLogger::log("C++ Exception caught in main loop: %s", e.what());
#endif
                result = 1;
                action = DONE_QUIT;
            }
            catch (...)
            {
#ifdef __WIIU__
                simpleLog("Unknown exception caught in main loop");
                love::DebugLogger::log("Unknown exception caught in main loop");
#endif
                result = 1;
                action = DONE_QUIT;
            }
        } while (action != DONE_QUIT);

#ifdef __WIIU__
        simpleLog("Main loop finished, calling onExit()");
        love::DebugLogger::log("Main loop finished, calling onExit()");
#endif
        
        try
        {
            love::onExit();
        }
        catch (const love::Exception& e)
        {
#ifdef __WIIU__
            love::DebugLogger::log("LOVE Exception in onExit(): %s", e.what());
#endif
        }
        catch (const std::exception& e)
        {
#ifdef __WIIU__
            love::DebugLogger::log("C++ Exception in onExit(): %s", e.what());
#endif
        }
        catch (...)
        {
#ifdef __WIIU__
            love::DebugLogger::log("Unknown exception in onExit()");
#endif
        }

#ifdef __WIIU__
        simpleLog("=== MAIN() COMPLETED SUCCESSFULLY ===");
        love::DebugLogger::close();
#endif
        return result;
    }
    catch (const love::Exception& e)
    {
#ifdef __WIIU__
        love::DebugLogger::log("FATAL LOVE Exception in main(): %s", e.what());
        love::DebugLogger::close();
#endif
        love::onExit();
        return 1;
    }
    catch (const std::exception& e)
    {
#ifdef __WIIU__
        love::DebugLogger::log("FATAL C++ Exception in main(): %s", e.what());
        love::DebugLogger::close();
#endif
        love::onExit();
        return 1;
    }
    catch (...)
    {
#ifdef __WIIU__
        love::DebugLogger::log("FATAL Unknown exception in main()");
        love::DebugLogger::close();
#endif
        love::onExit();
        return 1;
    }
}
