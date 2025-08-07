#include "common/LoadingScreen.hpp"

#ifdef __WIIU__

#include <coreinit/screen.h>
#include <coreinit/time.h>
#include <coreinit/memheap.h>
#include <coreinit/memory.h>
#include <coreinit/thread.h>
#include <whb/proc.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

LoadingScreen* g_loadingScreen = nullptr;

LoadingScreen::LoadingScreen() : m_initialized(false)
{
}

LoadingScreen::~LoadingScreen()
{
    shutdown();
}

void LoadingScreen::initialize()
{
    if (m_initialized)
        return;
        
    // Initialize OSScreen for simple text rendering
    OSScreenInit();
    
    // Setup screen buffers for TV and GamePad
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    void* tvBuffer = malloc(tvBufferSize);
    void* drcBuffer = malloc(drcBufferSize);
    
    if (tvBuffer && drcBuffer)
    {
        OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
        OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
        
        OSScreenEnableEx(SCREEN_TV, 1);
        OSScreenEnableEx(SCREEN_DRC, 1);
        
        m_initialized = true;
        printf("Loading screen initialized using OSScreen\n");
    }
    else
    {
        printf("Failed to allocate screen buffers\n");
        if (tvBuffer) free(tvBuffer);
        if (drcBuffer) free(drcBuffer);
    }
}

void LoadingScreen::update(float progress, const char* message)
{
    if (!m_initialized)
        return;
        
    // Clamp progress to 0.0 - 1.0
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    
    // Clear screens to dark blue
    OSScreenClearBufferEx(SCREEN_TV, 0x003366FF);
    OSScreenClearBufferEx(SCREEN_DRC, 0x003366FF);
    
    // Draw title
    OSScreenPutFontEx(SCREEN_TV, 2, 2, "Balatro U - Loading...");
    OSScreenPutFontEx(SCREEN_DRC, 2, 2, "Balatro U - Loading...");
    
    // Draw loading message if provided
    if (message && strlen(message) > 0)
    {
        OSScreenPutFontEx(SCREEN_TV, 2, 4, message);
        OSScreenPutFontEx(SCREEN_DRC, 2, 4, message);
    }
    
    // Draw simple text-based progress bar
    char progressBar[81]; // 80 chars + null terminator
    int barLength = 60;
    int filledLength = (int)(barLength * progress);
    
    progressBar[0] = '[';
    for (int i = 0; i < barLength; i++)
    {
        if (i < filledLength)
            progressBar[i + 1] = '=';
        else if (i == filledLength && filledLength < barLength)
            progressBar[i + 1] = '>';
        else
            progressBar[i + 1] = ' ';
    }
    progressBar[barLength + 1] = ']';
    progressBar[barLength + 2] = '\0';
    
    // Draw progress bar
    OSScreenPutFontEx(SCREEN_TV, 2, 10, progressBar);
    OSScreenPutFontEx(SCREEN_DRC, 2, 10, progressBar);
    
    // Draw progress percentage
    char progressText[32];
    snprintf(progressText, sizeof(progressText), "%.1f%% Complete", progress * 100.0f);
    OSScreenPutFontEx(SCREEN_TV, 2, 12, progressText);
    OSScreenPutFontEx(SCREEN_DRC, 2, 12, progressText);
    
    // Present the frame
    present();
}

void LoadingScreen::shutdown()
{
    if (!m_initialized)
        return;
        
    // OSScreen cleanup would go here, but it's safer to leave it active
    // since we don't want to interfere with the main graphics system
        
    m_initialized = false;
    printf("Loading screen shutdown\n");
}

void LoadingScreen::drawRect(float x, float y, float width, float height, 
                            float r, float g, float b, float a)
{
    // Not implemented for OSScreen - would need proper graphics context
}

void LoadingScreen::drawText(float x, float y, const char* text, float r, float g, float b)
{
    // Use OSScreen text rendering
    int textX = (int)(x / 12); // Approximate character width
    int textY = (int)(y / 24); // Approximate character height
    
    OSScreenPutFontEx(SCREEN_TV, textX, textY, text);
    OSScreenPutFontEx(SCREEN_DRC, textX, textY, text);
}

void LoadingScreen::present()
{
    // Flip screen buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    
    // Small delay to make the loading screen visible
    OSYieldThread(); // Yield to other threads instead of sleep
}

void LoadingScreen::setupRenderState()
{
    // Not needed for OSScreen
}

// Global convenience functions
void initLoadingScreen()
{
    if (!g_loadingScreen)
    {
        g_loadingScreen = new LoadingScreen();
        g_loadingScreen->initialize();
    }
}

void updateLoadingScreen(float progress, const char* message)
{
    if (g_loadingScreen)
    {
        g_loadingScreen->update(progress, message);
    }
}

void shutdownLoadingScreen()
{
    if (g_loadingScreen)
    {
        g_loadingScreen->shutdown();
        delete g_loadingScreen;
        g_loadingScreen = nullptr;
    }
}

#endif // __WIIU__
