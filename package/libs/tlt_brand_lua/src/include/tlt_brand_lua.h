#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <libbrand/brand.h>

static int get_string(lua_State *L);
int luaopen_tlt_brand_lua(lua_State *L);
