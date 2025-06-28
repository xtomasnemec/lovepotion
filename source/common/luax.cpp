#include "common/luax.hpp"
#include "common/config.hpp"

#include "common/Module.hpp"
#include "common/Object.hpp"
#include "common/Reference.hpp"

#include <cmath>
#include <cstring>
#include <cstdio>

#include <algorithm>

namespace love
{
    // #region Startup

    int luax_preload(lua_State* L, lua_CFunction function, const char* name)
    {
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "preload");
        lua_pushcfunction(L, function);
        lua_setfield(L, -2, name);
        lua_pop(L, 2);

        return 0;
    }

    int luax_insistglobal(lua_State* L, const char* name)
    {
        lua_getglobal(L, name);

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setglobal(L, name);
        }

        return 1;
    }

    lua_State* luax_insistpinnedthread(lua_State* L)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);

        if (lua_isnoneornil(L, -1))
        {
            lua_pop(L, 1);
            lua_pushthread(L);
            lua_pushvalue(L, -1);
            lua_setfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);
        }

        lua_State* thread = lua_tothread(L, -1);
        lua_pop(L, 1);

        return thread;
    }

    lua_State* luax_getpinnedthread(lua_State* L)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);
        lua_State* thread = lua_tothread(L, -1);
        lua_pop(L, 1);

        return thread;
    }

    int luax_insist(lua_State* L, int index, const char* name)
    {
        if (index < 0 && index > LUA_REGISTRYINDEX)
            index += lua_gettop(L) + 1;

        lua_getfield(L, index, name);

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setfield(L, index, name);
        }

        return 1;
    }

    int luax_insistlove(lua_State* L, const char* name)
    {
        lua_getglobal(L, "love");
        luax_insist(L, -1, name);
        lua_replace(L, -2);

        return 1;
    }

    int luax_getlove(lua_State* L, const char* name)
    {
        lua_getglobal(L, "love");

        if (!lua_isnil(L, -1))
        {
            lua_getfield(L, -1, name);
            lua_replace(L, -2);
        }

        return 1;
    }

    int luax_insistregistry(lua_State* L, Registry registry)
    {
        switch (registry)
        {
            case REGISTRY_MODULES:
                return luax_insistlove(L, MODULES_REGISTRY_KEY);
            case REGISTRY_OBJECTS:
                return luax_insist(L, LUA_REGISTRYINDEX, OBJECTS_REGISTRY_KEY);
            default:
                return luaL_error(L, "Attempted to use invalid registry.");
        }
    }

    int luax_getregistry(lua_State* L, Registry registry)
    {
        switch (registry)
        {
            case REGISTRY_MODULES:
                return luax_getlove(L, MODULES_REGISTRY_KEY);
            case REGISTRY_OBJECTS:
                lua_getfield(L, LUA_REGISTRYINDEX, OBJECTS_REGISTRY_KEY);
                return 1;
            default:
                return luaL_error(L, "Attempted to use invalid registry.");
        }
    }

    int luax_resume(lua_State* L, int argc, int* nres)
    {
#ifdef __WIIU__
        static int resumeCallCount = 0;
        resumeCallCount++;
        
        printf("[LUAX_RESUME] Starting lua_resume() call #%d with argc=%d\n", resumeCallCount, argc);
        fflush(stdout);
        
        // Also log to file for debugging
        FILE* logFile = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile) {
            fprintf(logFile, "luax_resume() call #%d with argc=%d\n", resumeCallCount, argc);
            fflush(logFile);
            fclose(logFile);
        }
        
        // Log current Lua stack state
        FILE* logFile3 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile3) {
            fprintf(logFile3, "Lua stack top: %d\n", lua_gettop(L));
            fflush(logFile3);
            fclose(logFile3);
        }
        
        printf("[LUAX_RESUME] About to call lua_resume...\n");
        fflush(stdout);
        
        // Add file logging too  
        FILE* logFileA = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFileA) {
            fprintf(logFileA, "About to call lua_resume()...\n");
            fflush(logFileA);
            fclose(logFileA);
        }
