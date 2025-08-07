#pragma once

// Cemu compatibility macros
// These functions may not be supported or behave differently in Cemu emulator

#ifdef __CEMU_COMPATIBILITY__
    // For Cemu builds, disable potentially problematic functions
    #define CEMU_TRY_FUNCTION(func) \
        do { \
            /* func silently skipped for Cemu compatibility */ \
        } while(0)
        
    #define CEMU_TRY_FUNCTION_RETURN(func, retval) \
        do { \
            /* func silently skipped for Cemu compatibility */ \
            return retval; \
        } while(0)
#else
    // For real hardware, execute normally
    #define CEMU_TRY_FUNCTION(func) func
    #define CEMU_TRY_FUNCTION_RETURN(func, retval) func
#endif

// Runtime detection (if available)
#ifdef __WIIU__
    extern "C" bool isRunningOnCemu();
    
    #define CEMU_SAFE_CALL(func) \
        do { \
            if (isRunningOnCemu()) { \
                /* Skip function on Cemu */ \
            } else { \
                func; \
            } \
        } while(0)
#else
    #define CEMU_SAFE_CALL(func) func
#endif

// Specific problematic functions
#define SAFE_VPADInit() CEMU_TRY_FUNCTION(VPADInit())
#define SAFE_WPADEnableWiiRemote(enable) CEMU_TRY_FUNCTION(WPADEnableWiiRemote(enable))
#define SAFE_WPADEnableURCC(enable) CEMU_TRY_FUNCTION(WPADEnableURCC(enable))
#define SAFE_GX2SetTVEnable(enable) CEMU_TRY_FUNCTION(GX2SetTVEnable(enable))
#define SAFE_GX2SetTVScale(w, h) CEMU_TRY_FUNCTION(GX2SetTVScale(w, h))
#define SAFE_GX2SetDRCEnable(enable) CEMU_TRY_FUNCTION(GX2SetDRCEnable(enable))
