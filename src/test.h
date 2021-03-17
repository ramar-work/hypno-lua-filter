#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int test_string_arg ( lua_State *L );

int test_numeric_arg ( lua_State *L );

int test_table_arg ( lua_State *L );

int test_missing_arg ( lua_State *L );

struct luaL_Reg test[] = {
	{ "string", test_string_arg }
,	{ "numeric", test_numeric_arg }
,	{ "table", test_table_arg }
,	{ "missing", test_missing_arg }
,	{ NULL }
};

