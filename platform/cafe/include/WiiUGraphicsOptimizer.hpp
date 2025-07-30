// Wii U Performance Optimizations for Love Potion Graphics
// This patch reduces draw calls and improves batching for better performance

#pragma once

#ifdef __WIIU__

// Graphics performance optimizations for Wii U
class WiiUGraphicsOptimizer {
private:
    static int frameDrawCallCount;
    static int lastPrepareDrawFrame;
    static const int MAX_DRAW_CALLS_PER_FRAME = 30;  // Limit draw calls
    static const int MAX_PREPARE_DRAW_PER_FRAME = 10; // Limit prepareDraw calls
    static bool emergencySkipMode; // Emergency mode to skip all draws
    
public:
    // Check if we should skip this draw call to prevent performance issues
    static bool shouldSkipDrawCall() {
        frameDrawCallCount++;
        
        // Emergency mode: skip all draws if we're in trouble
        if (emergencySkipMode) {
            return true;
        }
        
        // Regular mode: limit draw calls per frame
        if (frameDrawCallCount > MAX_DRAW_CALLS_PER_FRAME) {
            return true;
        }
        
        return false;
    }
    
    // Check if we should skip prepareDraw to reduce overhead
    static bool shouldSkipPrepareDraw(int currentFrame) {
        if (emergencySkipMode) {
            return true; // Skip all prepareDraw in emergency mode
        }
        
        if (currentFrame != lastPrepareDrawFrame) {
            lastPrepareDrawFrame = currentFrame;
            return false; // Allow first prepareDraw of the frame
        }
        // Skip subsequent prepareDraw calls in the same frame
        return true;
    }
    
    // Reset counters at the start of each frame
    static void startFrame() {
        // If we exceeded limits severely, enter emergency mode
        if (frameDrawCallCount > MAX_DRAW_CALLS_PER_FRAME * 2) {
            emergencySkipMode = true;
        } else if (frameDrawCallCount < MAX_DRAW_CALLS_PER_FRAME / 2) {
            // If we're below half the limit, we can exit emergency mode
            emergencySkipMode = false;
        }
        
        frameDrawCallCount = 0;
    }
    
    // Force flush when we hit limits to prevent memory issues
    static bool shouldForceFlush() {
        return frameDrawCallCount >= MAX_DRAW_CALLS_PER_FRAME - 5;
    }
    
    // Manual emergency mode control
    static void setEmergencyMode(bool enable) {
        emergencySkipMode = enable;
    }
    
    // Check current status
    static bool isInEmergencyMode() {
        return emergencySkipMode;
    }
};

#endif // __WIIU__
