#include "GamepadInput.hpp"

#ifdef __WIIU__
#include <vpad/input.h>
#include <padscore/wpad.h>
#include <coreinit/time.h>
#include <coreinit/thread.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstring>

namespace love
{
    GamepadInput::GamepadInput() : 
        touchPressed(false), 
        touchX(0.0f), 
        touchY(0.0f),
        rumbleLeft(0.0f),
        rumbleRight(0.0f),
        rumbleEndTime(0)
    {
#ifdef __WIIU__
        VPADInit();
        WPADInit();
        
        // Initialize button mappings for Love2D compatibility
        initializeButtonMappings();
        
        // Clear status structures
        memset(&vpadStatus, 0, sizeof(VPADStatus));
        memset(&wpadStatus, 0, sizeof(WPADStatus) * 4);
        
        // Set up calibration for touch screen
        VPADSetGyroAngle(VPAD_CHAN_0, 0.0f, 0.0f, 0.0f);
        VPADSetGyroDirReviseBase(VPAD_CHAN_0, nullptr);
#endif
    }

    GamepadInput::~GamepadInput()
    {
#ifdef __WIIU__
        // Stop any active rumble
        uint8_t rumblePattern[15] = {0}; // All zeros = no rumble
        VPADControlMotor(VPAD_CHAN_0, rumblePattern, 0);
        
        VPADShutdown();
        WPADShutdown();
#endif
    }

    void GamepadInput::initializeButtonMappings()
    {
#ifdef __WIIU__
        // Map Love2D button names to VPAD constants
        vpadButtonMap["a"] = VPAD_BUTTON_A;
        vpadButtonMap["b"] = VPAD_BUTTON_B;
        vpadButtonMap["x"] = VPAD_BUTTON_X;
        vpadButtonMap["y"] = VPAD_BUTTON_Y;
        vpadButtonMap["back"] = VPAD_BUTTON_MINUS;
        vpadButtonMap["start"] = VPAD_BUTTON_PLUS;
        vpadButtonMap["leftstick"] = VPAD_BUTTON_STICK_L;
        vpadButtonMap["rightstick"] = VPAD_BUTTON_STICK_R;
        vpadButtonMap["leftshoulder"] = VPAD_BUTTON_L;
        vpadButtonMap["rightshoulder"] = VPAD_BUTTON_R;
        vpadButtonMap["dpup"] = VPAD_BUTTON_UP;
        vpadButtonMap["dpdown"] = VPAD_BUTTON_DOWN;
        vpadButtonMap["dpleft"] = VPAD_BUTTON_LEFT;
        vpadButtonMap["dpright"] = VPAD_BUTTON_RIGHT;
        vpadButtonMap["lefttrigger"] = VPAD_BUTTON_ZL;
        vpadButtonMap["righttrigger"] = VPAD_BUTTON_ZR;
        vpadButtonMap["guide"] = VPAD_BUTTON_HOME;
#endif
    }

    bool GamepadInput::isGamepadDown(int gamepad, const std::string& button)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            auto it = vpadButtonMap.find(button);
            if (it != vpadButtonMap.end())
            {
                return vpadStatus.trigger & it->second;
            }
        }
        else if (gamepad >= 1 && gamepad <= 4) // Wii Remote controllers
        {
            int wpadIndex = gamepad - 1;
            WPADStatusProController status;
            WPADExtensionType extType;
            
            if (WPADProbe((WPADChan)wpadIndex, &extType) == 0)
            {
                if (extType == WPAD_EXT_PRO_CONTROLLER)
                {
                    if (WPADReadProController((WPADChan)wpadIndex, &status) > 0)
                    {
                        auto it = vpadButtonMap.find(button);
                        if (it != vpadButtonMap.end())
                        {
                            uint32_t wpadButton = mapVPADToWPAD(it->second, WPAD_EXT_PRO_CONTROLLER);
                            return status.trigger & wpadButton;
                        }
                    }
                }
            }
        }
#endif
        return false;
    }

    bool GamepadInput::isGamepadUp(int gamepad, const std::string& button)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            auto it = vpadButtonMap.find(button);
            if (it != vpadButtonMap.end())
            {
                return vpadStatus.release & it->second;
            }
        }
        else if (gamepad >= 1 && gamepad <= 4) // Wii Remote controllers
        {
            int wpadIndex = gamepad - 1;
            WPADStatusProController status;
            WPADExtensionType extType;
            
            if (WPADProbe((WPADChan)wpadIndex, &extType) == 0)
            {
                if (extType == WPAD_EXT_PRO_CONTROLLER)
                {
                    if (WPADReadProController((WPADChan)wpadIndex, &status) > 0)
                    {
                        auto it = vpadButtonMap.find(button);
                        if (it != vpadButtonMap.end())
                        {
                            uint32_t wpadButton = mapVPADToWPAD(it->second, WPAD_EXT_PRO_CONTROLLER);
                            return status.release & wpadButton;
                        }
                    }
                }
            }
        }
