#include "DebugLogger.hpp"
#include <cstring>
#include <ctime>

namespace love
{
    FILE* DebugLogger::logFile = nullptr;
    bool DebugLogger::initialized = false;

    void DebugLogger::init()
    {
        if (initialized)
            return;

        const char* logPath = "sd:/lovepotion_debug.log";
        logFile = fopen(logPath, "w");
        if (logFile != nullptr)
        {
            initialized = true;
            log("=== LOVE Potion Debug Log Started ===");
            
            // Log build info
            log("Build Type: DEBUG");
            log("Platform: Wii U (Cafe)");
            
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];
            
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            log("Started at: %s", buffer);
            log("Log file: %s", logPath);
            log("=====================================");
            
            fflush(logFile);
        }
    }

    void DebugLogger::log(const char* format, ...)
    {
        if (!initialized || logFile == nullptr)
            return;

        va_list args;
        va_start(args, format);
        
        // Add timestamp
        time_t rawtime;
        struct tm* timeinfo;
        char timeBuffer[20];
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeinfo);
        
        fprintf(logFile, "[%s] ", timeBuffer);
        vfprintf(logFile, format, args);
        fprintf(logFile, "\n");
        fflush(logFile);
        
        va_end(args);
    }

    void DebugLogger::logLuaError(const char* error)
    {
        log("LUA ERROR: %s", error);
    }

    void DebugLogger::close()
    {
        if (initialized && logFile != nullptr)
        {
            log("=== LOVE Potion Debug Log Ended ===");
            fclose(logFile);
            logFile = nullptr;
            initialized = false;
        }
    }

    // C-style function for use in template code
    extern "C" void wiiu_debug_log_exception(const char* msg)
    {
        DebugLogger::log("C++ Exception caught: %s", msg);
    }
}
