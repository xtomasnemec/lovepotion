#pragma once

#include <map>
#include <string>
#include <utility>
#include <tuple>

#ifdef __WIIU__
#include <vpad/input.h>
#include <padscore/wpad.h>
#endif

namespace love
{
    class GamepadInput
    {
    public:
        GamepadInput();
        ~GamepadInput();
        
        // Core input functions
        bool isGamepadDown(int gamepad, const std::string& button);
        bool isGamepadUp(int gamepad, const std::string& button);
        bool isGamepadPressed(int gamepad, const std::string& button);
        float getGamepadAxis(int gamepad, const std::string& axis);
        
        // Gamepad management
        int getGamepadCount();
        std::string getGamepadName(int gamepad);
        bool hasGamepadSupport();
        
        // Wii U GamePad specific functions
        bool isTouchDown();
        std::pair<float, float> getTouchPosition();
        
        // Motion controls
        std::tuple<float, float, float> getAccelerometer();
        std::tuple<float, float, float> getGyroscope();
        
        // Rumble
        void setRumble(int gamepad, float strength);
        void stopRumble(int gamepad);
        
        void update();
        
    private:
        // Helper functions
        void initializeButtonMappings();
        uint32_t getVPADButtonBit(const std::string& button);
        uint32_t getWPADButtonBit(const std::string& button);
        
#ifdef __WIIU__
        VPADStatus vpadStatus;
        VPADReadError vpadError;
        WPADStatus wpadStatus[4];
        
        // Previous states for button press detection
        uint32_t prevVPADButtons;
        uint32_t prevWPADButtons[4];
        
        // Button mappings
        std::map<std::string, uint32_t> vpadButtonMap;
        std::map<std::string, uint32_t> wpadButtonMap;
#endif
    };
}
