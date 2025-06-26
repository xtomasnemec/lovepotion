#include "boot.hpp"
#include "common/Console.hpp"
#include "common/service.hpp"
#include "DebugLogger.hpp"

#include "driver/EventQueue.hpp"

#include <coreinit/bsp.h>
#include <coreinit/core.h>
#include <coreinit/dynload.h>
#include <coreinit/exit.h>
#include <coreinit/filesystem.h>
#include <coreinit/foreground.h>

#include <nn/ac/ac_c.h>

#include <proc_ui/procui.h>

#include <padscore/kpad.h>
#include <vpad/input.h>

#include <sysapp/launch.h>

namespace love
{
    // clang-format off
    static constexpr std::array<const Service, 5> services =
    {{
        { "procUI", BIND(ProcUIInit, OSSavesDone_ReadyToRelease), &ProcUIShutdown },
        { "vpad",   BIND(VPADInit),                               &VPADShutdown   },
        { "kpad",   BIND(KPADInit),                               &KPADShutdown   },
        { "ac",     BIND(ACInitialize),                           &ACFinalize     },
        { "fs",     BIND(FSInit),                                 &FSShutdown     }
        // { "bsp",    BIND(bspInitializeShimInterface),             []() { }        }
    }};
    // clang-format on

    uint32_t Console::mainCoreId = 0;
    bool Console::mainCoreIdSet  = false;

    static constexpr const char* DEFAULT_PATH = "fs:/vol/external01/lovepotion.wuhb";

    std::string getApplicationPath(const std::string& argv0)
    {
        if (argv0 == "embedded boot.lua")
            return DEFAULT_PATH;

        OSDynLoad_Module module;
        const auto type  = OS_DYNLOAD_EXPORT_FUNC;
        const char* name = "RL_GetPathOfRunningExecutable";

        if (OSDynLoad_Acquire("homebrew_rpx_loader", &module) == OS_DYNLOAD_OK)
        {
            char path[256];

            bool (*RL_GetPathOfRunningExecutable)(char*, uint32_t);
            auto** function = reinterpret_cast<void**>(&RL_GetPathOfRunningExecutable);

            if (OSDynLoad_FindExport(module, type, name, function) == OS_DYNLOAD_OK)
            {
                if (RL_GetPathOfRunningExecutable(path, sizeof(path)) == 0)
                    return path;
            }
        }

        return DEFAULT_PATH;
    }

    int preInit()
    {
        // Initialize debug logger first
        DebugLogger::init();
        DebugLogger::log("preInit() called");

        /* we aren't running Aroma */
        // if (getApplicationPath().empty())
        //     return -1;

        for (const auto& service : services)
        {
            DebugLogger::log("Initializing service: %s", service.name);
            if (!service.init().success())
            {
                DebugLogger::log("Failed to initialize service: %s", service.name);
                return -1;
            }
            DebugLogger::log("Service %s initialized successfully", service.name);
        }

        WPADEnableWiiRemote(true);
        WPADEnableURCC(true);
        DebugLogger::log("WPAD initialized");

        Console::setMainCoreId(OSGetCoreId());
        DebugLogger::log("Main core ID set to: %u", OSGetCoreId());

        DebugLogger::log("preInit() completed successfully");
        return 0;
    }

    static bool isRunning()
    {
        if (!Console::isMainCoreId(OSGetMainCoreId()))
        {
            ProcUISubProcessMessages(true);
            return true;
        }

        const auto status = ProcUIProcessMessages(true);

        switch (status)
        {
            case PROCUI_STATUS_IN_FOREGROUND:
                EventQueue::getInstance().sendFocus(true);
                break;
            case PROCUI_STATUS_RELEASE_FOREGROUND:
                EventQueue::getInstance().sendFocus(false);
                ProcUIDrawDoneRelease();
                break;
            case PROCUI_STATUS_EXITING:
                EventQueue::getInstance().sendQuit();
                return false;
            default:
                break;
        }

        return true;
    }

    static bool s_Shutdown = false;

    bool mainLoop(lua_State* L, int argc, int* nres)
    {
        if (!s_Shutdown)
        {
            const auto resumeResult = luax_resume(L, argc, nres);
            const auto yielding = (resumeResult == LUA_YIELD);

            if (!yielding)
            {
                // Check if there was a Lua error
                if (resumeResult != 0) // LUA_OK is 0, any non-zero value is an error
                {
                    const char* errorMsg = lua_tostring(L, -1);
                    if (errorMsg)
                    {
                        DebugLogger::log("Lua Error: %s", errorMsg);
                        
                        // Get traceback if available
                        lua_getglobal(L, "debug");
                        if (lua_istable(L, -1))
                        {
                            lua_getfield(L, -1, "traceback");
                            if (lua_isfunction(L, -1))
                            {
                                lua_pushvalue(L, -3); // error message
                                lua_pushinteger(L, 2);
                                lua_call(L, 2, 1);
                                
                                const char* traceback = lua_tostring(L, -1);
                                if (traceback)
                                {
                                    DebugLogger::log("Lua Traceback: %s", traceback);
                                }
                                lua_pop(L, 1); // pop traceback result
                            }
                            else
                            {
                                lua_pop(L, 1); // pop non-function
                            }
                            lua_pop(L, 1); // pop debug table
                        }
                        else
                        {
                            lua_pop(L, 1); // pop non-table
                        }
                    }
                }
                
                SYSLaunchMenu();
                s_Shutdown = true;
            }
        }

        return isRunning();
    }

    void onExit()
    {
        DebugLogger::log("onExit() called - shutting down services");
        
        for (auto it = services.rbegin(); it != services.rend(); ++it)
        {
            DebugLogger::log("Shutting down service: %s", it->name);
            it->exit();
        }
        
        DebugLogger::log("All services shut down");
        DebugLogger::close();
    }
} // namespace love
