/* ------------------------------------------------------ *
 * router-test.c
 * =============
 * 
 * Test that simple route resolution works.
 * 
 * You can pass in a custom receiver function if the datatype
 * is not something simple like a list of strings.
 *
 * ------------------------------------------------------ */
//Compile me with: 
//gcc -ldl -lpthread -o router vendor/single.o vendor/sqlite3.o router.c && ./router
#include <zwalker.h>
#include <zhasher.h>
#include <router.h>


//Simple list of strings
const char *routes_basic[] = {
	"/",
	"//",
	"/route",
	"/route/",
	"/route",
	"/route/:id",
	"/route/:id=string",
	"/route/:id=number",
	"/:id",
	"/route/*",
	"/route/*/jackpot",
	"/{route,julius}",
	"/{route,julius}/:id",
#if 0
//Do I want to handle '?'
, "/route/?"
, "/route/?accharat"
, "/route/??"
, "/route/???"
//Do I want to handle regular expressions?
//, "/route/[^a-z]"
#endif
	NULL
};



//Simple list of slightly complex datatypes
struct useless {
	const char *name;
	int useless_int;
	void * useless_void;
} routes_advanced [] = {
	{ "/" },
	{ "//" },
	{ "/route" },
	{ "/route/" },
	{ "/route" },
	{ "/route/:id" },
	{ "/route/:id=string" },
	{ "/route/:id=number" },
	{ "/:id" },
	{ "/route/*" },
	{ "/route/*/jackpot" },
	{ "/{route,julius}" },
	{ "/{route,julius}/:id" },
#if 0
//Do I want to handle '?'
, "/route/?"
, "/route/?accharat"
, "/route/??"
, "/route/???"
//Do I want to handle regular expressions?
//, "/route/[^a-z]"
#endif
	{ NULL }
};



const char * get_ustring ( void *u ) {
	fprintf( stderr, "%p\n", u );
	struct useless * uu = u;
	return uu->name;
}



//These are possible request lines
const char *requests[] = {
  "/"
, "/2"
, "/route"
, "/route/3"
, "/route/bashful"
, "/route/bashful/jackpot"
, "/route/333/jackpot"
, "/julius/3"
, "/unresolvable"
, "/joseph/route/337"
, "/route/337a"
#if 0
, "/route/3"
#endif
, NULL
};


int main (int argc, char *argv[]) {

	//Start simple string tests
	fprintf( stderr, "ROUTING MATCHES VIA STRING\n" );
	fprintf( stderr, "==========================\n" );

	//Move through above list and test out
	for ( const char **uri = requests; *uri; uri++ ) {
		const char *resv = NULL;
		int routecount = sizeof( routes_basic ) / sizeof( char * );
		fprintf( stderr,  "Checking %d routes against this URI: %s\n", routecount, *uri );
	
		if ( ( resv = route_resolve_stringlist( *uri, routes_basic ) ) )
			fprintf( stderr, "SUCC: Path %s resolved to name: %s\n", *uri, resv );
		else {
			fprintf( stderr, "FAIL: Path %s did not resolve.\n", *uri );		
			continue;
		}
	}


	//Start complex datatype tests
	fprintf( stderr, "\nROUTING MATCHES VIA COMPLEX TYPES\n" );
	fprintf( stderr, "=================================\n" );

	//The long (and slow) way to do it
	for ( const char **uri = requests; *uri; uri++ ) {
		struct useless *resv = NULL;
		int routecount = sizeof( routes_advanced ) / sizeof( struct useless );
		fprintf( stderr,  "Checking %d routes against this URI: %s\n", routecount, *uri );

		for ( struct useless *uu = routes_advanced; uu->name ; uu++ ) {
			if ( ( route_resolve( *uri, uu->name ) ) ) {
				resv = uu;
				break;
			}
		}
		if ( resv )
			fprintf( stderr, "SUCC: Path %s resolved to name: %s\n", *uri, resv->name );
		else {
			fprintf( stderr, "FAIL: Path %s did not resolve.\n", *uri );		
			continue;
		}
	}

#if 0
	//This is what I would actually like...
	for ( const char **uri = requests; *uri; uri++ ) {
	//Move through a list of "complex" datatypes
		struct useless *resv = NULL;
		if ( ( resv = route_resolve_set( *uri, ra, get_ustring ) ) )
			fprintf( stderr, "SUCCESS: %p\n", resv );
			//fprintf( stderr, "SUCC: Path %s resolved to name: %s\n", *uri, resv->name );
		else {
			fprintf( stderr, "FAIL: Path %s did not resolve.\n", *uri );		
			continue;
		}
	}
#endif

	return 0;
}
