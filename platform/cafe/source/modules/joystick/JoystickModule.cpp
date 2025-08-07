#include "modules/joystick/JoystickModule.hpp"

#include <padscore/kpad.h>
#include <vpad/input.h>

#include "modules/joystick/kpad/Joystick.hpp"
#include "modules/joystick/vpad/Joystick.hpp"
#include "DebugLogger.hpp"

namespace love::joystick
{
    int getJoystickCount()
    {
        DebugLogger::log("JoystickModule::getJoystickCount() called");
        int count = 0;

        VPADStatus vpadStatus {};
        VPADReadError error = VPAD_READ_SUCCESS;

        VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &error);
        DebugLogger::log("VPAD check - Error: %d", error);

        if (error == VPAD_READ_SUCCESS || error == VPAD_READ_NO_SAMPLES)
        {
            count++;
            DebugLogger::log("VPAD GamePad detected");
        }

        for (int channel = 0; channel < 4; channel++)
        {
            KPADStatus kpadStatus {};
            KPADError error = KPAD_ERROR_OK;

            KPADReadEx((KPADChan)channel, &kpadStatus, 1, &error);
            bool success = error == KPAD_ERROR_OK || error == KPAD_ERROR_NO_SAMPLES;
            
            DebugLogger::log("KPAD channel %d - Error: %d, ExtType: 0x%02X", channel, error, kpadStatus.extensionType);

            if (success && kpadStatus.extensionType != 0xFF)
            {
                count++;
                DebugLogger::log("KPAD controller detected on channel %d", channel);
            }
        }

        DebugLogger::log("Total joystick count: %d", count);
        return count;
    }

    JoystickBase* openJoystick(int index)
    {
        if (index == 0)
            return new vpad::Joystick(index);

        return new kpad::Joystick(index);
    }
} // namespace love::joystick