#endif
        return false;
    }

    bool GamepadInput::isGamepadPressed(int gamepad, const std::string& button)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            auto it = vpadButtonMap.find(button);
            if (it != vpadButtonMap.end())
            {
                return vpadStatus.hold & it->second;
            }
        }
        else if (gamepad >= 1 && gamepad <= 4) // Wii Remote controllers
        {
            int wpadIndex = gamepad - 1;
            WPADStatusProController status;
            WPADExtensionType extType;
            
            if (WPADProbe((WPADChan)wpadIndex, &extType) == 0)
            {
                if (extType == WPAD_EXT_PRO_CONTROLLER)
                {
                    if (WPADReadProController((WPADChan)wpadIndex, &status) > 0)
                    {
                        auto it = vpadButtonMap.find(button);
                        if (it != vpadButtonMap.end())
                        {
                            uint32_t wpadButton = mapVPADToWPAD(it->second, WPAD_EXT_PRO_CONTROLLER);
                            return status.hold & wpadButton;
                        }
                    }
                }
            }
        }
#endif
        return false;
    }

    float GamepadInput::getGamepadAxis(int gamepad, const std::string& axis)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            if (axis == "leftx") return vpadStatus.leftStick.x;
            if (axis == "lefty") return -vpadStatus.leftStick.y; // Invert Y
            if (axis == "rightx") return vpadStatus.rightStick.x;
            if (axis == "righty") return -vpadStatus.rightStick.y; // Invert Y
            if (axis == "lefttrigger") return (vpadStatus.hold & VPAD_BUTTON_ZL) ? 1.0f : 0.0f;
            if (axis == "righttrigger") return (vpadStatus.hold & VPAD_BUTTON_ZR) ? 1.0f : 0.0f;
        }
        else if (gamepad >= 1 && gamepad <= 4) // Wii Remote/Pro Controller
        {
            int wpadIndex = gamepad - 1;
            WPADStatusProController status;
            WPADExtensionType extType;
            
            if (WPADProbe((WPADChan)wpadIndex, &extType) == 0)
            {
                if (extType == WPAD_EXT_PRO_CONTROLLER)
                {
                    if (WPADReadProController((WPADChan)wpadIndex, &status) > 0)
                    {
                        if (axis == "leftx") return status.leftStick.x;
                        if (axis == "lefty") return -status.leftStick.y;
                        if (axis == "rightx") return status.rightStick.x;
                        if (axis == "righty") return -status.rightStick.y;
                    }
                }
            }
        }
