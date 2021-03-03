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

#define PRETTY_TABS( ct ) \
	fprintf( stderr, "%s", &"                    "[ 20 - ct ] );

static void lua_dumptable ( lua_State *L, int *pos, int *sd ) {
	lua_pushnil( L );
	FPRINTF( "*pos = %d\n", *pos );

	while ( lua_next( L, *pos ) != 0 ) {
		//Fancy printing
		//fprintf( stderr, "%s", &"\t\t\t\t\t\t\t\t\t\t"[ 10 - *sd ] );
		PRETTY_TABS( *sd );
		fprintf( stderr, "[%3d:%2d] => ", *pos, *sd );

		//Print both left and right side
		for ( int i = -2; i < 0; i++ ) {
			int t = lua_type( L, i );
			const char *type = lua_typename( L, t );

			if ( t == LUA_TSTRING )
				fprintf( stderr, "(%8s) %s", type, lua_tostring( L, i ));
			else if ( t == LUA_TFUNCTION )
				fprintf( stderr, "(%8s) %p", type, (void *)lua_tocfunction( L, i ) );
			else if ( t == LUA_TNUMBER )
				fprintf( stderr, "(%8s) %lld", type, (long long)lua_tointeger( L, i ));
			else if ( t == LUA_TBOOLEAN)
				fprintf( stderr, "(%8s) %s", type, lua_toboolean( L, i ) ? "true" : "false" );
			else if ( t == LUA_TTHREAD )
				fprintf( stderr, "(%8s) %p", type, lua_tothread( L, i ) );
			else if ( t == LUA_TLIGHTUSERDATA || t == LUA_TUSERDATA )
				fprintf( stderr, "(%8s) %p", type, lua_touserdata( L, i ) );
			else if ( t == LUA_TNIL ||  t == LUA_TNONE )
				fprintf( stderr, "(%8s) %p", type, lua_topointer( L, i ) );
			else if ( t == LUA_TTABLE ) {
			#if 1
				fprintf( stderr, "(%8s) %p\n", type, lua_topointer( L, i ) );
				(*sd) ++, (*pos) += 2;
				lua_dumptable( L, pos, sd );
				(*sd) --, (*pos) -= 2;
			#else
				fprintf( stderr, "(%8s) %p {\n", type, lua_topointer( L, i ) );
				int diff = lua_gettop( L ) - *pos;

				(*sd) ++, (*pos) += diff;
				lua_dumptable( L, pos, sd );
				(*sd) --, (*pos) -= diff;
			#endif
				PRETTY_TABS( *sd );
				fprintf( stderr, "}" );
			}

			fprintf( stderr, "%s", ( i == -2 ) ? " -> " : "\n" );
			//PRETTY_TABS( *sd );
		}
		getchar();
		FPRINTF( "Popping key\n" );
		lua_pop( L, 1 );
	}
	return;
}


void lua_stackdump ( lua_State *L ) {
	//No top
	if ( lua_gettop( L ) == 0 )
		return;

	//Loop again, but show the value of each key on the stack
	for ( int pos = 1; pos <= lua_gettop( L ); pos++ ) {
		int t = lua_type( L, pos );
		const char *type = lua_typename( L, t );
		fprintf( stderr, "[%3d] => ", pos );

		if ( t == LUA_TSTRING )
			fprintf( stderr, "(%8s) %s", type, lua_tostring( L, pos ));
		else if ( t == LUA_TFUNCTION )
			fprintf( stderr, "(%8s) %p", type, (void *)lua_tocfunction( L, pos ) );
		else if ( t == LUA_TNUMBER )
			fprintf( stderr, "(%8s) %lld", type, (long long)lua_tointeger( L, pos ));
		else if ( t == LUA_TBOOLEAN)
			fprintf( stderr, "(%8s) %s", type, lua_toboolean( L, pos ) ? "true" : "false" );
		else if ( t == LUA_TTHREAD )
			fprintf( stderr, "(%8s) %p", type, lua_tothread( L, pos ) );
		else if ( t == LUA_TLIGHTUSERDATA || t == LUA_TUSERDATA )
			fprintf( stderr, "(%8s) %p", type, lua_touserdata( L, pos ) );
		else if ( t == LUA_TNIL ||  t == LUA_TNONE )
			fprintf( stderr, "(%8s) %p", type, lua_topointer( L, pos ) );
		else if ( t == LUA_TTABLE ) {
		#if 0
			fprintf( stderr, "(%8s) %p", type, lua_topointer( L, pos ) );
		#else
			fprintf( stderr, "(%8s) %p {\n", type, lua_topointer( L, pos ) );
			int sd = 1;
			lua_dumptable( L, &pos, &sd );
			fprintf( stderr, "}" );
		#endif
		}	
		fprintf( stderr, "\n" );
	}
	return;
}

