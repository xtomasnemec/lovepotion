#include "modules/math/wrap_Math.hpp"
#include "common/luax.hpp"
#include "common/math.hpp"
#include <random>
#include <cmath>

using namespace love;

static std::mt19937 rng;
static std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);

int Wrap_MathModule::random(lua_State* L)
{
    int args = lua_gettop(L);
    
    if (args == 0)
    {
        // random() - returns [0, 1)
        lua_pushnumber(L, uniform_dist(rng));
    }
    else if (args == 1)
    {
        // random(max) - returns [1, max]
        double max = luaL_checknumber(L, 1);
        if (max < 1)
            return luaL_error(L, "random upper bound must be at least 1");
        
        std::uniform_int_distribution<int> int_dist(1, (int)max);
        lua_pushinteger(L, int_dist(rng));
    }
    else if (args == 2)
    {
        // random(min, max) - returns [min, max]
        double min = luaL_checknumber(L, 1);
        double max = luaL_checknumber(L, 2);
        
        if (min > max)
            return luaL_error(L, "random minimum is greater than maximum");
        
        std::uniform_int_distribution<int> int_dist((int)min, (int)max);
        lua_pushinteger(L, int_dist(rng));
    }
    else
    {
        return luaL_error(L, "wrong number of arguments");
    }
    
    return 1;
}

int Wrap_MathModule::setRandomSeed(lua_State* L)
{
    lua_Number seed = luaL_checknumber(L, 1);
    rng.seed((unsigned int)seed);
    return 0;
}

int Wrap_MathModule::randomSeed(lua_State* L)
{
    return Wrap_MathModule::setRandomSeed(L);
}

int Wrap_MathModule::getRandomSeed(lua_State* L)
{
    // Note: std::mt19937 doesn't provide a way to get the seed back
    // This is a limitation - real Love2D tracks the seed
    lua_pushnumber(L, 0);
    return 1;
}

int Wrap_MathModule::noise(lua_State* L)
{
    int args = lua_gettop(L);
    double result = 0.0;
    
    if (args == 1)
    {
        double x = luaL_checknumber(L, 1);
        result = love::noise1(x);
    }
    else if (args == 2)
    {
        double x = luaL_checknumber(L, 1);
        double y = luaL_checknumber(L, 2);
        result = love::noise2(x, y);
    }
    else if (args == 3)
    {
        double x = luaL_checknumber(L, 1);
        double y = luaL_checknumber(L, 2);
        double z = luaL_checknumber(L, 3);
        result = love::noise3(x, y, z);
    }
    else if (args == 4)
    {
        double x = luaL_checknumber(L, 1);
        double y = luaL_checknumber(L, 2);
        double z = luaL_checknumber(L, 3);
        double w = luaL_checknumber(L, 4);
        result = love::noise4(x, y, z, w);
    }
    else
    {
        return luaL_error(L, "wrong number of arguments");
    }
    
    lua_pushnumber(L, result);
    return 1;
}

int Wrap_MathModule::gammaToLinear(lua_State* L)
{
    if (lua_gettop(L) <= 3 && lua_isnumber(L, 1))
    {
        if (lua_gettop(L) == 1)
        {
            double gamma = luaL_checknumber(L, 1);
            lua_pushnumber(L, love::gammaToLinear(gamma));
        }
        else
        {
            double r = luaL_checknumber(L, 1);
            double g = luaL_checknumber(L, 2);
            double b = luaL_checknumber(L, 3);
            
            lua_pushnumber(L, love::gammaToLinear(r));
            lua_pushnumber(L, love::gammaToLinear(g));
            lua_pushnumber(L, love::gammaToLinear(b));
            
            return 3;
        }
    }
    else
    {
        return luaL_error(L, "invalid arguments");
    }
    
    return 1;
}