#endif

#if LUA_VERSION_NUM >= 504
        int result = lua_resume(L, nullptr, argc, nres);
#elif LUA_VERSION_NUM >= 502
        LOVE_UNUSED(nres);
        int result = lua_resume(L, nullptr, argc);
#else
        LOVE_UNUSED(nres);
        int result = lua_resume(L, argc);
#endif

#ifdef __WIIU__
        // Log immediately after lua_resume returns
        printf("[LUAX_RESUME] lua_resume() call #%d returned %d", resumeCallCount, result);
        if (result == LUA_YIELD) {
            printf(" (LUA_YIELD - coroutine yielded)\n");
        } else if (result == 0) {
            printf(" (LUA_OK - success)\n");
        } else {
            printf(" (ERROR - check for Lua error)\n");
        }
        fflush(stdout);
        
        // Also log to file for debugging with more detail
        FILE* logFile2 = fopen("fs:/vol/external01/simple_debug.log", "a");
        if (logFile2) {
            fprintf(logFile2, "lua_resume() call #%d returned %d", resumeCallCount, result);
            if (result == LUA_YIELD) {
                fprintf(logFile2, " (LUA_YIELD - coroutine yielded)\n");
            } else if (result == 0) {
                fprintf(logFile2, " (LUA_OK - success)\n");
            } else {
                fprintf(logFile2, " (ERROR - check for Lua error)\n");
            }
            fflush(logFile2);
            fclose(logFile2);
        }
