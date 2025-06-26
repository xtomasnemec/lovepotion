#pragma once

#include <cstdio>
#include <cstdarg>

namespace love
{
    class DebugLogger
    {
    public:
        static void init();
        static void close();
        static void log(const char* format, ...);
        static void logLuaError(const char* error);
        
    private:
        static FILE* logFile;
        static bool initialized;
    };
}
