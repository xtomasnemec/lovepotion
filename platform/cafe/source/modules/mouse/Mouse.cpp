#include "modules/mouse/Mouse.hpp"

#include <vpad/input.h>

namespace love
{
    Mouse::Mouse() : MouseBase()
    {
        // Initialize touch data
        touchData.x = 0;
        touchData.y = 0;
        touchData.touched = 0;
    }

    Mouse::~Mouse()
    {}

    void Mouse::getPosition(double& x, double& y) const
    {
        x = this->x;
        y = this->y;
    }

    void Mouse::setPosition(double x, double y)
    {
        this->x = x;
        this->y = y;
    }

    bool Mouse::isVisible() const
    {
        return this->visible;
    }

    void Mouse::setVisible(bool visible)
    {
        this->visible = visible;
    }

    bool Mouse::isGrabbed() const
    {
        return this->grabbed;
    }

    void Mouse::setGrabbed(bool grabbed)
    {
        this->grabbed = grabbed;
    }

    bool Mouse::getRelativeMode() const
    {
        return this->relativeMode;
    }

    void Mouse::setRelativeMode(bool relative)
    {
        this->relativeMode = relative;
    }

    void Mouse::updateTouchPosition()
    {
        // On Wii U, we can use the GamePad touch screen as mouse input
        VPADStatus vpadStatus;
        VPADReadError error;
        VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &error);
        
        if (error == VPAD_READ_SUCCESS && vpadStatus.tpNormal.touched)
        {
            // Convert touch coordinates to screen coordinates
            // GamePad touch screen is 854x480
            this->x = (double)vpadStatus.tpNormal.x;
            this->y = (double)vpadStatus.tpNormal.y;
        }
    }
} // namespace love