#endif
        return 0.0f;
    }

    int GamepadInput::getGamepadCount()
    {
#ifdef __WIIU__
        int count = 1; // GamePad is always present
        
        // Check for connected Wii Remote controllers
        for (int i = 0; i < 4; i++)
        {
            WPADExtensionType extType;
            if (WPADProbe((WPADChan)i, &extType) == 0)
            {
                count++;
            }
        }
        
        return count;
#else
        return 0;
#endif
    }

    std::string GamepadInput::getGamepadName(int gamepad)
    {
#ifdef __WIIU__
        if (gamepad == 0)
            return "Wii U GamePad";
        else if (gamepad >= 1 && gamepad <= 4)
        {
            int wpadIndex = gamepad - 1;
            WPADExtensionType extType;
            if (WPADProbe((WPADChan)wpadIndex, &extType) == 0)
            {
                switch (extType)
                {
                    case WPAD_EXT_PRO_CONTROLLER:
                        return "Wii U Pro Controller";
                    case WPAD_EXT_NUNCHUK:
                        return "Wii Remote + Nunchuk";
                    case WPAD_EXT_CLASSIC:
                        return "Wii Classic Controller";
                    default:
                        return "Wii Remote";
                }
            }
        }
#endif
        return "Unknown";
    }

    bool GamepadInput::isTouchDown()
    {
#ifdef __WIIU__
        return vpadStatus.tpNormal.touched;
#else
        return false;
#endif
    }

    std::pair<float, float> GamepadInput::getTouchPosition()
    {
#ifdef __WIIU__
        if (vpadStatus.tpNormal.touched)
        {
            float x = vpadStatus.tpNormal.x / 1280.0f; // Normalize to 0-1
            float y = vpadStatus.tpNormal.y / 720.0f;  // Normalize to 0-1
            return std::make_pair(x, y);
        }
#endif
        return std::make_pair(0.0f, 0.0f);
    }

    bool GamepadInput::isGamepadScreenDown()
    {
        return isTouchDown();
    }

    std::pair<float, float> GamepadInput::getGamepadScreenPosition()
    {
        return getTouchPosition();
    }

    std::tuple<float, float, float> GamepadInput::getAccelerometer()
    {
#ifdef __WIIU__
        return std::make_tuple(vpadStatus.accelorometer.acc.x,
                              vpadStatus.accelorometer.acc.y,
                              vpadStatus.accelorometer.acc.z);
#else
        return std::make_tuple(0.0f, 0.0f, 0.0f);
#endif
    }

    std::tuple<float, float, float> GamepadInput::getGyroscope()
    {
#ifdef __WIIU__
        return std::make_tuple(vpadStatus.gyro.x, vpadStatus.gyro.y, vpadStatus.gyro.z);
#else
        return std::make_tuple(0.0f, 0.0f, 0.0f);
#endif
    }

    void GamepadInput::setRumble(int gamepad, float leftMotor, float rightMotor, float duration)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            // Clamp values to 0-1 range
            leftMotor = std::max(0.0f, std::min(1.0f, leftMotor));
            rightMotor = std::max(0.0f, std::min(1.0f, rightMotor));
            
            rumbleLeft = leftMotor;
            rumbleRight = rightMotor;
            
            // Set duration
            if (duration > 0)
            {
                rumbleEndTime = OSGetTime() + OSSecondsToTicks(duration);
            }
            else
            {
                rumbleEndTime = 0; // Infinite duration
            }
            
            // Create rumble pattern
            uint8_t rumblePattern[15];
            memset(rumblePattern, 0, sizeof(rumblePattern));
            
            // Simple rumble pattern based on motor values
            float avgRumble = (leftMotor + rightMotor) * 0.5f;
            if (avgRumble > 0.0f)
            {
                // Create a simple on-off pattern
                for (int i = 0; i < 15; i++)
                {
                    rumblePattern[i] = (uint8_t)(avgRumble * 255);
                }
            }
            
            VPADControlMotor(VPAD_CHAN_0, rumblePattern, 0);
        }
        // Pro Controller rumble would need different implementation
#endif
    }

    void GamepadInput::stopRumble(int gamepad)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            rumbleLeft = rumbleRight = 0.0f;
            rumbleEndTime = 0;
            
            uint8_t rumblePattern[15] = {0}; // All zeros = no rumble
            VPADControlMotor(VPAD_CHAN_0, rumblePattern, 0);
        }
