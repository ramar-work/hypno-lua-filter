#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <zhttp.h>
#include <zhasher.h>
#include <errno.h>
#include <sys/stat.h>
#include <megadeth.h>

#define FPRINTF( ... ) \
	fprintf( stderr, __VA_ARGS__ )

#if 0
//Load Lua libraries
static lua_State * lua_load_libs( lua_State **L ) {
	if ( !( *L = luaL_newstate() ) ) {
		return NULL;
	}	
	luaL_openlibs( *L );
	return *L;
}
#endif



//A better load file
int lua_exec_file( lua_State *L, const char *f, char *err, int errlen ) {
	int len = 0, lerr = 0;
	struct stat check;

	if ( !f || !strlen( f ) ) {
		snprintf( err, errlen, "%s", "No filename supplied to load or execute." );
		return 0;
	}

	//Since this is supposed to accept a file, why not just check for existence?
	if ( stat( f, &check ) == -1 ) {
		snprintf( err, errlen, "File %s inaccessible: %s.", f, strerror(errno) );
		return 0;
	}

	if ( check.st_size == 0 ) {
		snprintf( err, errlen, "File %s is zero-length.  Nothing to execute.", f );
		return 0;
	}

	//Load the string, execute
	if (( lerr = luaL_loadfile( L, f )) != LUA_OK ) {
		if ( lerr == LUA_ERRSYNTAX )
			len = snprintf( err, errlen, "Syntax error at %s: ", f );
		else if ( lerr == LUA_ERRMEM )
			len = snprintf( err, errlen, "zWalkerory allocation error at %s: ", f );
	#ifdef LUA_53
		else if ( lerr == LUA_ERRGCMM )
			len = snprintf( err, errlen, "GC meta-method error at %s: ", f );
	#endif
		else if ( lerr == LUA_ERRFILE )
			len = snprintf( err, errlen, "File access error at %s: ", f );
		else {
			len = snprintf( err, errlen, "Unknown error occurred at %s: ", f );
		}
	
		errlen -= len;	
		snprintf( &err[ len ], errlen, "%s\n", (char *)lua_tostring( L, -1 ) );
		//fprintf(stderr, "LUA LOAD ERROR: %s, %s", err, (char *)lua_tostring( L, -1 ) );
		lua_pop( L, lua_gettop( L ) );
		return 0;	
	}

	//Then execute
	if (( lerr = lua_pcall( L, 0, LUA_MULTRET, 0 ) ) != LUA_OK ) {
		if ( lerr == LUA_ERRRUN ) 
			len = snprintf( err, errlen, "Runtime error when executing %s: ", f );
		else if ( lerr == LUA_ERRMEM ) 
			len = snprintf( err, errlen, "zWalkerory allocation error at %s: ", f );
		else if ( lerr == LUA_ERRERR ) 
			len = snprintf( err, errlen, "Error while running message handler for %s: ", f );
	#ifdef LUA_53
		else if ( lerr == LUA_ERRGCMM ) {
			len = snprintf( err, errlen, "Error while running __gc metamethod at %s: ", f );
		}
	#endif

		errlen -= len;	
		snprintf( &err[ len ], errlen, "%s\n", (char *)lua_tostring( L, -1 ) );
		//fprintf(stderr, "LUA EXEC ERROR: %s, %s", err, (char *)lua_tostring( L, -1 ) );	
		lua_pop( L, lua_gettop( L ) );
		return 0;	
	}
	return 1;	
}



//Convert Lua tables to regular tables
int lua_to_table (lua_State *L, int index, zTable *t ) {
	static int sd;
	lua_pushnil( L );
	FPRINTF( "Current stack depth: %d\n", sd++ );

	while ( lua_next( L, index ) != 0 ) {
		int kt, vt;
		FPRINTF( "key, value: " );

		//This should pop both keys...
		FPRINTF( "%s, %s\n", lua_typename( L, lua_type(L, -2 )), lua_typename( L, lua_type(L, -1 )));

		//Keys
		if (( kt = lua_type( L, -2 )) == LUA_TNUMBER )
			FPRINTF( "key: %lld\n", (long long)lua_tointeger( L, -2 ));
		else if ( kt  == LUA_TSTRING )
			FPRINTF( "key: %s\n", lua_tostring( L, -2 ));

		//Values
		if (( vt = lua_type( L, -1 )) == LUA_TNUMBER )
			FPRINTF( "val: %lld\n", (long long)lua_tointeger( L, -1 ));
		else if ( vt  == LUA_TSTRING )
			FPRINTF( "val: %s\n", lua_tostring( L, -1 ));

		//Get key (remember Lua indices always start at 1.  Hence the minus.
		if (( kt = lua_type( L, -2 )) == LUA_TNUMBER )
			lt_addintkey( t, lua_tointeger( L, -2 ) - 1);
		else if ( kt  == LUA_TSTRING )
			lt_addtextkey( t, (char *)lua_tostring( L, -2 ));

		//Get value
		if (( vt = lua_type( L, -1 )) == LUA_TNUMBER )
			lt_addintvalue( t, lua_tointeger( L, -1 ));
		else if ( vt  == LUA_TSTRING )
			lt_addtextvalue( t, (char *)lua_tostring( L, -1 ));
		else if ( vt == LUA_TTABLE ) {
			lt_descend( t );
			FPRINTF( "Descending because value at %d is table...\n", -1 );
			//lua_loop( L );
			lua_to_table( L, index + 2, t ); 
			lt_ascend( t );
			sd--;
		}

		FPRINTF( "popping last two values...\n" );
		if ( vt == LUA_TNUMBER || vt == LUA_TSTRING ) {
			lt_finalize( t );
		}
		lua_pop(L, 1);
	}

	lt_lock( t );
	return 1;
}



//The entry point for a Lua application
int lc (struct HTTPBody *req, struct HTTPBody *res, char *err, int errlen) {
	
	//Define variables and error positions...
	zTable csrc, *config; // = lt_init( NULL, 1024 )
	//char err[ 2048 ]; 
	char *models[ 64 ], *views[ 64 ];
	lua_State *L = NULL;
	const char path[] = "config.lua";

	//Declare
	config = &csrc;
	memset( err, 0, errlen );
	memset( models, 0, sizeof( models ) );
	memset( views, 0, sizeof( views ) );

	//If this fails, do something
	if ( !lt_init( config, NULL, 1024 ) ) {
		return http_error( res, 500, "%s", "lt_init out of memory." );
	}

	//First open whatever will be used for routes. 
	if ( !lua_exec_file( L, path, err, sizeof( err ) ) ) {
		return http_error( res, 500, "%s", err );
	}

	if ( !lua_istable( L, 1 ) ) {
		return http_error( res, 500, "%s", err );
	}

	if ( !lua_to_table( L, 1, config ) ) {
		return http_error( res, 500, "%s", "Failed to convert Lua to zTable" );
	}

	//destroy Lua here?
	lua_pop( L, 1 );




	//Define more stuff
	const char *db = lt_text( config, "db" );		
	const char *fqdn = lt_text( config, "fqdn" );		
	const char *title = lt_text( config, "title" );		
	const char *spath = lt_text( config, "spath" );		
	const char *root = lt_text( config, "root" );

	lt_dump( config );

	//Open whatever files (or functions) that are specified by the route
	//zTable routes = lt_table( &config, "routes" ); //there is something that will do the job...


	
	//Open the views and do stuff

	//Return the finished message if we got this far

	return 1;
}

