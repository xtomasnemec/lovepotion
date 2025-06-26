#pragma once

#include "modules/video/Video.tcc"

#include <gx2/display.h>
#include <gx2/mem.h>

namespace love
{
    class Video : public VideoBase
    {
      public:
        Video();

        virtual ~Video();

        void draw(GraphicsBase* graphics, const Matrix4& transform) override;

        void play() override;

        void pause() override;

        void rewind() override;

        bool isPlaying() const override;

        double getDuration() const override;

        void setSource(const std::string& source) override;

        const std::string& getSource() const override;

      private:
        std::string source;
        bool playing = false;
        double duration = 0.0;
        double currentTime = 0.0;
        
        // Wii U specific video data
        void* videoTexture = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
    };
} // namespace love
