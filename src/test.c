#include "test.h"

int test_string_arg ( lua_State *L ) {
	lua_isstring( L, 1 );
	return 1;
}

int test_numeric_arg ( lua_State *L ) {
	lua_isnumber( L, 1 );
	return 1;
}

int test_table_arg ( lua_State *L ) {
	return 1;
}

int test_missing_arg ( lua_State *L ) {
	return 1;
}


