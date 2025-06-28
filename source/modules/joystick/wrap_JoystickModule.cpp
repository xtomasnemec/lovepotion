#include "modules/joystick/wrap_JoystickModule.hpp"
#include "modules/joystick/wrap_Joystick.hpp"

using namespace love;

#define MODULE_INSTANCE() Module::getInstance<JoystickModule>(Module::M_JOYSTICK)

int Wrap_JoystickModule::getJoysticks(lua_State* L)
{
    int count = MODULE_INSTANCE()->getJoystickCount();
    lua_createtable(L, count, 0);

    for (int index = 0; index < count; index++)
    {
        auto* joystick = MODULE_INSTANCE()->getJoystick(index);
        luax_pushtype(L, joystick);
        lua_rawseti(L, -2, index + 1);
    }

    return 1;
}

int Wrap_JoystickModule::getIndex(lua_State* L)
{
    auto* joystick = luax_checkjoystick(L, 1);
    int index      = MODULE_INSTANCE()->getIndex(joystick);

    if (index >= 0)
        lua_pushinteger(L, index);
    else
        lua_pushnil(L);

    return 1;
}

int Wrap_JoystickModule::getJoystickCount(lua_State* L)
{
    lua_pushinteger(L, MODULE_INSTANCE()->getJoystickCount());

    return 1;
}

// clang-format off
static constexpr luaL_Reg functions[] =
{
    { "getJoysticks",     Wrap_JoystickModule::getJoysticks     },
    { "getIndex",         Wrap_JoystickModule::getIndex         },
    { "getJoystickCount", Wrap_JoystickModule::getJoystickCount }
};

static constexpr lua_CFunction types[] =
{
    love::open_joystick
};
// clang-format on

int Wrap_JoystickModule::open(lua_State* L)
{
    auto* module_instance = Module::getInstance<JoystickModule>(Module::M_JOYSTICK);

    if (module_instance == nullptr)
        luax_catchexcept(L, [&] { module_instance = new JoystickModule(); });
    else
        module_instance->retain();

    WrappedModule module {};
    module.instance  = module_instance;
    module.name      = "joystick";
    module.type      = &Module::type;
    module.functions = functions;
    module.types     = types;

    return luax_register_module(L, module);
}
