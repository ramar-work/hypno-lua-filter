//First web app built in C
#include <zhttp.h>
#include <sys/stat.h>
#include <router.h>
#include <megadeth.h>


//Include the app files
//#include "app/home.h"
//#include "app/form.h"
#include "app/lua.h"


//Leave only the real hostname...
int cut_host( char **h ) {
	char *host = *h;
	while ( *host ) {
		if ( *host == ':' ) { 
			memset( host, 0, strlen( host ) );
			return 1;
		}
		host++;
	}
	return 1;
}



//Check if the directory exists (and eventually if other stuff exists)
int dir_exists( const char *path ) {
	struct stat sb = {0};	
	return ( stat( path, &sb ) == -1 ); 
}



//This is a set of routes
int mvc( struct HTTPBody *req, struct HTTPBody *res ) {
	//Serve models asked for (which should grab and exec SQL)

	//And then serve views asked for
	return 1;
}


#if 0
//Define a route
typedef struct Route {
	const char *name;
	int (*exec)( struct HTTPBody *, struct HTTPBody * );
} Route;



//Routes 
Route routes[] = {
	// Create a new thing
	{ "/home", home }
//, { "/form", form }
, { "/catering", home }
, { "/blog", home }
, { "/robots.txt", home }
, { "/sitemap.xml", home }
, { "/assets", home }
, { NULL }
};



//
const char *static_paths[] = { 
	"/assets"
, "/robots.txt"
, "/sitemap.xml" 
, "/favicon.ico" 
};
#endif



//
int check_static ( const char *recvdpath, const char *checkpath ) {
	//are they the same at all
	if ( strlen( recvdpath ) < strlen( checkpath ) ) {
		return 0;
	}

	//check if one contains the other
	return memcmp( recvdpath, checkpath, strlen( checkpath ) ) == 0; 
}



//....
int app ( struct HTTPBody *req, struct HTTPBody *res ) {

	//Define stuff
	int status = 0;
	char err[ 2048 ] = { 0 };

	//Clean up host name
	cut_host( &req->host );

	//Check the domain name
	if ( !dir_exists( req->host ) ) {
		return http_error( res, 404, "%s", "Directory does not exist." );
	}

	//This may need to accept an error 
	if ( !( status = lc( req, res ) ) ) {
		return 0; 
	}

#if 0
	//Check if the requested resource is static
	for ( const char **path = static_paths; *path; path++ ) {
		if ( check_static( req->path, *path ) ) {
			return http_set_error( res, 200, "Would have served a static page." );	
		}	
	}
		
	//Then check the rest of the routes
	Route *r = NULL; 
	for ( Route *rlist = routes; rlist->name; rlist ++ ) {
		if ( route_resolve( req->path, rlist->name ) ) {
			if ( !( status = rlist->exec( req, res ) ) ) {
				return http_error( res, 500, "Failed to execute endpoint at %s\n", rlist->name );
			}
		}
	} 
#endif

	//Handle default requests...
	if ( !status ) {
		return http_error( res, 200, "%s", "<h2>Hello, World!</h2>" );
	}

	//Return true because it worked
	return 1;

}
