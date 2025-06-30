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
#include <coreinit/screen.h>
#include <coreinit/time.h>

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

    static constexpr const char* DEFAULT_PATH = "fs:/vol/external01/balatro.wuhb";

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

    // --- Custom font rendering support ---
    // TTF font rendering temporarily disabled for build compatibility.
    // All text will use OSScreenPutFontEx (system font).
    // #include <string>
    // #include <vector>
    // #include <cstring>
    // #define STB_TRUETYPE_IMPLEMENTATION
    // #include "stb_truetype.h"
    // unsigned char* fontBuffer = nullptr;
    // stbtt_fontinfo font;
    // bool fontLoaded = false;
    // void loadCustomFont() { /* disabled */ }

    void loadCustomFont() { /* TTF font rendering disabled */ }

    // Helper: Fill OSScreen buffer with RGB color
    void fillScreenColor(OSScreenID screen, uint8_t r, uint8_t g, uint8_t b) {
        uint32_t* buf = (uint32_t*)OSScreenGetBuffer(screen);
        int width = OSScreenGetWidth(screen);
        int height = OSScreenGetHeight(screen);
        uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                buf[y * width + x] = color;
            }
        }
    }

    void drawText(OSScreenID screen, int x, int y, const char* text) {
        OSScreenPutFontEx(screen, x, y, text);
    }

    // Pixelart loading bar (simple blocky style)
    void drawPixelartBar(OSScreenID screen, int x, int y, int width, int filled) {
        // Draw filled blocks as "█", empty as "·"
        std::string bar;
        for (int i = 0; i < width; ++i) bar += (i < filled) ? "\xDB" : "·";
        drawText(screen, x, y, bar.c_str());
    }

    // LÖVE2D-style loading screen (dark gray, centered bar, custom font placeholder)
    void drawLove2DLoading(int step, int total, const char* status) {
#ifdef __WIIU__
        OSScreenID screens[2] = { SCREEN_TV, SCREEN_DRC };
        for (int i = 0; i < 2; ++i) {
            OSScreenID screen = screens[i];
            // Fill with dark gray (RGB 32,32,32)
            fillScreenColor(screen, 32, 32, 32);
            int width = (screen == SCREEN_TV) ? 80 : 52;
            // Status text (centered)
            if (status) {
                int x = (width - (int)strlen(status)) / 2;
                drawText(screen, x, 10, status);
            }
            // Pixelart loading bar
            int barLen = 32;
            int filled = (step * barLen) / total;
            int x = (width - barLen) / 2;
            drawPixelartBar(screen, x, 14, barLen, filled);
            OSScreenFlipBuffers(screen);
        }
        OSSleepTicks(OSMillisecondsToTicks(30));
#endif
    }

    // Turquoise crash screen (RGB 7,151,176), error text and prompt, custom font placeholder
    void drawCrashScreen(const char* error, const char* tb1, const char* tb2) {
#ifdef __WIIU__
        OSScreenID screens[2] = { SCREEN_TV, SCREEN_DRC };
        for (int i = 0; i < 2; ++i) {
            OSScreenID screen = screens[i];
            fillScreenColor(screen, 7, 151, 176);
            int width = (screen == SCREEN_TV) ? 80 : 52;
            // Error text
            if (error) {
                int x = (width - (int)strlen(error)) / 2;
                drawText(screen, x, 10, error);
            }
            // Traceback lines
            if (tb1 && strlen(tb1) > 0) {
                int x = (width - (int)strlen(tb1)) / 2;
                drawText(screen, x, 12, tb1);
            }
            if (tb2 && strlen(tb2) > 0) {
                int x = (width - (int)strlen(tb2)) / 2;
                drawText(screen, x, 13, tb2);
            }
            // Prompt
            const char* prompt = "Press B to exit...";
            int x = (width - (int)strlen(prompt)) / 2;
            drawText(screen, x, 20, prompt);
            OSScreenFlipBuffers(screen);
        }
#endif
    }

    namespace {
        void drawLoadingBar(float progress, const char* info, const char* error = nullptr) {
#ifdef __WIIU__
            OSScreenID screens[2] = { SCREEN_TV, SCREEN_DRC };
            for (int i = 0; i < 2; ++i) {
                OSScreenID screen = screens[i];
                OSScreenClearBuffer(screen, 0);
                if (info) {
                    OSScreenPutFontEx(screen, 0, 2, info);
                }
                if (error) {
                    OSScreenPutFontEx(screen, 0, 4, error);
                }
                int barWidth = 40;
                int filled = (int)(progress * barWidth);
                char bar[barWidth + 2];
                for (int j = 0; j < barWidth; ++j) bar[j] = (j < filled) ? '#' : '-';
                bar[barWidth] = '\0';
                OSScreenPutFontEx(screen, 0, 6, bar);
                OSScreenFlipBuffers(screen);
            }
            OSSleepTicks(OSMillisecondsToTicks(30));
#endif
        }
    }

    // Helper: Draw loading bar and status text to TV and GamePad
    void drawLoadingBar(int step, int total, const char* status, const char* error = nullptr) {
        static bool screenInited = false;
        if (!screenInited) {
            OSScreenInit();
            screenInited = true;
        }
        OSScreenClearBuffer(SCREEN_TV, 0);
        OSScreenClearBuffer(SCREEN_DRC, 0);
        int tvWidth = 80;
        int drcWidth = 52;
        if (status) {
            int x_tv = (tvWidth - (int)strlen(status)) / 2;
            int x_drc = (drcWidth - (int)strlen(status)) / 2;
            OSScreenPutFontEx(SCREEN_TV, x_tv, 10, status);
            OSScreenPutFontEx(SCREEN_DRC, x_drc, 10, status);
        }
        if (error) {
            int x_tv = (tvWidth - (int)strlen(error)) / 2;
            int x_drc = (drcWidth - (int)strlen(error)) / 2;
            OSScreenPutFontEx(SCREEN_TV, x_tv, 14, error);
            OSScreenPutFontEx(SCREEN_DRC, x_drc, 14, error);
        }
        char bar[65] = {0};
        int barLen = 50;
        int filled = (step * barLen) / total;
        for (int i = 0; i < barLen; ++i) bar[i] = (i < filled) ? '#' : '-';
        bar[barLen] = '\0';
        int x_tv = (tvWidth - barLen) / 2;
        int x_drc = (drcWidth - barLen) / 2;
        OSScreenPutFontEx(SCREEN_TV, x_tv, 12, bar);
        OSScreenPutFontEx(SCREEN_DRC, x_drc, 12, bar);
        OSScreenFlipBuffers(SCREEN_TV);
        OSScreenFlipBuffers(SCREEN_DRC);
    }

    int preInit()
    {
        // Initialize debug logger (safe to call multiple times)
        DebugLogger::init();
        DebugLogger::log("preInit() called");
        loadCustomFont(); // Load TTF font for custom rendering
        int step = 0;
        int total = (int)services.size();
        drawLove2DLoading(step, total, "Starting initialization...");

        for (const auto& service : services)
        {
            char status[128];
            snprintf(status, sizeof(status), "Initializing: %s", service.name);
            drawLove2DLoading(step, total, status);
            DebugLogger::log("%s", status);
            if (!service.init().success())
            {
                char error[128];
                snprintf(error, sizeof(error), "Failed to initialize: %s", service.name);
                drawLove2DLoading(step, total, status);
                // Also show error as overlay
                drawCrashScreen(error, nullptr, nullptr);
                DebugLogger::log("%s", error);
                // Wait for user to see error
                for (int i = 0; i < 180; ++i) OSSleepTicks(OSMillisecondsToTicks(10));
                return -1;
            }
            DebugLogger::log("Service %s initialized successfully", service.name);
            ++step;
        }
        drawLove2DLoading(total, total, "Initialization complete.");
        OSSleepTicks(OSMillisecondsToTicks(200));

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

    // Helper to check for user-defined love.run and log which is used
    void logWhichLoveRun(lua_State* L) {
        lua_getglobal(L, "love");
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "run");
            if (lua_isfunction(L, -1)) {
                DebugLogger::log("User-defined love.run detected: using user main loop");
#ifdef __WIIU__
                FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
                if (logFile) {
                    fprintf(logFile, "User-defined love.run detected: using user main loop\n");
                    fflush(logFile);
                    fclose(logFile);
                }
#endif
            } else {
                DebugLogger::log("No user-defined love.run: using default main loop");
#ifdef __WIIU__
                FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
                if (logFile) {
                    fprintf(logFile, "No user-defined love.run: using default main loop\n");
                    fflush(logFile);
                    fclose(logFile);
                }
#endif
            }
            lua_pop(L, 1); // pop love.run
        } else {
            DebugLogger::log("No global 'love' table found in Lua state");
        }
        lua_pop(L, 1); // pop love
    }

    bool mainLoop(lua_State* L, int argc, int* nres)
    {
        static bool checkedLoveRun = false;
        if (!checkedLoveRun) {
            logWhichLoveRun(L);
            checkedLoveRun = true;
        }
        DebugLogger::log("mainLoop() called with argc=%d, shutdown=%s", argc, s_Shutdown ? "true" : "false");
        
#ifdef __WIIU__
        // Additional simple logging for debugging freeze
        FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile) {
            fprintf(logFile, "mainLoop() entered, about to check shutdown\n");
            fflush(logFile);
            fclose(logFile);
        }
