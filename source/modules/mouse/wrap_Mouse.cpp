#include "modules/mouse/wrap_Mouse.hpp"

using namespace love;

#define instance() Module::getInstance<Mouse>(Module::M_MOUSE)

int Wrap_Mouse::getPosition(lua_State* L)
{
    double x, y;
    instance()->getPosition(x, y);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);

    return 2;
}

int Wrap_Mouse::setPosition(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);

    instance()->setPosition(x, y);

    return 0;
}

int Wrap_Mouse::isVisible(lua_State* L)
{
    lua_pushboolean(L, instance()->isVisible());

    return 1;
}

int Wrap_Mouse::setVisible(lua_State* L)
{
    bool visible = luax_toboolean(L, 1);
    instance()->setVisible(visible);

    return 0;
}

int Wrap_Mouse::isGrabbed(lua_State* L)
{
    lua_pushboolean(L, instance()->isGrabbed());

    return 1;
}

int Wrap_Mouse::setGrabbed(lua_State* L)
{
    bool grabbed = luax_toboolean(L, 1);
    instance()->setGrabbed(grabbed);

    return 0;
}

int Wrap_Mouse::getRelativeMode(lua_State* L)
{
    lua_pushboolean(L, instance()->getRelativeMode());

    return 1;
}

int Wrap_Mouse::setRelativeMode(lua_State* L)
{
    bool relative = luax_toboolean(L, 1);
    instance()->setRelativeMode(relative);

    return 0;
}

int Wrap_Mouse::isWiimoteButtonDown(lua_State* L)
{
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, instance()->isWiimoteButtonDown(button));

    return 1;
}

int Wrap_Mouse::wasWiimoteButtonPressed(lua_State* L)
{
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, instance()->wasWiimoteButtonPressed(button));

    return 1;
}

int Wrap_Mouse::wasWiimoteButtonReleased(lua_State* L)
{
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, instance()->wasWiimoteButtonReleased(button));

    return 1;
}

static constexpr luaL_Reg functions[] =
{
    { "getPosition",              Wrap_Mouse::getPosition              },
    { "setPosition",              Wrap_Mouse::setPosition              },
    { "isVisible",                Wrap_Mouse::isVisible                },
    { "setVisible",               Wrap_Mouse::setVisible               },
    { "isGrabbed",                Wrap_Mouse::isGrabbed                },
    { "setGrabbed",               Wrap_Mouse::setGrabbed               },
    { "getRelativeMode",          Wrap_Mouse::getRelativeMode          },
    { "setRelativeMode",          Wrap_Mouse::setRelativeMode          },
    { "isWiimoteButtonDown",      Wrap_Mouse::isWiimoteButtonDown      },
    { "wasWiimoteButtonPressed",  Wrap_Mouse::wasWiimoteButtonPressed  },
    { "wasWiimoteButtonReleased", Wrap_Mouse::wasWiimoteButtonReleased }
};

int Wrap_Mouse::open(lua_State* L)
{
    auto* instance = instance();
    if (instance == nullptr)
        luax_catchexcept(L, [&]() { instance = new Mouse(); });
    else
        instance->retain();

    WrappedModule module {};
    module.instance  = instance;
    module.name      = "mouse";
    module.type      = &Module::type;
    module.functions = functions;

    return luax_register_module(L, module);
}
