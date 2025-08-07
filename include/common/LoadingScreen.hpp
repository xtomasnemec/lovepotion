#pragma once

#ifdef __WIIU__

#include <coreinit/screen.h>
#include <coreinit/time.h>

class LoadingScreen 
{
public:
    LoadingScreen();
    ~LoadingScreen();
    
    // Initialize the loading screen
    void initialize();
    
    // Update and draw the loading screen with progress (0.0 - 1.0)
    void update(float progress, const char* message = nullptr);
    
    // Clean up
    void shutdown();
    
    // Check if initialized
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized;
    
    // Draw a rectangle with specified color (simplified for OSScreen)
    void drawRect(float x, float y, float width, float height, 
                  float r, float g, float b, float a = 1.0f);
    
    // Draw text using OSScreen
    void drawText(float x, float y, const char* text, 
                  float r = 1.0f, float g = 1.0f, float b = 1.0f);
    
    // Present the frame
    void present();
    
    // Setup basic rendering state (not needed for OSScreen)
    void setupRenderState();
};

// Global loading screen instance
extern LoadingScreen* g_loadingScreen;

// Convenience functions
void initLoadingScreen();
void updateLoadingScreen(float progress, const char* message = nullptr);
void shutdownLoadingScreen();

#endif // __WIIU__
