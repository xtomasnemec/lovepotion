#pragma once

#include "common/luax.hpp"
#include "modules/mouse/Mouse.hpp"

namespace Wrap_Mouse
{
    int getPosition(lua_State* L);

    int setPosition(lua_State* L);

    int isVisible(lua_State* L);

    int setVisible(lua_State* L);

    int isGrabbed(lua_State* L);

    int setGrabbed(lua_State* L);

    int getRelativeMode(lua_State* L);

    int setRelativeMode(lua_State* L);

    int open(lua_State* L);
} // namespace Wrap_Mouse
