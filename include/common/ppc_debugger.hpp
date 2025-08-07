#pragma once

#include <string>
#include <cstdarg>
#include <cstdint>

namespace love {
namespace ppc {

class PPCDebugger {
public:
    // Hardware-level debugging
    static void logGX2Operation(const char* operation, const char* format, ...);
    static void logFramebufferState();
    static void logVertexData(const void* data, size_t size);
    static void logShaderState();
    static void logPresentState();
    
    // Critical path tracking  
    static void markCriticalPath(const char* function, const char* file, int line);
    static void logMemoryState();
    static void logDrawCallDetails(const char* type, float x, float y, float w, float h);
    
    // Screen/viewport debugging
    static void logScreenState();
    static void logViewportState();
    static void logCoordinateTransform(float in_x, float in_y, float* out_x, float* out_y);
    
    // Force visible output
    static void forceVisiblePixel(int x, int y, uint32_t color);
    static void drawDebugGrid();
    static void testMinimalDraw();
    
private:
    static void writeToFile(const char* message);
    static char debug_buffer[4096];
};

// Macros for easy debugging
#define PPC_DEBUG_MARK() love::ppc::PPCDebugger::markCriticalPath(__FUNCTION__, __FILE__, __LINE__)
#define PPC_DEBUG_GX2(op, ...) love::ppc::PPCDebugger::logGX2Operation(op, __VA_ARGS__)
#define PPC_DEBUG_DRAW(type, x, y, w, h) love::ppc::PPCDebugger::logDrawCallDetails(type, x, y, w, h)
#define PPC_DEBUG_SCREEN() love::ppc::PPCDebugger::logScreenState()

} // namespace ppc
} // namespace love
