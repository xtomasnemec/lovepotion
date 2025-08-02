#include "modules/audio/wrap_Audio.hpp"
#include "modules/audio/wrap_Source.hpp"

#include "modules/filesystem/wrap_Filesystem.hpp"

using namespace love;

#define instance() Module::getInstance<Audio>(Module::M_AUDIO)

int Wrap_Audio::newSource(lua_State* L)
{
    auto type = Source::TYPE_STREAM;

    if (!luax_istype(L, 1, SoundData::type))
    {
        if (!luax_istype(L, 1, Decoder::type))
        {
            const char* typeName = luaL_checkstring(L, 2);
            if (typeName && !Source::getConstant(typeName, type))
                return luax_enumerror(L, "source type", Source::SourceTypes, typeName);

            if (type == Source::TYPE_QUEUE)
                return luaL_error(L, E_CANNOT_CREATE_QUEUE_SOURCE);
        }

        if (luax_cangetdata(L, 1))
        {
            if (type == Source::TYPE_STATIC)
                lua_pushstring(L, "memory");
            else if (!lua_isnone(L, 3))
                lua_pushvalue(L, 3);
            else
                lua_pushnil(L);

            lua_pushnil(L);

            std::array<int, 3> indexes { 1, lua_gettop(L), lua_gettop(L) - 1 };
            luax_convobj(L, indexes, "sound", "newDecoder");
        }
    }

    if (type == Source::TYPE_STATIC && luax_istype(L, 1, Decoder::type))
        luax_convobj(L, 1, "sound", "newSoundData");

    Source* source = nullptr;

    luax_catchexcept(L, [&] {
        if (luax_istype(L, 1, SoundData::type))
            source = instance()->newSource(luax_totype<SoundData>(L, 1));
        else if (luax_istype(L, 1, Decoder::type))
            source = instance()->newSource(luax_totype<Decoder>(L, 1));
    });

    if (source != nullptr)
    {
        luax_pushtype(L, source);
        source->release();

        return 1;
    }
    else
        return luax_typeerror(L, 1, "Decoder or SoundData");
}

static std::vector<Source*> readSourceList(lua_State* L, int index)
{
    if (index < 0)
        index += lua_gettop(L) + 1;

    int count = luax_objlen(L, index);
    std::vector<Source*> sources(count);

    for (int i = 0; i < count; i++)
    {
        lua_rawgeti(L, index, i + 1);
        sources[i] = luax_checksource(L, -1);
        lua_pop(L, 1);
    }

    return sources;
}

static std::vector<Source*> readSourceVararg(lua_State* L, int index)
{
    const auto top = lua_gettop(L);

    if (index < 0)
        index += top + 1;

    int count = top - index + 1;
    std::vector<Source*> sources(count);

    for (int i = 0; i <= top; index++, i++)
        sources[i] = luax_checksource(L, -1);

    return sources;
}

int Wrap_Audio::play(lua_State* L)
{
    if (lua_istable(L, 1))
        luax_pushboolean(L, instance()->play(readSourceList(L, 1)));
    else if (lua_gettop(L) > 1)
        luax_pushboolean(L, instance()->play(readSourceVararg(L, 1)));
    else
    {
        auto* source = luax_checksource(L, 1);
        luax_pushboolean(L, source->play());
    }

    return 1;
}

int Wrap_Audio::stop(lua_State* L)
{
    if (lua_isnone(L, 1))
        instance()->stop();
    else if (lua_istable(L, 1))
        instance()->stop(readSourceList(L, 1));
    else if (lua_gettop(L) > 1)
        instance()->stop(readSourceVararg(L, 1));
    else
    {
        auto* source = luax_checksource(L, 1);
        source->stop();
    }

    return 0;
}

int Wrap_Audio::pause(lua_State* L)
{
    if (lua_isnone(L, 1))
    {
        auto sources = instance()->pause();
        lua_createtable(L, (int)sources.size(), 0);

        for (int index = 0; index < (int)sources.size(); index++)
        {
            luax_pushtype(L, sources[index]);
            lua_rawseti(L, -2, index + 1);
        }

        return 1;
    }
    else if (lua_istable(L, 1))
        instance()->pause(readSourceList(L, 1));
    else if (lua_gettop(L) > 1)
        instance()->pause(readSourceVararg(L, 1));
    else
    {
        auto* source = luax_checksource(L, 1);
        source->pause();
    }

    return 0;
}