#endif

        return result;
    }

    int luax_register_searcher(lua_State* L, lua_CFunction function, int position)
    {
        lua_getglobal(L, "package");

        if (lua_isnil(L, -1))
            return luaL_error(L, "Can't register searcher: package table does not exist.");

        lua_getfield(L, -1, "loaders");

        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            lua_getfield(L, -1, "searchers");
        }

        if (lua_isnil(L, -1))
            return luaL_error(L, "Can't register searcher: package.loaders table does not exist.");

        lua_pushcfunction(L, function);
        luax_table_insert(L, -2, -1, position);

        lua_pop(L, 3);

        return 0;
    }

    // #endregion

    // #region Objects

    void luax_pushobjectkey(lua_State* L, ObjectKey key)
    {
        if (luax_isfulllightuserdatasupported(L))
            lua_pushlightuserdata(L, (void*)key);
        else if (key > LUAX_MAX_OBJECT_KEY)
            luaL_error(L, E_POINTER_TOO_LARGE, key);
        else
            lua_pushnumber(L, (lua_Number)key);
    }

    static ObjectKey luax_computeobjectkey(lua_State* L, Object* object)
    {
        const size_t min_align = sizeof(void*) == 8 ? alignof(std::max_align_t) : 1;
        uintptr_t key          = (uintptr_t)object;

        if ((key & (min_align - 1)) != 0)
            luaL_error(L, E_UNEXPECTED_ALIGNMENT, object, (int)min_align);

        static const size_t shift = (size_t)std::log2(min_align);

        key >>= shift;
        return (ObjectKey)key;
    }

    static int w_release(lua_State* L)
    {
        Proxy* proxy   = (Proxy*)lua_touserdata(L, 1);
        Object* object = proxy->object;

        if (object != nullptr)
        {
            proxy->object = nullptr;
            object->release();

            luax_getregistry(L, REGISTRY_OBJECTS);

            if (lua_istable(L, -1))
            {
                ObjectKey key = luax_computeobjectkey(L, object);
                luax_pushobjectkey(L, key);
                lua_pushnil(L);
                lua_settable(L, -3);
            }

            lua_pop(L, 1);
        }

        luax_pushboolean(L, object != nullptr);

        return 1;
    }

    static int w__gc(lua_State* L)
    {
        Proxy* proxy = (Proxy*)lua_touserdata(L, 1);

        if (proxy->object != nullptr)
        {
            proxy->object->release();
            proxy->object = nullptr;
        }

        return 0;
    }

    static int w__tostring(lua_State* L)
    {
        Proxy* proxy = (Proxy*)lua_touserdata(L, 1);

        const char* name = lua_tostring(L, lua_upvalueindex(1));
        lua_pushfstring(L, "%s: %p", name, proxy->object);

        return 1;
    }

    static int w_type(lua_State* L)
    {
        lua_pushvalue(L, lua_upvalueindex(1));

        return 1;
    }

    static int w_typeOf(lua_State* L)
    {
        Proxy* proxy = (Proxy*)lua_touserdata(L, 1);
        Type* type   = luax_type(L, 2);

        if (!type)
            luax_pushboolean(L, false);
        else
            luax_pushboolean(L, proxy->type->isA(*type));

        return 1;
    }

    static int w__eq(lua_State* L)
    {
        Proxy* proxy1 = (Proxy*)lua_touserdata(L, 1);
        Proxy* proxy2 = (Proxy*)lua_touserdata(L, 2);

        luax_pushboolean(L, proxy1->object == proxy2->object && proxy1->object != nullptr);

        return 1;
    }

    // #endregion

    // #region Helpers

    int luax_isfulllightuserdatasupported(lua_State* L)
    {
        static bool checked   = false;
        static bool supported = false;

        if (sizeof(void*) == 4)
            return true;

        if (!checked)
        {
            lua_pushcclosure(
                L,
                [](lua_State* L) -> int {
                    lua_pushlightuserdata(L, (void*)(~((size_t)0)));
                    return 1;
                },
                0);

            supported = lua_pcall(L, 0, 1, 0) == 0;
            checked   = true;

            lua_pop(L, 1);
        }

        return supported;
    }

    Type* luax_type(lua_State* L, int index)
    {
        return Type::byName(luaL_checkstring(L, index));
    }

    int luax_typeerror(lua_State* L, int argc, const char* name)
    {
        int argtype             = lua_type(L, argc);
        const char* argTypeName = nullptr;

        if (argtype == LUA_TUSERDATA && luaL_getmetafield(L, argc, "type") != 0)
        {
            lua_pushvalue(L, argc);
            if (lua_pcall(L, 1, 1, 0) == 0 && lua_type(L, -1) == LUA_TSTRING)
            {
                argTypeName = lua_tostring(L, -1);
                if (!Type::byName(argTypeName))
                    argTypeName = nullptr;
            }
        }

        if (argTypeName == nullptr)
            argTypeName = lua_typename(L, argtype);

        const char* message = lua_pushfstring(L, "%s expected, got %s", name, argTypeName);
        return luaL_argerror(L, argc, message);
    }

    bool luax_istype(lua_State* L, int index, Type& type)
    {
        if (lua_type(L, index) != LUA_TUSERDATA)
            return false;

        Proxy* userdata = (Proxy*)lua_touserdata(L, index);

        if (userdata->type != nullptr)
            return userdata->type->isA(type);
        else
            return false;
    }

    Proxy* luax_tryextractproxy(lua_State* L, int index)
    {
        Proxy* proxy = (Proxy*)lua_touserdata(L, index);

        if (!proxy || !proxy->type)
            return nullptr;

        if (dynamic_cast<Object*>(proxy->object) != nullptr)
            return proxy;

        return nullptr;
    }

    void luax_rawnewtype(lua_State* L, Type& type, Object* object)
    {
        Proxy* proxy = (Proxy*)lua_newuserdata(L, sizeof(Proxy));
        object->retain();

        proxy->object = object;
        proxy->type   = &type;

        const char* name = type.getName();
        luaL_newmetatable(L, name);

        lua_getfield(L, -1, "__gc");
        bool has_gc = !lua_isnoneornil(L, -1);
        lua_pop(L, 1);

        if (!has_gc)
        {
            lua_pushcfunction(L, w__gc);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }

    void luax_pushtype(lua_State* L, Type& type, Object* object)
    {
        if (object == nullptr)
        {
            lua_pushnil(L);
            return;
        }

        luax_getregistry(L, REGISTRY_OBJECTS);

        if (lua_isnoneornil(L, -1))
        {
            lua_pop(L, 1);
            return luax_rawnewtype(L, type, object);
        }

        ObjectKey key = luax_computeobjectkey(L, object);
        luax_pushobjectkey(L, key);

        lua_gettable(L, -2);

        if (lua_type(L, -1) != LUA_TUSERDATA)
        {
            lua_pop(L, 1);

            luax_rawnewtype(L, type, object);

            luax_pushobjectkey(L, key);
            lua_pushvalue(L, -2);

            lua_settable(L, -4);
        }

        lua_remove(L, -2);
    }

    void luax_pushvariant(lua_State* L, const Variant& variant)
    {
        const Variant::Data& data = variant.getData();

        switch (variant.getType())
        {
            case Variant::BOOLEAN:
                lua_pushboolean(L, data.boolean);
                break;
            case Variant::NUMBER:
                lua_pushnumber(L, data.number);
                break;
            case Variant::STRING:
                lua_pushlstring(L, data.string->string, data.string->length);
                break;
            case Variant::SMALLSTRING:
                lua_pushlstring(L, data.smallstring.str, data.smallstring.len);
                break;
            case Variant::LUSERDATA:
                lua_pushlightuserdata(L, data.userdata);
                break;
            case Variant::LOVEOBJECT:
                luax_pushtype(L, *data.proxy.type, data.proxy.object);
                break;
            case Variant::TABLE:
            {
                auto& table = data.table->pairs;
                int tsize   = (int)table.size();

                lua_createtable(L, 0, tsize);

                for (int i = 0; i < tsize; ++i)
                {
                    std::pair<Variant, Variant>& kv = table[i];
                    luax_pushvariant(L, kv.first);
                    luax_pushvariant(L, kv.second);
                    lua_settable(L, -3);
                }

                break;
            }
            case Variant::NIL:
            default:
                lua_pushnil(L);
                break;
        }
    }

    Variant luax_checkvariant(lua_State* L, int index, bool allowuserdata, std::set<const void*>* tableSet)
    {
        size_t len;
        const char* str;
        Proxy* p = nullptr;

        if (index < 0) // Fix the stack position, we might modify it later
            index += lua_gettop(L) + 1;

        switch (lua_type(L, index))
        {
            case LUA_TBOOLEAN:
                return Variant(luax_toboolean(L, index));
            case LUA_TNUMBER:
                return Variant(lua_tonumber(L, index));
            case LUA_TSTRING:
                str = lua_tolstring(L, index, &len);
                return Variant(str, len);
            case LUA_TLIGHTUSERDATA:
                return Variant(lua_touserdata(L, index));
            case LUA_TUSERDATA:
                if (!allowuserdata)
                {
                    luax_typeerror(L, index, "copyable Lua value");
                    return Variant();
                }
                p = luax_tryextractproxy(L, index);
                if (p != nullptr)
                    return Variant(p->type, p->object);
                else
                {
                    luax_typeerror(L, index, "love type");
                    return Variant();
                }
            case LUA_TNIL:
                return Variant();
            case LUA_TTABLE:
            {
                bool success = true;
                std::set<const void*> topTableSet;

                // We can use a pointer to a stack-allocated variable because it's
                // never used after the stack-allocated variable is destroyed.
                if (tableSet == nullptr)
                    tableSet = &topTableSet;

                // Now make sure this table wasn't already serialised
                const void* tablePointer = lua_topointer(L, index);
                {
                    auto result = tableSet->insert(tablePointer);
                    if (!result.second) // insertion failed
                        throw love::Exception("Cycle detected in table");
                }

                Variant::SharedTable* table = new Variant::SharedTable();

                size_t len = luax_objlen(L, -1);
                if (len > 0)
                    table->pairs.reserve(len);

                lua_pushnil(L);

                while (lua_next(L, index))
                {
                    // clang-format off
                    table->pairs.emplace_back(
                        luax_checkvariant(L, -2, allowuserdata, tableSet),
                        luax_checkvariant(L, -1, allowuserdata, tableSet)
                    );
                    // clang-format on
                    lua_pop(L, 1);

                    const auto& p = table->pairs.back();
                    if (p.first.getType() == Variant::UNKNOWN || p.second.getType() == Variant::UNKNOWN)
                    {
                        success = false;
                        break;
                    }
                }

                // And remove the table from the set again
                tableSet->erase(tablePointer);

                if (success)
                    return Variant(table);
                else
                    table->release();
            }
            break;
        }

        return Variant::unknown();
    }

    bool luax_toboolean(lua_State* L, int index)
    {
        return (lua_toboolean(L, index) != 0);
    }

    bool luax_checkboolean(lua_State* L, int index)
    {
        luaL_checktype(L, index, LUA_TBOOLEAN);
        return luax_toboolean(L, index);
    }

    void luax_pushboolean(lua_State* L, bool boolean)
    {
        lua_pushboolean(L, boolean ? 1 : 0);
    }

    bool luax_optboolean(lua_State* L, int index, bool default_value)
    {
        if (lua_isboolean(L, index) == 1)
            return (lua_toboolean(L, index) == 1 ? true : false);

        return default_value;
    }

    std::string luax_tostring(lua_State* L, int index)
    {
        size_t length;
        const char* string = lua_tolstring(L, index, &length);

        return std::string(string, length);
    }

    std::string luax_checkstring(lua_State* L, int index)
    {
        size_t length;
        const char* string = luaL_checklstring(L, index, &length);

        return std::string(string, length);
    }

    void luax_pushstring(lua_State* L, std::string_view string)
    {
        lua_pushlstring(L, string.data(), string.size());
    }

    void luax_pushpointerasstring(lua_State* L, const void* pointer)
    {
        char string[sizeof(void*)] {};
        std::memcpy(string, &pointer, sizeof(void*));

        lua_pushlstring(L, string, sizeof(void*));
    }

    bool luax_checkboolflag(lua_State* L, int tableIndex, const char* key)
    {
        lua_getfield(L, tableIndex, key);

        bool result = false;

        if (lua_type(L, -1) != LUA_TBOOLEAN)
        {
            auto error = std::format("expected boolean field '{:s}' in table", key);
            luaL_argerror(L, tableIndex, error.c_str());
        }
        else
            result = luax_toboolean(L, -1);

        lua_pop(L, 1);

        return result;
    }

    bool luax_boolflag(lua_State* L, int index, const char* name, bool default_value)
    {
        lua_getfield(L, index, name);

        bool result;

        if (lua_isnoneornil(L, -1))
            result = default_value;
        else
            result = lua_toboolean(L, -1) != 0;

        lua_pop(L, 1);

        return result;
    }

    int luax_checkintflag(lua_State* L, int tableIndex, const char* key)
    {
        lua_getfield(L, tableIndex, key);

        int result = 0;

        if (!lua_isnumber(L, -1))
        {
            auto error = std::format("expected integer field '{:s}' in table", key);
            luaL_argerror(L, tableIndex, error.c_str());
        }
        else
            result = luaL_checkinteger(L, -1);

        lua_pop(L, 1);

        return result;
    }

    int luax_intflag(lua_State* L, int index, const char* name, int default_value)
    {
        lua_getfield(L, index, name);

        int result;

        if (!lua_isnumber(L, -1))
            result = default_value;
        else
            result = lua_tointeger(L, -1);

        lua_pop(L, 1);

        return result;
    }

    int luax_enumerror(lua_State* L, const char* enumName, const char* value)
    {
        return luaL_error(L, "Invalid %s: %s", enumName, value);
    }

    int luax_ioerror(lua_State* L, const char* format, ...)
    {
        std::va_list args;
        va_start(args, format);

        lua_pushnil(L);
        lua_pushvfstring(L, format, args);

        va_end(args);

        return 2;
    }

    size_t luax_objlen(lua_State* L, int index)
    {
#if LUA_VERSION_NUM == 501
        return lua_objlen(L, index);
#else
        return lua_rawlen(L, index);
#endif
    }

    int luax_table_insert(lua_State* L, int tableIndex, int vIndex, int position)
    {
        if (tableIndex < 0)
            tableIndex = lua_gettop(L) + 1 + tableIndex;

        if (vIndex < 0)
            vIndex = lua_gettop(L) + 1 + vIndex;

        if (position == -1)
        {
            lua_pushvalue(L, vIndex);
            lua_rawseti(L, tableIndex, (int)luax_objlen(L, tableIndex) + 1);

            return 0;
        }
        else if (position < 0)
            position = (int)luax_objlen(L, tableIndex) + 1 + position;

        for (int i = (int)luax_objlen(L, tableIndex) + 1; i > position; i--)
        {
            lua_rawgeti(L, tableIndex, i - 1);
            lua_rawseti(L, tableIndex, i);
        }

        lua_pushvalue(L, vIndex);
        lua_rawseti(L, tableIndex, position);

        return 0;
    }

    int luax_require(lua_State* L, const char* name)
    {
        lua_getglobal(L, "require");
        lua_pushstring(L, name);
        lua_call(L, 1, 1);

        return 1;
    }

    bool luax_isarrayoftables(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
            return false;

        lua_rawgeti(L, index, 1);
        bool tableOfTables = lua_istable(L, -1);

        lua_pop(L, 1);

        return tableOfTables;
    }

    int luax_assert_nilerror(lua_State* L, int index)
    {
        if (lua_isnoneornil(L, index))
        {
            if (lua_isstring(L, index + 1))
                return luaL_error(L, lua_tostring(L, index + 1));
            else
                return luaL_error(L, "assertion failed!");
        }

        return 0;
    }

    int luax_getfunction(lua_State* L, const char* module, const char* function)
    {
        lua_getglobal(L, "love");
        if (lua_isnil(L, -1))
            return luaL_error(L, "Could not find global love!");

        lua_getfield(L, -1, module);
        if (lua_isnil(L, -1))
            return luaL_error(L, "Could not find module love.%s!", module);

        lua_getfield(L, -1, function);
        if (lua_isnil(L, -1))
            return luaL_error(L, "Could not find function love.%s.%s!", module, function);

        lua_remove(L, -2);
        lua_remove(L, -2);

        return 0;
    }

    int luax_convobj(lua_State* L, int index, const char* module, const char* function)
    {
        if (index < 0 && index > LUA_REGISTRYINDEX)
            index += lua_gettop(L) + 1;

        luax_getfunction(L, module, function);
        lua_pushvalue(L, index);
        lua_call(L, 1, 2);
        luax_assert_nilerror(L, -2);
        lua_pop(L, 1);
        lua_replace(L, index);

        return 0;
    }

    int luax_convobj(lua_State* L, std::span<int> indices, const char* module, const char* function)
    {
        luax_getfunction(L, module, function);

        for (int index : indices)
            lua_pushvalue(L, index);

        lua_call(L, (int)indices.size(), 2);
        luax_assert_nilerror(L, -2);
        lua_pop(L, 1);

        if (indices.size() > 0)
            lua_replace(L, indices[0]);

        return 0;
    }

    void luax_gettypemetatable(lua_State* L, const Type& type)
    {
        const char* name = type.getName();
        lua_getfield(L, LUA_REGISTRYINDEX, name);
    }

    void luax_runwrapper(lua_State* L, const char* filedata, size_t datalen, const char* filename,
                         const Type& type)
    {
        luax_gettypemetatable(L, type);

        if (lua_istable(L, -1))
        {
            const auto name = std::format("=[love \"{:s}\"]", filename);
            luaL_loadbuffer(L, filedata, datalen, name.c_str());
            lua_pushvalue(L, -2);
            lua_pushnil(L);
            lua_call(L, 2, 0);
        }

        lua_pop(L, 1);
    }

    // #endregion

    // #region Registry

    int luax_register_module(lua_State* L, const WrappedModule& module)
    {
        module.type->initialize();

        luax_insistregistry(L, REGISTRY_MODULES);
        Proxy* proxy = (Proxy*)lua_newuserdata(L, sizeof(Proxy));

        proxy->object = module.instance;
        proxy->type   = module.type;

        luaL_newmetatable(L, module.instance->getName());
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, w__gc);
        lua_setfield(L, -2, "__gc");

        lua_setmetatable(L, -2);
        lua_setfield(L, -2, module.name);
        lua_pop(L, 1);

        luax_insistglobal(L, "love");

        lua_newtable(L);

        if (!module.functions.empty())
            luax_register_type_inner(L, module.functions);

        if (!module.platformFunctions.empty())
            luax_register_type_inner(L, module.platformFunctions);

        if (!module.types.empty())
            luax_register_types(L, module.types);

        lua_pushvalue(L, -1);
        lua_setfield(L, -3, module.name);
        lua_remove(L, -2);

        return 1;
    }

    void luax_register_type_init(lua_State* L, Type* type)
    {
        type->initialize();

        luax_getregistry(L, REGISTRY_OBJECTS);

        if (!lua_istable(L, -1))
        {
            lua_newtable(L);
            lua_replace(L, -2);

            lua_newtable(L);

            lua_pushliteral(L, "v");
            lua_setfield(L, -2, "__mode");

            lua_setmetatable(L, -2);

            lua_setfield(L, LUA_REGISTRYINDEX, OBJECTS_REGISTRY_KEY);
        }
        else
            lua_pop(L, 1);

        luaL_newmetatable(L, type->getName());

        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, w__gc);
        lua_setfield(L, -2, "__gc");

        lua_pushcfunction(L, w__eq);
        lua_setfield(L, -2, "__eq");

        lua_pushstring(L, type->getName());
        lua_pushcclosure(L, w__tostring, 1);
        lua_setfield(L, -2, "__tostring");

        lua_pushstring(L, type->getName());
        lua_pushcclosure(L, w_type, 1);
        lua_setfield(L, -2, "type");

        lua_pushcfunction(L, w_typeOf);
        lua_setfield(L, -2, "typeOf");

        lua_pushcfunction(L, w_release);
        lua_setfield(L, -2, "release");
    }

    void luax_register_type_inner(lua_State* L, std::span<const luaL_Reg> functions)
    {
        for (const auto& registry : functions)
        {
            lua_pushcfunction(L, registry.func);
            lua_setfield(L, -2, registry.name);
        }
    }

    void luax_register_types(lua_State* L, std::span<const lua_CFunction> types)
    {
        for (const auto& registry : types)
            registry(L);
    }

    int luax_traceback(lua_State* L)
    {
        if (!lua_isstring(L, 1))
            return 1;

        lua_getglobal(L, "debug");
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return 1;
        }

        lua_getfield(L, -1, "traceback");
        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 2);
            return 1;
        }

        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_call(L, 2, 1);

        return 1;
    }

    Reference* luax_refif(lua_State* L, int type)
    {
        Reference* r = nullptr;

        // Create a reference only if the test succeeds.
        if (lua_type(L, -1) == type)
            r = new Reference(L);
        else // Pop the value manually if it fails (done by Reference if it succeeds).
            lua_pop(L, 1);

        return r;
    }
    // #endregion
} // namespace love
