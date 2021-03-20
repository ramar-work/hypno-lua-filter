/* --------------------------------------------- *
 * echo.c
 * ------
 *
 * All functions take an argument that matches
 * the function name. (e.g. echo.string takes 
 * a string, and so forth).  If the argument
 * does not match, die.
 * --------------------------------------------- */
#include "echo.h"
#include "lua.h"

int echo_numeric_arg ( lua_State *L ) {
	int a = 0;
	const int arg = 1;
#if 0
	a = luaL_checknumber( L, 1 );
#else
	if ( !lua_isnumber( L, arg ) ) {
	#if 0
		lua_Debug ar;
		lua_getinfo( L, ">S", &ar );
		fprintf( stderr, "Debug:\n" ); 
		fprintf( stderr, "%d\n", ar.event );
		fprintf( stderr, "%d\n", ar.currentline );
		fprintf( stderr, "%d\n", ar.linedefined );
		fprintf( stderr, "%d\n", ar.lastlinedefined );
		fprintf( stderr, "%c\n", ar.nups );
		fprintf( stderr, "%c\n", ar.nparams );
		fprintf( stderr, "%s\n", ar.name );
		fprintf( stderr, "%s\n", ar.namewhat );
		fprintf( stderr, "%s\n", ar.what );
		fprintf( stderr, "%s\n", ar.source );
	#endif
		return luaL_error( L, "echo.number: argument %d not a number.", arg ); 
	}

	a = lua_tonumber( L, 1 );
	lua_pop( L, 1 );
#endif
	lua_pushnumber( L, a );
	return 1;
}

int echo_string_arg ( lua_State *L ) {
	const int arg = 1;
	const char *a, f[] = "echo.string";
#if 0
	luaL_checkstring( L, 1 );
#else
	if ( !lua_isstring( L, arg ) ) {
		return luaL_error( L, "%s: argument %d not a number.", f, arg ); 
	}
	a = lua_tostring( L, arg );
#endif
	return 1;
}

int echo_table_arg ( lua_State *L ) {
	return 0;
}

struct luaL_Reg echo_set[] = {
	{ "string", echo_string_arg }
,	{ "number", echo_numeric_arg }
,	{ "table", echo_table_arg }
,	{ NULL }
};


