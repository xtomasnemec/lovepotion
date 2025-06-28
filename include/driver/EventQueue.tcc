#pragma once

#include "common/Singleton.tcc"
#include "events.hpp"

#include "modules/joystick/Joystick.tcc"

#if defined(__WIIU__)
#include "DebugLogger.hpp"
#endif

#include <list>
#include <memory>
#include <vector>

namespace love
{
    template<class T>
    class EventQueueBase : public Singleton<T>
    {
      public:
        EventQueueBase() : hysteresis(false), touchHeld(false), focused(false), events {}
        {}

        bool poll(LOVE_Event* event)
        {
            static int pollCount = 0;
            pollCount++;
            
            if (!this->events.empty())
            {
                if (pollCount % 60 == 1) // Log every 60 calls
                {
#if defined(__WIIU__)
                    DebugLogger::log("EventQueue::poll #%d: returning cached event (type=%d)", pollCount, this->events.front().type);
#else
                    printf("[DEBUG] EventQueue::poll #%d: returning cached event (type=%d)\n", pollCount, this->events.front().type);
#endif
                }
                *event = this->events.front();
                this->events.pop_front();
                return true;
            }

            if (this->hysteresis)
            {
                if (pollCount % 60 == 1)
                {
#if defined(__WIIU__)
                    DebugLogger::log("EventQueue::poll #%d: hysteresis false", pollCount);
#else
                    printf("[DEBUG] EventQueue::poll #%d: hysteresis false\n", pollCount);
#endif
                }
                return this->hysteresis = false;
            }

            if (pollCount % 60 == 1)
            {
#if defined(__WIIU__)
                DebugLogger::log("EventQueue::poll #%d: calling pollInternal()", pollCount);
#else
                printf("[DEBUG] EventQueue::poll #%d: calling pollInternal()\n", pollCount);
#endif
            }
            
            this->pollInternal();

            if (this->events.empty())
            {
                if (pollCount % 60 == 1)
                {
#if defined(__WIIU__)
                    DebugLogger::log("EventQueue::poll #%d: no events after pollInternal", pollCount);
#else
                    printf("[DEBUG] EventQueue::poll #%d: no events after pollInternal\n", pollCount);
#endif
                }
                return false;
            }

            if (pollCount % 60 == 1)
            {
#if defined(__WIIU__)
                DebugLogger::log("EventQueue::poll #%d: returning new event (type=%d)", pollCount, this->events.front().type);
#else
                printf("[DEBUG] EventQueue::poll #%d: returning new event (type=%d)\n", pollCount, this->events.front().type);
#endif
            }
            *event = this->events.front();
            this->events.pop_front();

            return this->hysteresis = true;
        }

        void sendFocus(bool focus)
        {
            if (this->focused == focus)
                return;

            auto& event = this->events.emplace_back();

            event.type    = TYPE_WINDOW;
            event.subtype = focus ? SUBTYPE_FOCUS_GAINED : SUBTYPE_FOCUS_LOST;
            this->focused = focus;
        }

        void sendQuit()
        {
            auto& event = this->events.emplace_back();

            event.type    = TYPE_GENERAL;
            event.subtype = SUBTYPE_QUIT;
        }

        void sendResize(int width, int height)
        {
            auto& event = this->events.emplace_back();

            event.type    = TYPE_WINDOW;
            event.subtype = SUBTYPE_RESIZE;
            event.resize  = { width, height };
        }

        void sendJoystickStatus(bool added, int which)
        {
            auto& event = this->events.emplace_back();

            event.type    = TYPE_GAMEPAD;
            event.subtype = added ? SUBTYPE_GAMEPADADDED : SUBTYPE_GAMEPADREMOVED;

            event.gamepadStatus.added = added;
            event.gamepadStatus.which = which;
        }

        void sendTextInput(std::unique_ptr<char[]>& text)
        {
            auto& event = this->events.emplace_back();

            event.type               = TYPE_KEYBOARD;
            event.subtype            = SUBTYPE_TEXTINPUT;
            event.keyboardInput.text = text.get();
        }

        void sendTextInput(std::string_view text)
        {
            auto& event = this->events.emplace_back();

            event.type               = TYPE_KEYBOARD;
            event.subtype            = SUBTYPE_TEXTINPUT;
            event.keyboardInput.text = text;
        }

        virtual void deInitialize()
        {}

      private:
        bool hysteresis;

      protected:
        virtual void pollInternal() = 0;

        void sendTouchEvent(SubEventType type, size_t id, float x, float y, float dx, float dy,
                            float pressure)
        {
            auto& event = this->events.emplace_back();

            event.type    = TYPE_TOUCH;
            event.subtype = type;
            event.finger  = { id, x, y, dx, dy, pressure };
        }

        void sendGamepadButtonEvent(SubEventType type, int which, int input)
        {
            auto& event = this->events.emplace_back();

            event.type          = TYPE_GAMEPAD;
            event.subtype       = type;
            event.gamepadButton = { which, input };

            // also send a joystick event
            auto& jevent = this->events.emplace_back();

            jevent.type = TYPE_GAMEPAD;
            auto jtype  = type == SUBTYPE_GAMEPADDOWN ? SUBTYPE_JOYSTICKDOWN : SUBTYPE_JOYSTICKUP;

            jevent.subtype       = jtype;
            jevent.gamepadButton = { which, input };
        }

        void sendGamepadAxisEvent(int which, int axis, float value)
        {
            auto& event = this->events.emplace_back();

            event.type        = TYPE_GAMEPAD;
            event.subtype     = SUBTYPE_GAMEPADAXIS;
            event.gamepadAxis = { which, axis, value };

            // also send a joystick event
            auto& jevent       = this->events.emplace_back();
            jevent.type        = TYPE_GAMEPAD;
            jevent.subtype     = SUBTYPE_JOYSTICKAXIS;
            jevent.gamepadAxis = { which, axis, value };
        }

        void sendGamepadSensorEvent(int which, int sensor, const std::vector<float>& data)
        {
            auto& event = this->events.emplace_back();

            event.type          = TYPE_GAMEPAD;
            event.subtype       = SUBTYPE_GAMEPADSENSORUPDATED;
            event.gamepadSensor = { which, sensor, data };
        }

        bool touchHeld;
        bool focused;

        bool initJoysticksAdded;
        std::list<LOVE_Event> events;
    };
} // namespace love
