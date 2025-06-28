#include "debug_log.h"
#include "DebugLogger.hpp"
#include <cstdio>

extern "C" void wiiu_debug_log_exception(const char* msg)
{
    if (msg && *msg)
    {
        // Use DebugLogger static methods directly
        love::DebugLogger::log("StreamBuffer Exception: %s", msg);
    }
}
