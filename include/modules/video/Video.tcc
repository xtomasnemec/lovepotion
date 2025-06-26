#pragma once

#include "modules/graphics/Drawable.hpp"
#include "common/Matrix.hpp"

namespace love
{
    class VideoBase : public Drawable
    {
      public:
        static inline Type type = Type("Video", &Drawable::type);

        VideoBase() {}

        virtual ~VideoBase() {}

        virtual void draw(GraphicsBase* graphics, const Matrix4& transform) = 0;

        virtual void play() = 0;

        virtual void pause() = 0;

        virtual void rewind() = 0;

        virtual bool isPlaying() const = 0;

        virtual double getDuration() const = 0;

        virtual void setSource(const std::string& source) = 0;

        virtual const std::string& getSource() const = 0;
    };
} // namespace love
