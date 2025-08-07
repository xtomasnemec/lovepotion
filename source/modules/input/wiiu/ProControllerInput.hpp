#pragma once

#include <string>
#include <vector>
#include <cstdint>

#ifdef __WIIU__
#include <padscore/wpad.h>
#endif

namespace love
{
    class ProControllerInput
    {
    public:
        ProControllerInput();
        ~ProControllerInput();

        // Core functionality
        void update();
        
        // Controller state queries
        bool isButtonPressed(int channel, const std::string& button) const;
        float getAxisValue(int channel, const std::string& axis) const;
        bool isControllerConnected(int channel) const;
        
        // Rumble support
        void setRumble(int channel, float strength);

    private:
        struct ControllerState
        {
            bool connected = false;
            uint32_t buttonState = 0;
            uint32_t prevButtonState = 0;
            struct { float x, y; } leftStick = {0.0f, 0.0f};
            struct { float x, y; } rightStick = {0.0f, 0.0f};
        };

        // Internal methods
        uint32_t getButtonMask(const std::string& button) const;
        float normalizeStickValue(int16_t value) const;

        // State
        ControllerState controllers[4];  // Support up to 4 Pro Controllers
    };
}
