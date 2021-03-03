//System dependencies first
#include <router.h>
//This should be the only header included...
#include <megadeth.h>


//App-level dependencies second.
#include "app/home.h"
#include "app/form.h"
#include "app/books.h"


//Routes 
struct route_t routes[] = {
	// Create a new thing
	{ "/home", home, "intro, home, outro" }
, { "/menu", books, "intro, menu, outro" }
, { "/form", form }
, { "/catering", home }
, { "/blog", home }
, { NULL }
};


//
const char *static_paths[] = { 
	"/assets"
, "/robots.txt"
, "/sitemap.xml" 
, "/favicon.ico" 
, NULL
};


