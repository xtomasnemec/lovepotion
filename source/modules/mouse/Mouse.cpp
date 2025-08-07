#include "modules/mouse/Mouse.hpp"

namespace love
{
    Mouse::Mouse() : MouseBase()
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
} // namespace love