static struct mvcmeta_t { 
	const char *dir; 
	const char *reserved; 
	const char *ext;
} mvcmeta [] = {
	{ "app", "model", "lua" }
,	{ "sql", "query", "sql" }
,	{ "views", "views", "tpl" }
};


struct route_t { 
	int iroute_tlen;
	struct iroute_t { char *route; int index; } **iroute_tlist;
	zTable *src;
};


struct mvc_t {
	struct mvcmeta_t *mset;
	int flen, type;
	struct imvc_t {
		const char file[ 2048 ];
		//leave some space here...
		//const char *dir;
	} **imvc_tlist;
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
	lua_pushnil( L );
	//static int sd;
	//FPRINTF( "Current stack depth: %d\n", sd++ );

	while ( lua_next( L, index ) != 0 ) {
		int kt, vt;
	#if 0
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
	#endif

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
			//FPRINTF( "Descending because value at %d is table...\n", -1 );
			//lua_loop( L );
			lua_to_table( L, index + 2, t ); 
			lt_ascend( t );
			//sd--;
		}

		//FPRINTF( "popping last two values...\n" );
		if ( vt == LUA_TNUMBER || vt == LUA_TSTRING ) {
			lt_finalize( t );
		}
		lua_pop(L, 1);
	}

	lt_lock( t );
	return 1;
}


int is_reserved( const char *a ) {
	#if 0
	//for ( const char **b = builtins; *b; b++ ) {
	for ( struct mvcmeta_t **b = mvcmeta; *b; b++ ) {
		if ( strcmp( a, *b ) == 0 ) return 1;
	}
	#endif
	for ( int i = 0; i < sizeof( mvcmeta ) / sizeof( struct mvcmeta_t ); i ++ ) {
		if ( strcmp( a, mvcmeta[i].reserved ) == 0 ) return 1;
	}
	return 0;
}



int make_route_list ( zKeyval *kv, int i, void *p ) {
	struct route_t *tt = (struct route_t *)p;
	const int routes_wordlen = 6;
	if ( kv->key.type == LITE_TXT && !is_reserved( kv->key.v.vchar ) ) {
		char key[ 2048 ] = { 0 };
		lt_get_full_key( tt->src, i, (unsigned char *)&key, sizeof( key ) );
		struct iroute_t *ii = malloc( sizeof( struct iroute_t ) );
		ii->index = i, ii->route = zhttp_dupstr( &key[ routes_wordlen ] ), *ii->route = '/';
		add_item( &tt->iroute_tlist, ii, struct iroute_t *, &tt->iroute_tlen );
	}
	return 1;	
}



void free_route_list ( struct iroute_t **list ) {
	for ( struct iroute_t **l = list; *l; l++ ) {
		free( (*l)->route );
		free( *l );
	}
	free( list );
}



int make_mvc_list ( zKeyval *kv, int i, void *p ) {
	struct mvc_t *tt = (struct mvc_t *)p;
	char *key = NULL;

	if ( kv->key.type == LITE_TXT && is_reserved( key = kv->key.v.vchar ) ) {
		if ( !strcmp( key, "model" ) )
			tt->mset = &mvcmeta[ 0 ], tt->type = kv->value.type;
		else if ( !strcmp( key, "query" ) )
			tt->mset = &mvcmeta[ 1 ], tt->type = kv->value.type;
		else if ( !strcmp( key, "view" ) || !strcmp( key, "views" ) ) {
			tt->mset = &mvcmeta[ 2 ], tt->type = kv->value.type;
		}
	}

	if ( tt->mset && kv->value.type == LITE_TXT && memchr( "mvq", *tt->mset->reserved, 3 ) ) {
		struct imvc_t *imvc = malloc( sizeof( struct imvc_t ) );
		memset( imvc, 0, sizeof( struct imvc_t ) );
		snprintf( (char *)imvc->file, sizeof(imvc->file) - 1, "%s/%s.%s", 
			tt->mset->dir, kv->value.v.vchar, tt->mset->ext );
		add_item( &tt->imvc_tlist, imvc, struct imvc_t *, &tt->flen );
	}

	if ( kv->key.type == LITE_TRM || tt->type == LITE_TXT ) {
		tt->mset = NULL;	
	}
	return 1;	
}



void free_mvc_list ( void **list ) {
	for ( void **l = list; *l; l++ ) {
		free( *l );
	} 
	free( list );
}


