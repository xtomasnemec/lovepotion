#include "driver/display/GX2.hpp"

#include "modules/graphics/Texture.hpp"
#include "driver/display/utility.hpp"

#include <gx2/state.h>
#include <gx2/utils.h>

#include <malloc.h>

namespace love
{
    static void createTextureObject(GX2Texture*& texture, PixelFormat format, int width, int height)
    {
        texture = new GX2Texture();

        if (!texture)
            throw love::Exception("Failed to create GX2Texture.");

        std::memset(&texture->surface, 0, sizeof(GX2Surface));

        texture->surface.use    = GX2_SURFACE_USE_TEXTURE;
        texture->surface.dim    = GX2_SURFACE_DIM_TEXTURE_2D;
        texture->surface.width  = width;
        texture->surface.height = height;

        texture->surface.depth     = 1;
        texture->surface.mipLevels = 1;

        GX2SurfaceFormat gpuFormat;
        if (!GX2::getConstant(format, gpuFormat))
            throw love::Exception("Invalid pixel format {:s}.", love::getConstant(format));

        texture->surface.format   = gpuFormat;
        texture->surface.aa       = GX2_AA_MODE1X;
        texture->surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
        texture->viewFirstMip     = 0;
        texture->viewNumMips      = 0;
        texture->viewFirstSlice   = 0;
        texture->viewNumSlices    = 1;
        texture->compMap          = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);

        GX2CalcSurfaceSizeAndAlignment(&texture->surface);
        GX2InitTextureRegs(texture);

        texture->surface.image = memalign(texture->surface.alignment, texture->surface.imageSize);

        if (!texture->surface.image)
            throw love::Exception("Failed to allocate texture memory.");