#endif
        
        if (!s_Shutdown)
        {
#ifdef __WIIU__
            FILE* logFile2 = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (logFile2) {
                fprintf(logFile2, "Not shutdown, about to call luax_resume()\n");
                fflush(logFile2);
                fclose(logFile2);
            }
#endif
            DebugLogger::log("About to call luax_resume()...");
            const auto resumeResult = luax_resume(L, argc, nres);
#ifdef __WIIU__
            FILE* logFile3 = fopen("fs:/vol/external01/simple_debug.log", "a");
            if (logFile3) {
                fprintf(logFile3, "luax_resume() returned, result=%d\n", resumeResult);
                fflush(logFile3);
                fclose(logFile3);
            }
#endif
            DebugLogger::log("luax_resume() returned %d", resumeResult);
            
            const auto yielding = (resumeResult == LUA_YIELD);
            DebugLogger::log("yielding = %s", yielding ? "true" : "false");

            if (!yielding)
            {
                // Check if there was a Lua error
                if (resumeResult != 0) // LUA_OK is 0, any non-zero value is an error
                {
                    const char* errorMsg = lua_tostring(L, -1);
                    std::string errorText = errorMsg ? errorMsg : "Unknown Lua error";
                    std::string traceback;
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
                            const char* tb = lua_tostring(L, -1);
                            if (tb) traceback = tb;
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
                    DebugLogger::log("Lua Error: %s", errorText.c_str());
                    if (!traceback.empty()) DebugLogger::log("Lua Traceback: %s", traceback.c_str());

#ifdef __WIIU__
                    // Show error on screen and wait for B button
                    bool waiting = true;
                    VPADStatus vpad;
                    KPADStatus kpad;
                    std::string tb1, tb2;
                    if (!traceback.empty()) {
                        size_t nl = traceback.find('\n');
                        tb1 = traceback.substr(0, nl);
                        tb2 = (nl != std::string::npos) ? traceback.substr(nl+1, traceback.find('\n', nl+1)-nl-1) : "";
                    }
                    while (waiting) {
                        drawCrashScreen(errorText.c_str(), tb1.c_str(), tb2.c_str());
                        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);
                        KPADRead(WPAD_CHAN_0, &kpad, 1);
                        if ((vpad.hold & VPAD_BUTTON_B) || (kpad.hold & WPAD_BUTTON_B)) {
                            waiting = false;
                        }
                        OSSleepTicks(OSMillisecondsToTicks(50));
                    }
#endif
                }
                
                DebugLogger::log("mainLoop() error handling complete, calling SYSLaunchMenu()");
                SYSLaunchMenu();
                s_Shutdown = true;
            }
        }

        bool running = isRunning();
        DebugLogger::log("mainLoop() returning %s", running ? "true" : "false");
        return running;
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
