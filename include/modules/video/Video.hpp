#pragma once

#include "modules/graphics/Drawable.hpp"
#include "common/Matrix.hpp"

namespace love
{
    class Video : public Drawable
    {
      public:
        static inline Type type = Type("Video", &Drawable::type);

        Video();

        virtual ~Video();

        void draw(GraphicsBase* graphics, const Matrix4& transform) override;

        void play();

        void pause();

        void rewind();

        bool isPlaying() const;

        double getDuration() const;

        void setSource(const std::string& source);

        const std::string& getSource() const;

      private:
        std::string source;
        bool playing = false;
        double duration = 0.0;
    };
} // namespace love
