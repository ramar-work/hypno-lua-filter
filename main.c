//First web app built in C
#include <zhttp.h>
#include <sys/stat.h>

//Include the app files
#include "app/home.h"

//Define a route
typedef struct Route {

	const char *name;
	
	const char *under;

	int (*exec)( struct HTTPBody *, struct HTTPBody * );
	
} Route;



//Leave only the real hostname...
int cuthost( char **h ) {
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
int direxists( const char *path ) {
	struct stat sb = {0};	
	return ( stat( path, &sb ) == -1 ); 
}



//This is a set of routes
int mvc( struct HTTPBody *req, struct HTTPBody *res ) {
	//Serve models asked for (which should grab and exec SQL)

	//And then serve views asked for
	return 1;
}



//Routes 
Route routes[] = {
	// Create a new thing
	{ "/home", NULL, home }
, { "/menu", NULL, home }
, { "/form", NULL, home }
, { "/catering", NULL, home }
, { "/blog", NULL, home }
, { "/robots.txt", NULL, home }
, { "/sitemap.xml", NULL, home }
, { "/assets", "*", home }
};



//int app ( int fd, struct HTTPBody *req, struct HTTPBody *res, struct cdata *conn ) {
int app ( struct HTTPBody *req, struct HTTPBody *res ) {

	//Clean up host name
	cuthost( &req->host );

	//Debugging stuff
	//fprintf( stderr, "** HTTPBODY is below **\n\n" );
	//print_httpbody( req );

	//Check the domain name
	if ( !direxists( req->host ) ) {
		const char error[] = "Directory does not exist.";
		http_set_status( res, 404 ); 
		http_set_ctype( res, "text/html" );
		http_copy_content( res, error, strlen( error ) );
		return 0;
	}

#if 1
	//Serve home
	if ( !home( req, res ) ) {
		return http_set_error( res, 500, "Failed to execute app." );
	}
#else
	//Define a message
	char * message = "<h2>Hello, World!</h2>";
	http_set_status( res, 200 ); 
	http_set_ctype( res, "text/html" );
	http_copy_content( res, message, strlen( message ) );
#endif

	//Return true because it worked
	return 1;

}
