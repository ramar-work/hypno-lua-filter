#include <zhttp.h>
#include <ztable.h>

#ifndef ROUTE_H
#define ROUTE_H

//Define a route
struct route_t {
	const char *name;
	int (*exec)( struct HTTPBody *, struct HTTPBody *, zTable ** );
	const char *views;
};

#endif
