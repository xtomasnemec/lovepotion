#pragma once

#include "common/Module.hpp"

namespace love
{
    class MouseBase : public Module
    {
      public:
        MouseBase() : Module(M_MOUSE, "love.mouse")
        {}

        virtual ~MouseBase()
        {}

        virtual void getPosition(double& x, double& y) const = 0;

        virtual void setPosition(double x, double y) = 0;

        virtual bool isVisible() const = 0;

        virtual void setVisible(bool visible) = 0;

        virtual bool isGrabbed() const = 0;

        virtual void setGrabbed(bool grabbed) = 0;

        virtual bool getRelativeMode() const = 0;

        virtual void setRelativeMode(bool relative) = 0;
    };
} // namespace love
