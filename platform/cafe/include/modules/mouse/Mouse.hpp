#pragma once

#include "modules/mouse/Mouse.tcc"

#include <vpad/input.h>
#include <padscore/kpad.h>

namespace love
{
    class Mouse : public MouseBase
    {
      public:
        Mouse();

        virtual ~Mouse();

        void getPosition(double& x, double& y) const override;

        void setPosition(double x, double y) override;

        bool isVisible() const override;

        void setVisible(bool visible) override;

        bool isGrabbed() const override;

        void setGrabbed(bool grabbed) override;

        bool getRelativeMode() const override;

        void setRelativeMode(bool relative) override;

        void updateTouchPosition();

      private:
        double x = 0.0;
        double y = 0.0;
        bool visible = true;
        bool grabbed = false;
        bool relativeMode = false;
        
        // Wii U GamePad touch screen coordinates
        VPADTouchData touchData;
    };
} // namespace love
