#include "modules/video/Video.hpp"
#include "modules/graphics/Graphics.hpp"

namespace love
{
    Video::Video() : VideoBase()
    {
        this->width = 1280;  // Default Wii U TV resolution
        this->height = 720;
    }

    Video::~Video()
    {
        if (videoTexture)
        {
            // Clean up GX2 video texture
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, videoTexture, width * height * 4);
        }
    }

    void Video::draw(GraphicsBase* graphics, const Matrix4& transform)
    {
        if (!playing || !videoTexture)
            return;

        // On Wii U, this would render the video texture using GX2
        // For now, this is a placeholder implementation
    }

    void Video::play()
    {
        this->playing = true;
    }

    void Video::pause()
    {
        this->playing = false;
    }

    void Video::rewind()
    {
        this->currentTime = 0.0;
    }

    bool Video::isPlaying() const
    {
        return this->playing;
    }

    double Video::getDuration() const
    {
        return this->duration;
    }

    void Video::setSource(const std::string& source)
    {
        this->source = source;
        // In a real implementation, load video file and get duration
        this->duration = 10.0; // placeholder duration
    }

    const std::string& Video::getSource() const
    {
        return this->source;
    }
} // namespace love
