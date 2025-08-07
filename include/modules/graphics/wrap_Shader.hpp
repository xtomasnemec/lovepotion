#pragma once

#include <common/luax.hpp>

namespace love
{
    class ShaderBase;

    namespace Wrap_Shader
    {
        ShaderBase* CheckShader(lua_State* L, int index);
    } // namespace Wrap_Shader
} // namespace love