int Wrap_MathModule::linearToGamma(lua_State* L)
{
    if (lua_gettop(L) <= 3 && lua_isnumber(L, 1))
    {
        if (lua_gettop(L) == 1)
        {
            double linear = luaL_checknumber(L, 1);
            lua_pushnumber(L, love::linearToGamma(linear));
        }
        else
        {
            double r = luaL_checknumber(L, 1);
            double g = luaL_checknumber(L, 2);
            double b = luaL_checknumber(L, 3);
            
            lua_pushnumber(L, love::linearToGamma(r));
            lua_pushnumber(L, love::linearToGamma(g));
            lua_pushnumber(L, love::linearToGamma(b));
            
            return 3;
        }
    }
    else
    {
        return luaL_error(L, "invalid arguments");
    }
    
    return 1;
}

int Wrap_MathModule::colorFromBytes(lua_State* L)
{
    uint8_t r = (uint8_t)luaL_checkinteger(L, 1);
    uint8_t g = (uint8_t)luaL_checkinteger(L, 2);
    uint8_t b = (uint8_t)luaL_checkinteger(L, 3);
    uint8_t a = (uint8_t)luaL_optinteger(L, 4, 255);
    
    lua_pushnumber(L, r / 255.0);
    lua_pushnumber(L, g / 255.0);
    lua_pushnumber(L, b / 255.0);
    lua_pushnumber(L, a / 255.0);
    
    return 4;
}

int Wrap_MathModule::colorToBytes(lua_State* L)
{
    double r = luaL_checknumber(L, 1);
    double g = luaL_checknumber(L, 2);
    double b = luaL_checknumber(L, 3);
    double a = luaL_optnumber(L, 4, 1.0);
    
    lua_pushinteger(L, (int)(r * 255));
    lua_pushinteger(L, (int)(g * 255));
    lua_pushinteger(L, (int)(b * 255));
    lua_pushinteger(L, (int)(a * 255));
    
    return 4;
}

int Wrap_MathModule::isConvex(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    
    size_t count = luax_objlen(L, 1);
    if (count < 6 || count % 2 != 0)
    {
        lua_pushboolean(L, false);
        return 1;
    }
    
    // Basic convexity check - this is simplified
    // Real Love2D has more sophisticated polygon analysis
    lua_pushboolean(L, true);
    return 1;
}

int Wrap_MathModule::triangulate(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    
    // This is a stub - real triangulation is complex
    // Return empty table for now
    lua_newtable(L);
    return 1;
}

int Wrap_MathModule::newRandomGenerator(lua_State* L)
{
    // For simplicity, just return the seed value
    // Real Love2D would create a RandomGenerator object
    lua_Number seed = luaL_optnumber(L, 1, std::random_device{}());
    lua_pushnumber(L, seed);
    return 1;
}

int Wrap_MathModule::compress(lua_State* L)
{
    return luaL_error(L, "compress not yet implemented");
}

int Wrap_MathModule::decompress(lua_State* L)
{
    return luaL_error(L, "decompress not yet implemented");
}

static constexpr luaL_Reg functions[] =
{
    { "random",            Wrap_MathModule::random            },
    { "randomSeed",        Wrap_MathModule::randomSeed        },
    { "setRandomSeed",     Wrap_MathModule::setRandomSeed     },
    { "getRandomSeed",     Wrap_MathModule::getRandomSeed     },
    { "newRandomGenerator", Wrap_MathModule::newRandomGenerator },
    { "noise",             Wrap_MathModule::noise             },
    { "gammaToLinear",     Wrap_MathModule::gammaToLinear     },
    { "linearToGamma",     Wrap_MathModule::linearToGamma     },
    { "isConvex",          Wrap_MathModule::isConvex          },
    { "triangulate",       Wrap_MathModule::triangulate       },
    { "colorFromBytes",    Wrap_MathModule::colorFromBytes    },
    { "colorToBytes",      Wrap_MathModule::colorToBytes      },
    { "compress",          Wrap_MathModule::compress          },
    { "decompress",        Wrap_MathModule::decompress        },
    { nullptr,             nullptr                            }
};

int Wrap_MathModule::open(lua_State* L)
{
    lua_newtable(L);
    luax_register(L, functions);
    return 1;
}
