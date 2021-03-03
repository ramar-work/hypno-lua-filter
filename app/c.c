//First web app built in C
#include <zhttp.h>
#include <zmime.h>
#include <sys/stat.h>
#include <router.h>
#include <megadeth.h>
#include "c.h"
#include "../config.h"

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




const char ** get_list ( const char *views ) {
	zWalker w = {0};
	const char **viewlist = NULL;
	int vlen = 0;

	for ( const char *vv = views; strwalk( &w, views, ", " ); vv += w.size )	{
		if ( w.size > 1 ) {
			void *p = malloc( w.size + 1 );
			memset( p, 0, w.size + 1 );
			memcpy( p, vv, ( w.chr == ',' ) ? w.size - 1 : w.size );	
			add_item( &viewlist, p, char *, &vlen ); 
		}
	}
	return viewlist;
}


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
int c_handler ( struct HTTPBody *req, struct HTTPBody *res ) {

	//Define stuff
	char err[ 2048 ] = { 0 };

	//Clean up host name
	cut_host( &req->host );

	//Config

	//Check if the requested resource is static
	for ( const char **path = static_paths; *path; path++ ) {
		if ( check_static( req->path, *path ) ) {
			int len = 0;
			char *p = req->path, err[2048] = {0};
			unsigned char *file = NULL;
			const struct mime_t *mime;

			//Find the file in this directory
			if ( !( file = read_file( ++p, &len, err, sizeof( err ) ) ) ) {
				return http_error( res, 404, err );
			}
		
			//Find it's mimetype
			if ( !( mime = zmime_get_by_filename( p ) ) ) {
				//TODO: This should be user selectable...
				mime = zmime_get_default();
			}

			//This is the final status
			http_set_status( res, 200 ); 
			http_set_ctype( res, mime->mimetype );
			http_copy_content( res, file, len ); 
			if ( !http_finalize_response( res, err, sizeof(err) ) ) {
				return http_error( res, 500, err );
			}
			free( file );
			return 1;		
		}	
	}
		
	//Then check the rest of the routes
	struct route_t *r = NULL; 
	for ( struct route_t *rlist = routes; rlist->name; rlist ++ ) {
		if ( route_resolve( req->path, rlist->name ) ) {
			//Define
			const char **views = NULL;
			int status = 0, clen = 0;
			unsigned char *content = NULL;
			r = rlist;
			zTable *model = NULL; 
	
			//Initialize this table	
			if ( !( model = malloc( sizeof( zTable ) ) ) || !lt_init( model, NULL, 4096 ) ) {
				return http_error( res, 500, "Failed to initialize model environment\n" );
			}
			
			//Run the model(s)
			for ( ;; ) {
				if ( !( status = rlist->exec( req, res, &model ) ) ) {
					return http_error( res, 500, "Failed to execute endpoint at %s\n", rlist->name );
				}
				break;
			}

			//...and any views
			views = get_list( rlist->views );
			for ( const char **v = views; *v; v++ ) { 
				//Define junk
				unsigned char *src, *render;
				int len =0, renlen = 0;
			
			#if 0
			#else
				//This should be one line...
				char view[ 1024 ] = { 0 };
				snprintf( view, sizeof( view ), "%s/%s.%s", "/home/ramar/prj/www/app-template/views", *v, "tpl" );	
				free( (void *)*v );
				fprintf( stderr, "Attempting to render %s\n", view );
			#endif

				zRender * rz = zrender_init();
				//CONFIG: SET RENDER DIALECT
				zrender_set_default_dialect( rz );
				zrender_set_fetchdata( rz, model );
				if ( !( src = read_file( view, &len, err, sizeof( err ) )	) || !len ) {
					return http_error( res, 500, "Problem with view '%s': %s", view, err );
				}
				if ( !( render = zrender_render( rz, src, strlen((char *)src), &renlen ) ) ) {
					return http_error( res, 500, "%s", "Renderer error." );
				}
				zhttp_append_to_uint8t( &content, &clen, render, renlen ); 
				zrender_free( rz );
				free( render );
				free( src );
			}
	
			//Destroy everything	
			free( views );	
			lt_free( model );
			free( model );

			http_set_status( res, 200 ); 
			http_set_ctype( res, "text/html" );
			http_copy_content( res, content, clen ); 
			if ( !http_finalize_response( res, err, sizeof(err) ) ) {
				return http_error( res, 500, err );
			}
			free( content );
			return 1;	
		}
	} 

	if ( !r ) {
		return http_error( res, 404, "Can't find page at %s\n", req->path );
	}

	//Return true because it worked
	return 1;

}
