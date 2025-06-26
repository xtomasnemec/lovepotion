#pragma once

#include "common/luax.hpp"
#include "modules/video/Video.hpp"

namespace love
{
    Video* luax_checkvideo(lua_State* L, int index);

    int open_video(lua_State* L);
}

namespace Wrap_Video
{
    int play(lua_State* L);

    int pause(lua_State* L);

    int rewind(lua_State* L);

    int isPlaying(lua_State* L);

    int getDuration(lua_State* L);

    int getSource(lua_State* L);
} // namespace Wrap_Video
