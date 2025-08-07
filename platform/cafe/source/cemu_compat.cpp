#include "cemu_compat.hpp"
#include <cstring>

#ifdef __WIIU__
#include <coreinit/core.h>
#include <coreinit/dynload.h>

// Simple runtime detection of Cemu emulator
// This is a heuristic approach - not 100% reliable but should work for most cases
bool isRunningOnCemu() {
    static int cached_result = -1; // -1 = not checked, 0 = real hardware, 1 = Cemu
    
    if (cached_result != -1) {
        return cached_result == 1;
    }
    
    // Method 1: Check for Cemu-specific behavior
    // Cemu often has different memory layout or core count behavior
    
    // Method 2: Try to detect emulation artifacts
    // Real Wii U hardware has 3 cores, Cemu might behave differently
    uint32_t coreCount = OSGetCoreCount();
    if (coreCount != 3) {
        cached_result = 1; // Likely Cemu
        return true;
    }
    
    // Method 3: Check for Cemu-specific modules or behaviors
    // This is a simple heuristic - assume real hardware for now
    // In practice, you might check for specific memory patterns,
    // timing behaviors, or other emulation artifacts
    
    cached_result = 0; // Assume real hardware
    return false;
}

#else
bool isRunningOnCemu() {
    return false; // Not Wii U platform
}
#endif
