#include "util.h"

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


