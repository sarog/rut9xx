#include "tlt_brand_lua.h"

static int brandlib_perror( lua_State *L, const char *message )
{
	lua_pushnil(L);
	lua_pushstring(L, message);

	return 2;
}

static int get_string(lua_State *L){
	int number = luaL_checkint( L, 1 );
	char *output = brand3(number);

	lua_pushstring(L, output);

	return 1;
}

static const struct luaL_reg brandlib[]={
	{"print", get_string},
	{NULL, NULL}
};

int luaopen_tlt_brand_lua(lua_State *L){
	luaL_register(L, "brand", brandlib);
	return 1;
}
