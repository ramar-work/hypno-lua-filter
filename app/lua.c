#include "lua.h"

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


struct action {
	const char *action, *dirname;
	int mlen, vlen, qlen, type;
	const char **model;
	const char **view;
	const char **query;
	const char *ctype;
};


struct set { 
	int isetlen;
	struct iset { char *route; int index; } **isetlist;
	zTable *src;
};


static const char *bnames[] = { "app", "views", "query", NULL }; 
static const char *builtins[] = { "model", "views", "view", "query", NULL };
static const char *bext[] = { "lua", "tpl", "sql", NULL };

static const struct { 
	const char *dirname, *reserved, *extension;
} bu_names [] = {
	{ "app", "model", "lua" }
,	{ "sql", "query", "sql" }
,	{ "views", "views", "tpl" }
};



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

#if 1
	//Load the string, execute
	if (( lerr = luaL_loadfile( L, f )) != LUA_OK ) {
	//if (( lerr = luaL_loadstring( L, (char *)ff )) != LUA_OK ) {
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

#if 1
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
#endif
#endif

	return 1;	
}



//Convert Lua tables to regular tables
int lua_to_table( lua_State *L, int index, zTable *t ) {
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


int is_reserved( const char *a ) {
	for ( const char **b = builtins; *b; b++ ) {
		if ( strcmp( a, *b ) == 0 ) return 1;
	}
	return 0;
}



int make_route_list ( zKeyval *kv, int i, void *p ) {
	struct set *tt = (struct set *)p;
	if ( kv->key.type == LITE_TXT && !is_reserved( kv->key.v.vchar ) ) {
		char key[ 2048 ] = { 0 };
		lt_get_full_key( tt->src, i, (unsigned char *)&key, sizeof( key ) );
		struct iset *ii = malloc( sizeof( struct iset ) );
		ii->index = i, ii->route = zhttp_dupstr( &key[ 6 ] ), *ii->route = '/';
		add_item( &(tt->isetlist), ii, struct iset *, &tt->isetlen );
	}
	return 1;	
}



int make_action_list ( zKeyval *kv, int i, void *p ) {
	struct action *tt = (struct action *)p;

	//
	if ( kv->key.type == LITE_TXT && is_reserved( kv->key.v.vchar ) ) {
		char *key = kv->key.v.vchar;
		if ( !strcmp( key, "model" ) )
			tt->action = builtins[ 0 ], tt->dirname = bnames[ 0 ], tt->type = kv->value.type;
		else if ( !strcmp( key, "view" ) || !strcmp( key, "views" ) ) {
			tt->action = builtins[ 1 ], tt->dirname = bnames[ 1 ], tt->type = kv->value.type;
		}
	}

	if ( tt->action && kv->value.type == LITE_TXT && memchr( "mvq", *tt->action, 2 ) ) {
		char buf[ 128 ] = { 0 };
		void ***vp = NULL;
		const char *ext;
		int *len = 0;

		//fprintf( stderr, "%s\n", buf );
		if ( *tt->action == 'm' && kv->value.type == LITE_TXT ) {
			vp = (void ***)&tt->model, len = &tt->mlen, ext = bext[ 0 ];
		}
		else if ( *tt->action == 'v' && kv->value.type == LITE_TXT ) {
			vp = (void ***)&tt->view, len = &tt->vlen, ext = bext[ 1 ];
		}

		snprintf( buf, sizeof( buf ) - 1, "%s/%s.%s", tt->dirname, kv->value.v.vchar, ext );
		add_item( vp, zhttp_dupstr( buf ), char *, len );

	}

	if ( kv->key.type == LITE_TRM || tt->type == LITE_TXT ) {
		tt->action = 0;	
	}
	return 1;	
}


//The entry point for a Lua application
int lc (struct HTTPBody *req, struct HTTPBody *res ) {

	//Define variables and error positions...
	zTable csrc, *zconfig, *zroutes = NULL, *zmodel = NULL, *croute = NULL;
	char err[2048] = {0}, *models[ 64 ] = {0}, *views[ 64 ] = {0};
	const char *db, *fqdn, *title, *spath, *root;
	lua_State *L = NULL;
	int clen = 0;
	unsigned char *content = NULL;

	//Initialize
	L = luaL_newstate();
	zconfig = &csrc;

	//If this fails, do something
	if ( !lt_init( zconfig, NULL, 1024 ) ) {
		return http_error( res, 500, "%s", "lt_init out of memory." );
	}

	//Open the configuration file
	if ( !lua_exec_file( L, "config.lua", err, sizeof( err ) ) ) {
		return http_error( res, 500, "%s", err );
	}

	//If it's anything but a Lua table, we're in trouble
	if ( !lua_istable( L, 1 ) ) {
		return http_error( res, 500, "%s", err );
	}

	//Convert the Lua values to real values for extraction.
	if ( !lua_to_table( L, 1, zconfig ) ) {
		return http_error( res, 500, "%s", "Failed to convert Lua to zTable" );
	}

	//Destroy loaded table here...
	lua_pop( L, 1 );

	//Get the rotues from the config file.
	if ( !( zroutes = lt_copy_by_key( zconfig, "routes" ) ) ) {
		return http_error( res, 500, "%s", "Failed to copy routes from config." );
	}

	//Turn the routes into a list of strings, and search for a match
	struct set p = { .src = zroutes };
	lt_exec_complex( zroutes, 1, zroutes->count, &p, make_route_list );
	
	//Loop through the routes
	struct action pp = {0};
	for ( struct iset **lroutes = p.isetlist; *lroutes; lroutes++ ) {
		if ( route_resolve( req->path, (*lroutes)->route ) ) {
			//get hash and put all of the keys into the right place
			croute = lt_copy_table( zroutes, (*lroutes)->index );
			lt_dump( croute );
			pp.action = 0;
			lt_exec_complex( croute, 1, croute->count, &pp, make_action_list );
			break;
		}
	}

	//Die when unavailable...
	if ( !croute ) {
		return http_error( res, 404, "Couldn't find path at %s\n", req->path );
	}

	//Load and run all the models 
	zTable model = {0};	
	zmodel = &model;
	lt_init( zmodel, NULL, 128 );

	//Each model needs to run, but it needs to insert itself into the calling context...
	for ( const char **m = pp.model; m && *m; m++ ) {
		fprintf( stderr, "opening model: %s\n", *m );
		char err[ 2048 ] = { 0 };

		//First open whatever will be used for routes. 
		if ( !lua_exec_file( L, *m, err, sizeof( err ) ) ) {
			return http_error( res, 500, "%s", err );
		}

		if ( !lua_istable( L, 1 ) ) {
			return http_error( res, 500, "%s", err );
		}

		if ( !lua_to_table( L, 1, zmodel ) ) {
			return http_error( res, 500, "%s", "Failed to convert Lua to zTable" );
		}

		//Destroy loaded table here...
		lua_pop( L, 1 );
		
		//If you want to execute multiple at a time, you might need to 
		//put things back on the stack...	
		//lt_dump( zmodel );
	} 

	//Load and run all the views
	for ( const char **v = pp.view; v && *v; v++ ) {
		//fprintf( stderr, "opening view: %s\n", *v );
		int len = 0, renlen = 0;
		unsigned char *src, *render;
		zRender * rz = zrender_init();
		zrender_set_default_dialect( rz );
		zrender_set_fetchdata( rz, zmodel );
		if ( !( src = read_file( *v, &len, err, sizeof( err ) )	) || !len ) {
			return http_error( res, 500, "%s", err );
		}
		if ( !( render = zrender_render( rz, src, strlen((char *)src), &renlen ) ) ) {
			return http_error( res, 500, "%s", "Renderer error." );
		}
		zhttp_append_to_uint8t( &content, &clen, render, renlen ); 
		zrender_free( rz );
		free( src );
	}

	//Return the finished message if we got this far
	res->clen = clen;
	http_set_status( res, 200 ); 
	http_set_ctype( res, "text/html" );
	http_set_content( res, content, clen ); 
	if ( !http_finalize_response( res, err, sizeof(err) ) ) {
		return http_error( res, 500, err );
	}
	free( content );
	return 1;
}

