#include "megadeth.h"


//HTTP error
int http_error( struct HTTPBody *res, int status, char *fmt, ... ) { 
	va_list ap;
	char err[ 2048 ];
	memset( err, 0, sizeof( err ) );
	va_start( ap, fmt );
	vsnprintf( err, sizeof( err ), fmt, ap );
	va_end( ap );
	http_set_status( res, status ); 
	http_set_ctype( res, "text/html" );
	http_copy_content( res, err, strlen( err ) );
	return 0;
}



//TODO: None of these should take an error buffer.  They are just utilities...
unsigned char *read_file ( const char *filename, int *len, char *err, int errlen ) {
	//Check for and load whatever file
	int fd, fstat, bytesRead, fileSize;
	unsigned char *buf = NULL;
	struct stat sb;
	memset( &sb, 0, sizeof( struct stat ) );

	//Check for the file 
	if ( (fstat = stat( filename, &sb )) == -1 ) {
		snprintf( err, errlen, "FILE STAT ERROR: %s\n", strerror( errno ) );
		return NULL;	
	}

	//Check for the file 
	if ( (fd = open( filename, O_RDONLY )) == -1 ) {
		snprintf( err, errlen, "FILE OPEN ERROR: %s\n", strerror( errno ) );
		return NULL;	
	}

	//Allocate a buffer
	fileSize = sb.st_size + 1;
	if ( !(buf = malloc( fileSize )) || !memset(buf, 0, fileSize) ) {
		snprintf( err, errlen, "COULD NOT OPEN VIEW FILE: %s\n", strerror( errno ) );
		close( fd );
		return NULL;	
	}

	//Read the entire file into memory, b/c we'll probably have space 
	if ( (bytesRead = read( fd, buf, sb.st_size )) == -1 ) {
		snprintf( err, errlen, "COULD NOT READ ALL OF VIEW FILE: %s\n", strerror( errno ) );
		free( buf );
		close( fd );
		return NULL;	
	}

	//This should have happened before...
	if ( close( fd ) == -1 ) {
		snprintf( err, errlen, "COULD NOT CLOSE FILE %s: %s\n", filename, strerror( errno ) );
		free( buf );
		return NULL;	
	}

	*len = sb.st_size;
	return buf;
}



//Utility to add to a series of items
void * add_item_to_list( void ***list, void *element, int size, int * len ) {
	//Reallocate
	if (( (*list) = realloc( (*list), size * ( (*len) + 2 ) )) == NULL ) {
		return NULL;
	}

	(*list)[ *len ] = element; 
	(*list)[ (*len) + 1 ] = NULL; 
	(*len) += 1; 
	return list;
}
