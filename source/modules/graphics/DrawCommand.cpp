#if defined(__SWITCH__)
    #include "driver/display/deko.hpp"
#endif

#include "driver/graphics/DrawCommand.hpp"

#ifdef __WIIU__
#include "debug_log.h"
#endif

namespace love
{
    StreamBuffer<Vertex>* newVertexBuffer(size_t size)
    {
#ifdef __WIIU__
        char debugMsg[256];
        sprintf(debugMsg, "Creating new vertex buffer with size %zu (%zu bytes total)", size, size * sizeof(Vertex));
        wiiu_debug_log_exception(debugMsg);
#endif
        auto* buffer = new StreamBuffer<Vertex>(BUFFERUSAGE_VERTEX, size);
#if defined(__SWITCH__)
        buffer->allocate(d3d.getMemoryPool(deko3d::MEMORYPOOL_DATA));
#endif
        return buffer;
    }

    StreamBuffer<uint16_t>* newIndexBuffer(size_t size)
    {
#ifdef __WIIU__
        char debugMsg[256];
        sprintf(debugMsg, "Creating new index buffer with size %zu (%zu bytes total)", size, size * sizeof(uint16_t));
        wiiu_debug_log_exception(debugMsg);
#endif
        auto* buffer = new StreamBuffer<uint16_t>(BUFFERUSAGE_INDEX, size);
#if defined(__SWITCH__)
        buffer->allocate(d3d.getMemoryPool(deko3d::MEMORYPOOL_DATA));
#endif
        return buffer;
    }
} // namespace love
