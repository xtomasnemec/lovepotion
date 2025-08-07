#pragma once

#include "common/luax.hpp"

namespace Wrap_MathModule
{
    int random(lua_State* L);
    int randomSeed(lua_State* L);
    int setRandomSeed(lua_State* L);
    int getRandomSeed(lua_State* L);
    int newRandomGenerator(lua_State* L);
    int noise(lua_State* L);
    int gammaToLinear(lua_State* L);
    int linearToGamma(lua_State* L);
    int isConvex(lua_State* L);
    int triangulate(lua_State* L);
    int colorFromBytes(lua_State* L);
    int colorToBytes(lua_State* L);
    int compress(lua_State* L);
    int decompress(lua_State* L);
    int open(lua_State* L);
}
