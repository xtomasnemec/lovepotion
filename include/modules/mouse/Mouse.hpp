#pragma once

#include "common/Module.hpp"

namespace love
{
    class Mouse : public Module
    {
      public:
        Mouse();

        virtual ~Mouse()
        {}

        void getPosition(double& x, double& y) const;

        void setPosition(double x, double y);

        bool isVisible() const;

        void setVisible(bool visible);

        bool isGrabbed() const;

        void setGrabbed(bool grabbed);

        bool getRelativeMode() const;

        void setRelativeMode(bool relative);

      private:
        double x = 0.0;
        double y = 0.0;
        bool visible = true;
        bool grabbed = false;
        bool relativeMode = false;
    };
} // namespace love
