#include "home.h"

int home( struct HTTPBody *req, struct HTTPBody *res ) {

	//Declare stuff
	char *render, *query, err[ 2048 ];
	memset( err, 0, sizeof( err ) );
	void *p = NULL;
	zTable *t = NULL;	
	int len = 0;

	//Load the SQL file
	if ( !( query = (char *)read_file( "sql/home.sql", &len, err, sizeof(err) ) ) ) {
		return http_set_error( res, 500, "SQL load failed" );
	}

	//Open the database
	if ( !( p = db_open( "sites/clutchclt/wall.db", err, sizeof( err ) - 1 ) ) ) {
		//return an error
		return http_set_error( res, 500, err );
	}

	//Execute the SQL file
	if ( !( t = db_exec( p, query, NULL, err, sizeof( err ) - 1 ) ) ) {
		return http_set_error( res, 500, err );
	}  

	//Close
	if ( !db_close( &p, err, sizeof( err ) ) ) {
		return http_set_error( res, 500, err );
	}

	//Dump first, then render
	lt_dump( t );
	int renlen = 0;
	zRender * rz = zrender_init();
	zrender_set_default_dialect( rz );
	zrender_set_fetchdata( rz, t );
	unsigned char *src = read_file( "views/home.tpl", &len, err, sizeof( err ) );	
	render = (char *)zrender_render( rz, src, strlen((char *)src), &renlen );
	write( 2, render, renlen );
	zrender_free( rz );

	//You can choose to return something here or filter
	char *message = "bob";
	http_set_status( res, 200 ); 
	http_set_ctype( res, "text/html" );
	http_copy_content( res, message, strlen( message ) );
	
	return 1;
}


