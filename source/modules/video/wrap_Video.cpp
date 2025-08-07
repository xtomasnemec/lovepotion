#include "modules/video/wrap_Video.hpp"

using namespace love;

Video* love::luax_checkvideo(lua_State* L, int index)
{
    return luax_checktype<Video>(L, index);
}

int Wrap_Video::play(lua_State* L)
{
    Video* video = luax_checkvideo(L, 1);
    video->play();

    return 0;
}

int Wrap_Video::pause(lua_State* L)
{
    Video* video = luax_checkvideo(L, 1);
    video->pause();

    return 0;
}

int Wrap_Video::rewind(lua_State* L)
{
    Video* video = luax_checkvideo(L, 1);
    video->rewind();

    return 0;
}

int Wrap_Video::isPlaying(lua_State* L)
{
    Video* video = luax_checkvideo(L, 1);
    lua_pushboolean(L, video->isPlaying());

    return 1;
}

int Wrap_Video::getDuration(lua_State* L)
{
    Video* video = luax_checkvideo(L, 1);
    lua_pushnumber(L, video->getDuration());

    return 1;
}

int Wrap_Video::getSource(lua_State* L)
{
    Video* video = luax_checkvideo(L, 1);
    luax_pushstring(L, video->getSource());

    return 1;
}

static constexpr luaL_Reg functions[] =
{
    { "play",        Wrap_Video::play        },
    { "pause",       Wrap_Video::pause       },
    { "rewind",      Wrap_Video::rewind      },
    { "isPlaying",   Wrap_Video::isPlaying   },
    { "getDuration", Wrap_Video::getDuration },
    { "getSource",   Wrap_Video::getSource   }
};

int love::open_video(lua_State* L)
{
    return luax_register_type(L, &Video::type, functions);
}
