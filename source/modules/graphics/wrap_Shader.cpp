#include <modules/graphics/wrap_Shader.hpp>

using namespace love;

int Wrap_Shader::send(lua_State* L)
{
    // Minimal stub implementation - just return 0 to indicate success
    return 0;
}

int Wrap_Shader::hasUniform(lua_State* L)
{
    // Always return false to prevent crashes
    lua_pushboolean(L, false);
    return 1;
}

// Empty function array to prevent any shader function registration
const luaL_Reg Wrap_Shader::functions[] = {
    { nullptr, nullptr }
};

int love::open_shader(lua_State* L)
{
    // Create a minimal shader table without registering any functions
    lua_newtable(L);
    return 1;
}
