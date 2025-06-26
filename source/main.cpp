#include "common/luax.hpp"
#include "common/version.hpp"

#include "boot.hpp"
#include "modules/love/love.hpp"

#ifdef __WIIU__
#include "DebugLogger.hpp"
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

enum DoneAction
{
    DONE_QUIT,
    DONE_RESTART
};

static DoneAction runLove(char** argv, int argc, int& result, love::Variant& restartValue)
{
#ifdef __WIIU__
    love::DebugLogger::log("runLove() called with argc=%d", argc);
    for (int i = 0; i < argc; i++)
        love::DebugLogger::log("argv[%d] = %s", i, argv[i]);
#endif

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_bit(L);

    love::luax_preload(L, love_initialize, "love");

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
            args.push_back("game");

        lua_pushstring(L, "embedded boot.lua");
        lua_rawseti(L, -2, -1);

        for (int index = 1; index < (int)args.size(); index++)
        {
            lua_pushstring(L, args[index]);
            lua_rawseti(L, -2, index);
        }

        lua_setglobal(L, "arg");
    }

    lua_getglobal(L, "require");
    lua_pushstring(L, "love");
    lua_call(L, 1, 1); // leave the returned table on the stack.

    {
        lua_pushboolean(L, 1);
        lua_setfield(L, -2, "_exe");
    }

    love::luax_pushvariant(L, restartValue);
    lua_setfield(L, -2, "restart");
    restartValue = love::Variant();

    lua_pop(L, 1);

    lua_getglobal(L, "require");
    lua_pushstring(L, "love.boot");
    lua_call(L, 1, 1);

    lua_newthread(L);
    lua_pushvalue(L, -2);

    int position = lua_gettop(L);
    int results  = 0;

    while (love::mainLoop(L, 0, &results))
    {
#if LUA_VERSION_NUM >= 504
        lua_pop(L, results)
#else
        lua_pop(L, lua_gettop(L) - position);
#endif
    }

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

    return action;
}

int main(int argc, char** argv)
{
#ifdef __WIIU__
    love::DebugLogger::log("main() starting with argc=%d", argc);
    for (int i = 0; i < argc; i++)
        love::DebugLogger::log("argv[%d] = %s", i, argv[i]);
#endif

    try
    {
        if (love::preInit() != 0)
        {
#ifdef __WIIU__
            love::DebugLogger::log("ERROR: preInit() failed");
#endif
            love::onExit();
            return 0;
        }

        int result        = 0;
        DoneAction action = DONE_QUIT;
        love::Variant restartValue;

#ifdef __WIIU__
        love::DebugLogger::log("Starting main loop");
#endif

        do
        {
            try
            {
                action = runLove(argv, argc, result, restartValue);
#ifdef __WIIU__
                love::DebugLogger::log("runLove completed with action=%d, result=%d", (int)action, result);
#endif
            }
            catch (const love::Exception& e)
            {
#ifdef __WIIU__
                love::DebugLogger::log("LOVE Exception caught in main loop: %s", e.what());
#endif
                result = 1;
                action = DONE_QUIT;
            }
            catch (const std::exception& e)
            {
#ifdef __WIIU__
                love::DebugLogger::log("C++ Exception caught in main loop: %s", e.what());
#endif
                result = 1;
                action = DONE_QUIT;
            }
            catch (...)
            {
#ifdef __WIIU__
                love::DebugLogger::log("Unknown exception caught in main loop");
#endif
                result = 1;
                action = DONE_QUIT;
            }
        } while (action != DONE_QUIT);

#ifdef __WIIU__
        love::DebugLogger::log("Main loop finished, calling onExit()");
#endif
        love::onExit();

        return result;
    }
    catch (const love::Exception& e)
    {
#ifdef __WIIU__
        love::DebugLogger::log("FATAL LOVE Exception in main(): %s", e.what());
#endif
        love::onExit();
        return 1;
    }
    catch (const std::exception& e)
    {
#ifdef __WIIU__
        love::DebugLogger::log("FATAL C++ Exception in main(): %s", e.what());
#endif
        love::onExit();
        return 1;
    }
    catch (...)
    {
#ifdef __WIIU__
        love::DebugLogger::log("FATAL Unknown exception in main()");
#endif
        love::onExit();
        return 1;
    }
}