int Wrap_Audio::setVolume(lua_State* L)
{
    float volume = luaL_checknumber(L, 1);
    instance()->setVolume(volume);

    return 0;
}

int Wrap_Audio::getVolume(lua_State* L)
{
    lua_pushnumber(L, instance()->getVolume());

    return 1;
}

int Wrap_Audio::setPosition(lua_State* L)
{
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_optnumber(L, 3, 0.0);
    
    // Audio listener position - stub implementation
    // Real implementation would set OpenAL listener position
    return 0;
}

int Wrap_Audio::getPosition(lua_State* L)
{
    // Return default listener position
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    return 3;
}

int Wrap_Audio::setOrientation(lua_State* L)
{
    float fx = (float)luaL_checknumber(L, 1);
    float fy = (float)luaL_checknumber(L, 2);
    float fz = (float)luaL_checknumber(L, 3);
    float ux = (float)luaL_checknumber(L, 4);
    float uy = (float)luaL_checknumber(L, 5);
    float uz = (float)luaL_checknumber(L, 6);
    
    // Audio listener orientation - stub implementation
    return 0;
}

int Wrap_Audio::getOrientation(lua_State* L)
{
    // Return default listener orientation (forward and up vectors)
    lua_pushnumber(L, 0.0);  // fx
    lua_pushnumber(L, 0.0);  // fy  
    lua_pushnumber(L, -1.0); // fz
    lua_pushnumber(L, 0.0);  // ux
    lua_pushnumber(L, 1.0);  // uy
    lua_pushnumber(L, 0.0);  // uz
    return 6;
}

int Wrap_Audio::setVelocity(lua_State* L)
{
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_optnumber(L, 3, 0.0);
    
    // Audio listener velocity - stub implementation
    return 0;
}

int Wrap_Audio::getVelocity(lua_State* L)
{
    // Return default listener velocity
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    return 3;
}

int Wrap_Audio::setDopplerScale(lua_State* L)
{
    float scale = (float)luaL_checknumber(L, 1);
    // Set doppler scale - stub implementation
    return 0;
}

int Wrap_Audio::getDopplerScale(lua_State* L)
{
    // Return default doppler scale
    lua_pushnumber(L, 1.0);
    return 1;
}

int Wrap_Audio::setDistanceModel(lua_State* L)
{
    const char* model = luaL_checkstring(L, 1);
    // Set distance model - stub implementation
    // Valid models: "none", "inverse", "inverseclamped", "linear", "linearclamped", "exponent", "exponentclamped"
    return 0;
}

int Wrap_Audio::getDistanceModel(lua_State* L)
{
    // Return default distance model
    lua_pushstring(L, "inverseclamped");
    return 1;
}

// clang-format off
static constexpr luaL_Reg functions[] =
{
    { "newSource",        Wrap_Audio::newSource        },
    { "play",             Wrap_Audio::play             },
    { "stop",             Wrap_Audio::stop             },
    { "pause",            Wrap_Audio::pause            },
    { "setVolume",        Wrap_Audio::setVolume        },
    { "getVolume",        Wrap_Audio::getVolume        },
    { "setPosition",      Wrap_Audio::setPosition      },
    { "getPosition",      Wrap_Audio::getPosition      },
    { "setOrientation",   Wrap_Audio::setOrientation   },
    { "getOrientation",   Wrap_Audio::getOrientation   },
    { "setVelocity",      Wrap_Audio::setVelocity      },
    { "getVelocity",      Wrap_Audio::getVelocity      },
    { "setDopplerScale",  Wrap_Audio::setDopplerScale  },
    { "getDopplerScale",  Wrap_Audio::getDopplerScale  },
    { "setDistanceModel", Wrap_Audio::setDistanceModel },
    { "getDistanceModel", Wrap_Audio::getDistanceModel }
};

static constexpr lua_CFunction types[] =
{
    love::open_source
};
// clang-format on

int Wrap_Audio::open(lua_State* L)
{
    auto* instance = instance();

    if (instance == nullptr)
        luax_catchexcept(L, [&] { instance = new Audio(); });
    else
        instance->retain();

    WrappedModule module {};
    module.instance  = instance;
    module.name      = "audio";
    module.type      = &Module::type;
    module.functions = functions;
    module.types     = types;

    return luax_register_module(L, module);
}
