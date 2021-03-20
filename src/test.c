#include "test.h"

int test_string_arg ( lua_State *L ) {
	if ( !lua_isstring( L, 1 ) || lua_isnumber( L, 1 ) ) {
		//pop the value...
		lua_pop( L, 1 );
		lua_error( L, "Got a bad value." );
		return 0; 
	}
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


