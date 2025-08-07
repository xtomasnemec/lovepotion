#include "modules/graphics/wrap_Mesh.hpp"
#include "modules/graphics/wrap_Texture.hpp"

using namespace love;

MeshBase* love::luax_checkmesh(lua_State* L, int idx)
{
    return luax_checktype<MeshBase>(L, idx);
}

int Wrap_Mesh::setVertices(lua_State* L)
{
    MeshBase* mesh = luax_checkmesh(L, 1);
    
    if (!lua_istable(L, 2))
        return luaL_error(L, "Expected table for vertices");
    
    std::vector<Vertex> vertices;
    size_t count = luax_objlen(L, 2);
    
    for (size_t i = 1; i <= count; i++)
    {
        lua_rawgeti(L, 2, i);
        
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return luaL_error(L, "Each vertex must be a table");
        }
        
        Vertex vertex = {};
        
        // Get position (required)
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        vertex.x = (float)luaL_checknumber(L, -2);
        vertex.y = (float)luaL_checknumber(L, -1);
        lua_pop(L, 2);
        
        // Get texture coordinates (optional)
        lua_rawgeti(L, -1, 3);
        lua_rawgeti(L, -2, 4);
        if (!lua_isnil(L, -2) && !lua_isnil(L, -1))
        {
            vertex.s = (float)luaL_checknumber(L, -2);
            vertex.t = (float)luaL_checknumber(L, -1);
        }
        lua_pop(L, 2);
        
        // Get color (optional)
        lua_rawgeti(L, -1, 5);
        lua_rawgeti(L, -2, 6);
        lua_rawgeti(L, -3, 7);
        lua_rawgeti(L, -4, 8);
        if (!lua_isnil(L, -4))
        {
            vertex.color.r = (float)luaL_checknumber(L, -4);
            vertex.color.g = (float)luaL_checknumber(L, -3);
            vertex.color.b = (float)luaL_checknumber(L, -2);
            vertex.color.a = (float)luaL_optnumber(L, -1, 1.0);
        }
        else
        {
            vertex.color.r = vertex.color.g = vertex.color.b = vertex.color.a = 1.0f;
        }
        lua_pop(L, 4);
        
        vertices.push_back(vertex);
        lua_pop(L, 1);
    }
    
    mesh->setVertices(vertices);
    return 0;
}

int Wrap_Mesh::getVertexCount(lua_State* L)
{
    MeshBase* mesh = luax_checkmesh(L, 1);
    lua_pushinteger(L, mesh->getVertexCount());
    return 1;
}

int Wrap_Mesh::setTexture(lua_State* L)
{
    MeshBase* mesh = luax_checkmesh(L, 1);
    
    if (lua_isnil(L, 2))
    {
        mesh->setTexture(nullptr);
    }
    else
    {
        TextureBase* texture = luax_checktexture(L, 2);
        mesh->setTexture(texture);
    }
    
    return 0;
}

int Wrap_Mesh::getTexture(lua_State* L)
{
    MeshBase* mesh = luax_checkmesh(L, 1);
    TextureBase* texture = mesh->getTexture();
    
    if (texture)
        luax_pushtype(L, texture);
    else
        lua_pushnil(L);
    
    return 1;
}

int Wrap_Mesh::setDrawMode(lua_State* L)
{
    MeshBase* mesh = luax_checkmesh(L, 1);
    const char* str = luaL_checkstring(L, 2);
    
    MeshBase::DrawMode mode;
    if (!MeshBase::getConstant(str, mode))
        return luaL_error(L, "Invalid draw mode: %s", str);
    
    mesh->setDrawMode(mode);
    return 0;
}

int Wrap_Mesh::getDrawMode(lua_State* L)
{
    MeshBase* mesh = luax_checkmesh(L, 1);
    MeshBase::DrawMode mode = mesh->getDrawMode();
    
    const char* str;
    if (!MeshBase::getConstant(mode, str))
        return luaL_error(L, "Unknown draw mode");
    
    lua_pushstring(L, str);
    return 1;
}

static constexpr luaL_Reg functions[] =
{
    { "setVertices",    Wrap_Mesh::setVertices    },
    { "getVertexCount", Wrap_Mesh::getVertexCount },
    { "setTexture",     Wrap_Mesh::setTexture     },
    { "getTexture",     Wrap_Mesh::getTexture     },
    { "setDrawMode",    Wrap_Mesh::setDrawMode    },
    { "getDrawMode",    Wrap_Mesh::getDrawMode    },
    { nullptr,          nullptr                   }
};

int love::open_mesh(lua_State* L)
{
    return luax_register_type(L, &MeshBase::type, functions);
}
