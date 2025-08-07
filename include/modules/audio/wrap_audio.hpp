#pragma once

#include "common/luax.hpp"
#include "modules/audio/Audio.hpp"

namespace Wrap_Audio
{
    int newSource(lua_State* L);

    int play(lua_State* L);

    int pause(lua_State* L);

    int stop(lua_State* L);

    int setVolume(lua_State* L);

    int getVolume(lua_State* L);

    int setPosition(lua_State* L);
    
    int getPosition(lua_State* L);
    
    int setOrientation(lua_State* L);
    
    int getOrientation(lua_State* L);
    
    int setVelocity(lua_State* L);
    
    int getVelocity(lua_State* L);
    
    int setDopplerScale(lua_State* L);
    
    int getDopplerScale(lua_State* L);
    
    int setDistanceModel(lua_State* L);
    
    int getDistanceModel(lua_State* L);

    int open(lua_State* L);
} // namespace Wrap_Audio
