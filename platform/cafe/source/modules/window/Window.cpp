#include "modules/window/Window.hpp"
#include "DebugLogger.hpp"
#include "driver/EventQueue.hpp"

#include <coreinit/energysaver.h>

namespace love
{
    Window::Window() : WindowBase("love.window.gx2")
    {
        DebugLogger::log("Window constructor called");
        
        // Set default window size for Wii U to match graphics system
        this->windowWidth = 800;   // Match what graphics system expects
        this->windowHeight = 600;  // Match what graphics system expects
        this->pixelWidth = 800;
        this->pixelHeight = 600;
        this->open = true;  // Set to true immediately for Wii U
        
        this->setDisplaySleepEnabled(false);
        DebugLogger::log("Window constructor completed - default window %dx%d, open=%s", 
                        this->windowWidth, this->windowHeight, this->open ? "true" : "false");
        
        // Send initial resize event to Lua to ensure love.resize is called during startup
        DebugLogger::log("Sending initial resize event from constructor: %dx%d", this->windowWidth, this->windowHeight);
        EventQueue::getInstance().sendResize(this->windowWidth, this->windowHeight);
        DebugLogger::log("Initial resize event sent from constructor");
    }

    Window::~Window()
    {
        this->close(false);
        this->graphics.set(nullptr);
    }

    void Window::close()
    {
        this->close(true);
    }

    void Window::close(bool allowExceptions)
    {
        if (this->graphics.get())
        {
            if (allowExceptions && this->graphics->isRenderTargetActive())
                throw love::Exception(E_WINDOW_CLOSING_RENDERTARGET_ACTIVE);

            this->graphics->unsetMode();
        }

        this->open = false;
    }

    bool Window::setWindow(int width, int height, WindowSettings* settings)
    {
        DebugLogger::log("Window::setWindow() called with %dx%d (SIMPLIFIED)", width, height);
        
        // Simplified approach for Wii U - just set dimensions and state
        this->windowWidth = width;
        this->windowHeight = height;
        this->pixelWidth = width;
        this->pixelHeight = height;
        this->open = true;
        
        DebugLogger::log("Window dimensions set to %dx%d", width, height);
        
        // Update settings if provided
        if (settings) {
            this->updateSettings(*settings, false);
            DebugLogger::log("Window settings updated");
        }
        
        // Get graphics module and set mode
        if (!this->graphics.get())
        {
            DebugLogger::log("Getting graphics module instance...");
            this->graphics.set(Module::getInstance<GraphicsBase>(Module::M_GRAPHICS));
        }

        if (this->graphics.get())
        {
            DebugLogger::log("Setting graphics mode to %dx%d", width, height);
            this->graphics->setMode(width, height, width, height, false, false, 0);
            DebugLogger::log("Graphics mode set successfully");
            
            // Send resize event to Lua to trigger love.resize callback
            DebugLogger::log("Sending resize event to Lua: %dx%d", width, height);
            EventQueue::getInstance().sendResize(width, height);
            DebugLogger::log("Resize event sent successfully");
        }
        else
        {
            DebugLogger::log("WARNING: No graphics module available");
        }

        DebugLogger::log("Window::setWindow() completed successfully (SIMPLIFIED)");
        return true;
    }

    void Window::updateSettings(const WindowSettings& settings, bool updateGraphicsViewport)
    {
        WindowBase::updateSettings(settings, updateGraphicsViewport);

        if (updateGraphicsViewport && this->graphics.get())
        {
            double scaledw, scaledh;
            this->fromPixels(this->pixelWidth, this->pixelHeight, scaledw, scaledh);

            this->graphics->backbufferChanged(0, 0, scaledw, scaledh);
        }
    }

    bool Window::onSizeChanged(int width, int height)
    {
        return false;
    }

    void Window::setDisplaySleepEnabled(bool enable)
    {
        // CEMU COMPATIBILITY: IMDisableDim causes problems in Cemu
        // enable ? IMEnableDim() : IMDisableDim();
        // Function disabled for Cemu compatibility
    }

    bool Window::isDisplaySleepEnabled() const
    {
        // CEMU COMPATIBILITY: IMIsDimEnabled might cause problems in Cemu
        // uint32_t enabled;
        // IMIsDimEnabled(&enabled);
        // return enabled;
        return false; // Return false for Cemu compatibility
    }
} // namespace love