#endif
    }

    std::pair<float, float> GamepadInput::getRumble(int gamepad)
    {
#ifdef __WIIU__
        if (gamepad == 0) // GamePad
        {
            return std::make_pair(rumbleLeft, rumbleRight);
        }
#endif
        return std::make_pair(0.0f, 0.0f);
    }

    uint32_t GamepadInput::mapVPADToWPAD(uint32_t vpadButton, uint32_t extensionType)
    {
#ifdef __WIIU__
        // Map VPAD buttons to WPAD buttons based on extension type
        switch (extensionType)
        {
            case WPAD_EXT_PRO_CONTROLLER:
                switch (vpadButton)
                {
                    case VPAD_BUTTON_A: return WPAD_PRO_BUTTON_A;
                    case VPAD_BUTTON_B: return WPAD_PRO_BUTTON_B;
                    case VPAD_BUTTON_X: return WPAD_PRO_BUTTON_X;
                    case VPAD_BUTTON_Y: return WPAD_PRO_BUTTON_Y;
                    case VPAD_BUTTON_L: return WPAD_PRO_TRIGGER_L;
                    case VPAD_BUTTON_R: return WPAD_PRO_TRIGGER_R;
                    case VPAD_BUTTON_ZL: return WPAD_PRO_TRIGGER_ZL;
                    case VPAD_BUTTON_ZR: return WPAD_PRO_TRIGGER_ZR;
                    case VPAD_BUTTON_PLUS: return WPAD_PRO_BUTTON_PLUS;
                    case VPAD_BUTTON_MINUS: return WPAD_PRO_BUTTON_MINUS;
                    case VPAD_BUTTON_HOME: return WPAD_PRO_BUTTON_HOME;
                    case VPAD_BUTTON_UP: return WPAD_PRO_BUTTON_UP;
                    case VPAD_BUTTON_DOWN: return WPAD_PRO_BUTTON_DOWN;
                    case VPAD_BUTTON_LEFT: return WPAD_PRO_BUTTON_LEFT;
                    case VPAD_BUTTON_RIGHT: return WPAD_PRO_BUTTON_RIGHT;
                    case VPAD_BUTTON_STICK_L: return WPAD_PRO_BUTTON_STICK_L;
                    case VPAD_BUTTON_STICK_R: return WPAD_PRO_BUTTON_STICK_R;
                    default: return 0;
                }
                break;
                
            case WPAD_EXT_NUNCHUK:
                switch (vpadButton)
                {
                    case VPAD_BUTTON_A: return WPAD_BUTTON_A;
                    case VPAD_BUTTON_B: return WPAD_BUTTON_B;
                    case VPAD_BUTTON_PLUS: return WPAD_BUTTON_PLUS;
                    case VPAD_BUTTON_MINUS: return WPAD_BUTTON_MINUS;
                    case VPAD_BUTTON_HOME: return WPAD_BUTTON_HOME;
                    case VPAD_BUTTON_UP: return WPAD_BUTTON_UP;
                    case VPAD_BUTTON_DOWN: return WPAD_BUTTON_DOWN;
                    case VPAD_BUTTON_LEFT: return WPAD_BUTTON_LEFT;
                    case VPAD_BUTTON_RIGHT: return WPAD_BUTTON_RIGHT;
                    case VPAD_BUTTON_ZL: return WPAD_NUNCHUK_BUTTON_Z;
                    case VPAD_BUTTON_ZR: return WPAD_NUNCHUK_BUTTON_C;
                    default: return 0;
                }
                break;
                
            default:
                // Basic Wii Remote
                switch (vpadButton)
                {
                    case VPAD_BUTTON_A: return WPAD_BUTTON_A;
                    case VPAD_BUTTON_B: return WPAD_BUTTON_B;
                    case VPAD_BUTTON_X: return WPAD_BUTTON_1;
                    case VPAD_BUTTON_Y: return WPAD_BUTTON_2;
                    case VPAD_BUTTON_PLUS: return WPAD_BUTTON_PLUS;
                    case VPAD_BUTTON_MINUS: return WPAD_BUTTON_MINUS;
                    case VPAD_BUTTON_HOME: return WPAD_BUTTON_HOME;
                    case VPAD_BUTTON_UP: return WPAD_BUTTON_UP;
                    case VPAD_BUTTON_DOWN: return WPAD_BUTTON_DOWN;
                    case VPAD_BUTTON_LEFT: return WPAD_BUTTON_LEFT;
                    case VPAD_BUTTON_RIGHT: return WPAD_BUTTON_RIGHT;
                    default: return 0;
                }
                break;
        }
#endif
        return 0;
    }

    void GamepadInput::setButtonMapping(const std::string& button, int mapping)
    {
#ifdef __WIIU__
        vpadButtonMap[button] = mapping;
#endif
    }

    std::string GamepadInput::getButtonMapping(int button)
    {
#ifdef __WIIU__
        for (const auto& pair : vpadButtonMap)
        {
            if (pair.second == button)
                return pair.first;
        }
#endif
        return "";
    }

    bool GamepadInput::hasGamepadSupport()
    {
#ifdef __WIIU__
        return true;
#else
        return false;
#endif
    }

    void GamepadInput::update()
    {
#ifdef __WIIU__
        // Read GamePad status
        VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadError);
        
        // Handle rumble timeout
        if (rumbleEndTime > 0 && OSGetTime() >= rumbleEndTime)
        {
            stopRumble(0);
            rumbleEndTime = 0;
            rumbleLeft = rumbleRight = 0.0f;
        }
        
        // Update touch state
        touchPressed = vpadStatus.tpNormal.touched;
        if (touchPressed)
        {
            touchX = vpadStatus.tpNormal.x / 1280.0f;
            touchY = vpadStatus.tpNormal.y / 720.0f;
        }
        
        // Read WPAD controllers
        for (int i = 0; i < 4; i++)
        {
            WPADRead((WPADChan)i, &wpadStatus[i]);
        }
#endif
    }
}
