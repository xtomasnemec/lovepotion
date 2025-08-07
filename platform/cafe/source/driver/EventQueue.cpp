#include "driver/EventQueue.hpp"

#include "modules/joystick/JoystickModule.hpp"
#include "modules/keyboard/Keyboard.hpp"

#include "utility/guid.hpp"
#include "DebugLogger.hpp"

#include <nn/swkbd.h>

using namespace love;

#define JOYSTICK_MODULE() Module::getInstance<JoystickModule>(Module::M_JOYSTICK)
#define KEYBOARD_MODULE() Module::getInstance<Keyboard>(Module::M_KEYBOARD)

namespace love
{
    EventQueue::EventQueue() : gamepad(nullptr), previousTouch {}, wasTouched(false)
    {}

    EventQueue::~EventQueue()
    {}

    void checkSoftwareKeyboard(VPADStatus& status)
    {
        VPADGetTPCalibratedPoint(VPAD_CHAN_0, &status.tpNormal, &status.tpNormal);

        nn::swkbd::ControllerInfo info {};
        info.vpad = &status;

        nn::swkbd::Calc(info);

        if (nn::swkbd::IsNeedCalcSubThreadFont())
            nn::swkbd::CalcSubThreadFont();

        if (nn::swkbd::IsNeedCalcSubThreadPredict())
            nn::swkbd::CalcSubThreadPredict();

        bool okPressed     = nn::swkbd::IsDecideOkButton(nullptr);
        bool cancelPressed = nn::swkbd::IsDecideCancelButton(nullptr);

        if (okPressed || cancelPressed)
        {
            KEYBOARD_MODULE()->hide();

            if (okPressed)
                EventQueue::getInstance().sendTextInput(KEYBOARD_MODULE()->getText());
        }
    }

    void EventQueue::pollInternal()
    {
        DebugLogger::log("EventQueue::pollInternal() called");
        
        if (!JOYSTICK_MODULE())
        {
            DebugLogger::log("EventQueue: No joystick module available");
            return;
        }

        int joystickCount = JOYSTICK_MODULE()->getJoystickCount();
        DebugLogger::log("EventQueue: Joystick count = %d", joystickCount);
        
        if (joystickCount == 0)
        {
            static bool warned = false;
            if (!warned)
            {
                DebugLogger::log("EventQueue: No joysticks available for polling");
                warned = true;
            }
            return;
        }

        for (int index = 0; index < joystickCount; index++)
        {
            auto* joystick = JOYSTICK_MODULE()->getJoystick(index);

            if (joystick == nullptr)
            {
                DebugLogger::log("EventQueue: Joystick %d is null", index);
                continue;
            }

            DebugLogger::log("EventQueue: Polling joystick %d (type=%d)", index, joystick->getGamepadType());

            if (joystick->getGamepadType() == GAMEPAD_TYPE_NINTENDO_WII_U_GAMEPAD)
                this->gamepad = (vpad::Joystick*)joystick;

            joystick->update();
            const auto which = joystick->getInstanceID();

            for (int input = 0; input < JoystickBase::GAMEPAD_BUTTON_MAX_ENUM; input++)
            {
                std::vector<JoystickBase::GamepadButton> inputs = { JoystickBase::GamepadButton(input) };

                if (joystick->isDown(inputs))
                {
                    DebugLogger::log("EventQueue: Button %d pressed on joystick %d", input, which);
                    this->sendGamepadButtonEvent(SUBTYPE_GAMEPADDOWN, which, input);
                }

                if (joystick->isUp(inputs))
                {
                    DebugLogger::log("EventQueue: Button %d released on joystick %d", input, which);
                    this->sendGamepadButtonEvent(SUBTYPE_GAMEPADUP, which, input);
                }
            }

            for (int input = 0; input < JoystickBase::GAMEPAD_AXIS_MAX_ENUM; input++)
            {
                if (joystick->isAxisChanged(JoystickBase::GamepadAxis(input)))
                {
                    float value = joystick->getAxis(JoystickBase::GamepadAxis(input));
                    DebugLogger::log("EventQueue: Axis %d changed to %f on joystick %d", input, value, which);
                    this->sendGamepadAxisEvent(which, input, value);
                }
            }

            for (int input = 0; input < Sensor::SENSOR_MAX_ENUM; input++)
            {
                if (!joystick->isSensorEnabled(Sensor::SensorType(input)))
                    continue;

                auto data = joystick->getSensorData(Sensor::SensorType(input));
                this->sendGamepadSensorEvent(which, input, data);
            }
        }

        if (this->gamepad)
        {
            auto& status = this->gamepad->getVPADStatus();
            SubEventType touchType;

            if (status.tpNormal.touched)
            {

                VPADTouchData data {};
                VPADGetTPCalibratedPointEx(VPAD_CHAN_0, VPAD_TP_854X480, &data, &status.tpNormal);

                float x = data.x, y = data.y;
                float dx = 0, dy = 0;

                dx = (data.x - this->previousTouch.x);
                dy = (data.y - this->previousTouch.y);

                if (dx == 0 && dy == 0)
                    touchType = SUBTYPE_TOUCHPRESS;
                else
                    touchType = SUBTYPE_TOUCHMOVED;

                this->sendTouchEvent(touchType, 0, x, y, dx, dy, 1.0f);
                this->previousTouch = status.tpNormal;

                if (touchType == SUBTYPE_TOUCHMOVED && !dx && !dy)
                    this->events.pop_back();

                this->wasTouched = true;
            }
            else
            {
                if (!this->wasTouched)
                    return;

                const auto& previous = this->previousTouch;
                this->sendTouchEvent(SUBTYPE_TOUCHRELEASE, 0, previous.x, previous.y, 0, 0, 1.0f);
                this->wasTouched = false;
            }
        }
    }
} // namespace love
