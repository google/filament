#include <stdio.h>

#include "lua.h"
#include "lauxlib.h"

static int smile(lua_State *L)
{
    (void) L;  // Unused
    printf("%s\n", ":-)");
    return 0;
}

int LUA_API luaopen_lua_dll(lua_State *L)
{
    static const struct luaL_Reg api[] = {
        {"smile", smile},
        {NULL, NULL},
    };
    luaL_openlib(L, "lua_dll", api, 0);
    return 1;
}
