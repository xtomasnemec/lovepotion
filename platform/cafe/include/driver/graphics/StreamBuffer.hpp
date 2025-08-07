#pragma once

#include "driver/graphics/StreamBuffer.tcc"
#include "modules/graphics/Volatile.hpp"

#include <gx2/mem.h>
#include <gx2r/buffer.h>
#include <gx2r/draw.h>

#ifdef __WIIU__
#include <coreinit/memheap.h>
#include <coreinit/memory.h>
#include <cstdio>
#include "debug_log.h"
#endif

namespace love
{
    static GX2RResourceFlags getBufferUsage(BufferUsage usage)
    {
        if (usage == BUFFERUSAGE_VERTEX)
            return GX2R_RESOURCE_BIND_VERTEX_BUFFER;

        return GX2R_RESOURCE_BIND_INDEX_BUFFER;
    }

    template<typename T>
    class StreamBuffer final : public StreamBufferBase<T>
    {
      public:
        StreamBuffer(BufferUsage mode, size_t size) : StreamBufferBase<T>(mode, size), buffer {}
        {
            const auto flags = getBufferUsage(mode);

#ifdef __WIIU__
            // Wii U memory limit: Cap buffer size to prevent memory allocation failures
            const size_t WII_U_MAX_BUFFER_SIZE = 256 * 1024 * 1024; // 256MB limit
            const size_t requestedSize = size * sizeof(T);
            
            char debugMsg[256];
            sprintf(debugMsg, "StreamBuffer constructor: mode=%d, requested elemCount=%zu, elemSize=%zu, totalSize=%zu bytes", 
                    (int)mode, size, sizeof(T), requestedSize);
            wiiu_debug_log_exception(debugMsg);
            
            if (requestedSize > WII_U_MAX_BUFFER_SIZE)
            {
                sprintf(debugMsg, "StreamBuffer size capped: requested %zu bytes, using %zu bytes (256MB limit)", 
                        requestedSize, WII_U_MAX_BUFFER_SIZE);
                wiiu_debug_log_exception(debugMsg);
                
                size = WII_U_MAX_BUFFER_SIZE / sizeof(T);
            }
#endif

            this->buffer.elemCount = size;
            this->buffer.elemSize  = sizeof(T);
            this->buffer.flags     = flags | BUFFER_CREATE_FLAGS;

#ifdef __WIIU__
            // Debug logging for buffer creation
            sprintf(debugMsg, "Creating StreamBuffer: elemCount=%zu, elemSize=%zu, totalSize=%zu bytes, flags=0x%x", 
                    size, sizeof(T), size * sizeof(T), this->buffer.flags);
            wiiu_debug_log_exception(debugMsg);
#endif

            if (!GX2RCreateBuffer(&this->buffer))
            {
#ifdef __WIIU__
                sprintf(debugMsg, "GX2RCreateBuffer failed for buffer size %zu bytes", size * sizeof(T));
                wiiu_debug_log_exception(debugMsg);
#endif
                throw love::Exception("Failed to create StreamBuffer");
            }

#ifdef __WIIU__
            sprintf(debugMsg, "StreamBuffer created successfully: %zu bytes", size * sizeof(T));
            wiiu_debug_log_exception(debugMsg);
#endif
        }

        StreamBuffer(StreamBuffer&&) = delete;

        StreamBuffer& operator=(const StreamBuffer&) = delete;

        ~StreamBuffer()
        {
            if (!GX2RBufferExists(&this->buffer))
                return;

            GX2RDestroyBufferEx(&this->buffer, GX2R_RESOURCE_BIND_NONE);
        }

        MapInfo<T> map(size_t)
        {
            MapInfo<T> info {};

            auto* data = (T*)GX2RLockBufferEx(&this->buffer, GX2R_RESOURCE_BIND_NONE);

            info.data = &data[this->index];
            info.size = this->bufferSize - this->frameGPUReadOffset;

            return info;
        }

        size_t unmap(size_t)
        {
            GX2RUnlockBufferEx(&this->buffer, GX2R_RESOURCE_BIND_NONE);

            if (this->mode == BufferUsage::BUFFERUSAGE_VERTEX)
                GX2RSetAttributeBuffer(&this->buffer, 0, this->buffer.elemSize, 0);

            return this->index;
        }

        ptrdiff_t getHandle() const override
        {
            return (ptrdiff_t)std::addressof(this->buffer);
        }

      private:
        static constexpr auto BUFFER_CREATE_FLAGS =
            GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ;

        GX2RBuffer buffer;
    };
} // namespace love