        std::memset(texture->surface.image, 0, texture->surface.imageSize);
        GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, texture->surface.image, texture->surface.imageSize);
    }

    Texture::Texture(GraphicsBase* graphics, const Settings& settings, const Slices* data) :
        TextureBase(graphics, settings, data),
        slices(settings.type),
        sampler {}
    {
        if (data != nullptr)
            slices = *data;

        if (!this->loadVolatile())
            throw love::Exception("Failed to create texture.");

        slices.clear();
    }

    Texture::~Texture()
    {
        this->unloadVolatile();
    }

    bool Texture::loadVolatile()
    {
        if (this->texture != nullptr || this->target != nullptr)
            return true;

        if (this->parentView.texture != this)
        {
            Texture* baseTexture = (Texture*)this->parentView.texture;
            baseTexture->loadVolatile();
        }

        if (this->isReadable())
            this->createTexture();

        int64_t memorySize = 0;

        for (int mip = 0; mip < this->getMipmapCount(); mip++)
        {
            int width  = this->getPixelWidth(mip);
            int height = this->getPixelHeight(mip);

            const auto faces = (this->textureType == TEXTURE_CUBE) ? 6 : 1;
            int slices       = this->getDepth(mip) * this->layers * faces;

            memorySize += getPixelFormatSliceSize(this->format, width, height, false) * slices;
        }

        this->setGraphicsMemorySize(memorySize);

        return true;
    }

    void Texture::unloadVolatile()
    {
        if (this->texture != nullptr)
        {
            if (this->texture->surface.image)
                free(this->texture->surface.image);
            delete this->texture;
            this->texture = nullptr;
        }

        if (this->target != nullptr)
        {
            if (this->target->surface.image)
                free(this->target->surface.image);
            delete this->target;
            this->target = nullptr;
        }

        this->setGraphicsMemorySize(0);
    }

    void Texture::createTexture()
    {
        if (!this->isRenderTarget())
        {
            try
            {
                createTextureObject(this->texture, this->format, this->pixelWidth, this->pixelHeight);
            }
            catch (love::Exception&)
            {
                throw;
            }

            int mipCount   = this->getMipmapCount();
            int sliceCount = 1;

            if (this->textureType == TEXTURE_VOLUME)
                sliceCount = this->getDepth();
            else if (this->textureType == TEXTURE_2D_ARRAY)
                sliceCount = this->layers;
            else if (this->textureType == TEXTURE_CUBE)
                sliceCount = 6;

            for (int mipmap = 0; mipmap < mipCount; mipmap++)
            {
                for (int slice = 0; slice < sliceCount; slice++)
                {
                    auto* data = this->slices.get(slice, mipmap);

                    if (data != nullptr)
                        this->uploadImageData(data, mipmap, slice, 0, 0);
                }
            }
        }

        bool hasData  = this->slices.get(0, 0) != nullptr;
        int clearMips = 1;

        if (isPixelFormatDepthStencil(this->format))
            clearMips = mipmapCount;

        if (this->isRenderTarget())
        {
            // Create GX2ColorBuffer for render target
            this->target = new GX2ColorBuffer();
            
            if (!this->target)
                throw love::Exception("Failed to create GX2ColorBuffer for render target.");
                
            std::memset(this->target, 0, sizeof(GX2ColorBuffer));
            
            this->target->surface.use    = GX2_SURFACE_USE_TEXTURE_COLOR_BUFFER_TV;
            this->target->surface.dim    = GX2_SURFACE_DIM_TEXTURE_2D;
            this->target->surface.aa     = GX2_AA_MODE1X;
            this->target->surface.width  = this->pixelWidth;
            this->target->surface.height = this->pixelHeight;
            
            this->target->surface.depth     = 1;
            this->target->surface.mipLevels = 1;
            
            // Convert pixel format to GX2 format
            GX2SurfaceFormat gpuFormat;
            if (!GX2::getConstant(this->format, gpuFormat))
                throw love::Exception("Invalid pixel format {:s} for render target.", love::getConstant(this->format));
                
            this->target->surface.format   = gpuFormat;
            this->target->surface.tileMode = GX2_TILE_MODE_DEFAULT;
            this->target->viewNumSlices    = 1;
            
            GX2CalcSurfaceSizeAndAlignment(&this->target->surface);
            GX2InitColorBufferRegs(this->target);
            
            // Allocate memory for the render target
            this->target->surface.image = memalign(this->target->surface.alignment, this->target->surface.imageSize);
            
            if (!this->target->surface.image)
                throw love::Exception("Failed to allocate render target memory.");
                
            std::memset(this->target->surface.image, 0, this->target->surface.imageSize);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, this->target->surface.image, this->target->surface.imageSize);
        }
        else if (!hasData)
        {
            for (int mipmap = 0; mipmap < clearMips; mipmap++)
            {
            }
        }

        this->setSamplerState(this->samplerState);

        if (this->slices.getMipmapCount() <= 1 && this->getMipmapsMode() != MIPMAPS_NONE)
            this->generateMipmaps();
    }

    void Texture::setSamplerState(const SamplerState& state)
    {
        this->samplerState = this->validateSamplerState(state);
        gx2.setSamplerState(this, this->samplerState);
    }

    void Texture::uploadByteData(const void* data, size_t size, int level, int slice, const Rect& rect)
    {
        const auto pitch = this->texture->surface.pitch;

        uint8_t* destination = (uint8_t*)this->texture->surface.image;
        uint8_t* source      = (uint8_t*)data;

        size_t pixelSize = getPixelFormatBlockSize(this->format);

        for (uint32_t y = 0; y < (uint32_t)rect.h; y++)
        {
            const auto srcRow  = (y * rect.w * pixelSize);
            const auto destRow = (rect.x + (y + rect.y) * pitch) * pixelSize;

            std::memcpy(destination + destRow, source + srcRow, rect.w * pixelSize);
        }

        const auto imageSize = this->texture->surface.imageSize;

        GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, this->texture->surface.image, imageSize);
        GX2Flush();
    }

    void Texture::generateMipmapsInternal()
    {}

    ptrdiff_t Texture::getHandle() const
    {
        return (ptrdiff_t)this->texture;
    }

    ptrdiff_t Texture::getRenderTargetHandle() const
    {
        return (ptrdiff_t)this->target;
    }

    ptrdiff_t Texture::getSamplerHandle() const
    {
        return (ptrdiff_t)std::addressof(this->sampler);
    }
} // namespace love