#if 0
int load_lua_model( lua_State *L, const char *f, char **err, int errlen ) {
	//Open the configuration file
	if ( !lua_exec_file( L, "config.lua", err, errlen ) ) {
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
}
#endif


//The entry point for a Lua application
int lc (struct HTTPBody *req, struct HTTPBody *res ) {

	//Define variables and error positions...
	zTable zc, zm; 
	zTable *zconfig = &zc, *zmodel = &zm, *zroutes = NULL, *croute = NULL;
	char err[2048] = {0};
	const char *db, *fqdn, *title, *spath, *root;
	lua_State *L = NULL;
	int clen = 0;
	unsigned char *content = NULL;

	//Initialize
	if ( !( L = luaL_newstate() ) ) {
		return http_error( res, 500, "%s", "Failed to initialize Lua environment." );
	}

#if 0
	if ( !load_lua_model( L, "config.lua", zconfig, err, sizeof( err ) ) ) {
		return http_error( res, 500, "%s\n", err );
	}
#else
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
#endif

#if 1
	//EVERYTHING HERE REALLY SHOULD GO IN A PLACE OF ITS OWN...
	//Get the rotues from the config file.
	if ( !( zroutes = lt_copy_by_key( zconfig, "routes" ) ) ) {
		return http_error( res, 500, "%s", "Failed to copy routes from config." );
	}

	//Turn the routes into a list of strings, and search for a match
	struct route_t p = { .src = zroutes };
	lt_exec_complex( zroutes, 1, zroutes->count, &p, make_route_list );
	
	//Loop through the routes
	struct mvc_t pp = {0};
	for ( struct iroute_t **lroutes = p.iroute_tlist; *lroutes; lroutes++ ) {
		//fprintf( stderr, "Checking %s against %s\n", req->path, (*lroutes)->route );
		if ( route_resolve( req->path, (*lroutes)->route ) ) {
			croute = lt_copy_by_index( zroutes, (*lroutes)->index );
			lt_exec_complex( croute, 1, croute->count, &pp, make_mvc_list );
			break;
		}
	}

	//Die when unavailable...
	if ( !croute ) {
		MDETH( "no route found" );
		return http_error( res, 404, "Couldn't find path at %s\n", req->path );
	}

	//Destroy anything having to do with routes 
	lt_free( croute );
	free( croute );
	free_route_list( p.iroute_tlist );	
	lt_free( zroutes );
	free( zroutes );
#endif

	//Load all models
	luaL_openlibs( L );
	for ( struct imvc_t **m = pp.imvc_tlist; *m; m++ ) {
		const char *f = (*m)->file;
		if ( *f == 'a' ) {
			fprintf( stderr, "model: %s\n", f );
			char err[ 2048 ] = { 0 };

			//The executed environment needs to be sent to whatever (it's the context)
			//this can be done via a GIANT table (if there are like a bajillion models)
			//it could also be done by putting values back on the stack...
			//most likely though, each would need to be global or in another context...

			//Open the file that will execute the model
			if ( !lua_exec_file( L, f, err, sizeof( err ) ) ) {
				return http_error( res, 500, "%s", err );
			}
	
			//When successfully executed (or not), return with 
			//[ `basename f` ] = whatever (could be code, function or something else) 
#if 0
			//If this brings back some other real value, fine.  Does not have to be a table...
			//add the name of whatever was executed as the key, and the value as the value
			if ( !lua_istable( L, 1 ) ) 
				0; //we would add whatever values are on the stack to the current table...
			else {
				if ( !lua_to_table( L, 1, zmodel ) ) {
					return http_error( res, 500, "%s", "Failed to convert Lua to zTable" );
				}
			}

			//Take values off the stack here...
			lua_pop( L, 1 );
#endif
		} 
	}

#if 0	
	//Load all views
	for ( struct imvc_t **v = pp.imvc_tlist; *v; v++ ) {
		const char *f = (*v)->file;
		if ( *f == 'v' ) {
			fprintf( stderr, "view: %s\n", f );
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
	}

	free_mvc_list( (void **)pp.imvc_tlist );

	//Return the finished message if we got this far
	res->clen = clen;
	http_set_status( res, 200 ); 
	http_set_ctype( res, "text/html" );
	http_set_content( res, content, clen ); 
	if ( !http_finalize_response( res, err, sizeof(err) ) ) {
		return http_error( res, 500, err );
	}

	//Destroying everything is kind of tough...
	free( content );
#endif
	lt_free( zconfig );
	lua_close( L );
	return 1;
}

