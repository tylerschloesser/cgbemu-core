#include "tools.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

unsigned int last_error = 0;

const int ERROR_FILE_NOT_FOUND = 1;
const int ERROR_MAX_SIZE_EXCEEDED = 2;
const int ERROR_FILE_NOT_READ = 3;

bool debug_mode = true;

/*
 * input: size in bytes
 * output: string with size in B, KB, or MB
 */
char size_as_string[32];
char *size_to_string(int size) 
{
	char  B[] = " B";
	char KB[] = " KB";
	char MB[] = " MB";
	char* unit = B;
	static const int limit = 1024 * 10;
	if(size > limit) {
		size /= 1024;
		if(size > limit) {
			size /= 1024;
			unit = MB;
		} else {
			unit = KB;
		}		
	}
	
	sprintf( size_as_string, "%i", size );
	strncat(size_as_string, unit, 32 - strlen(size_as_string));
	return size_as_string;
}

void fatal_error() 
{
	fprintf(stderr, "cgbemu encountered a fatal error and must terminate\n");
	fprintf(stderr, "press enter to finish\n");
	getchar();
	exit(last_error);
}

/* input:
 *	filepath: the file to read
 *	dest: a pointer to a u8 array
 *	max_size: the max number of bytes to read
 * output: The total number of bytes read	
 */
unsigned int binary_read_file( char* filepath, u8* dest, int max_size ) {
	
	/* just temporary... */
	printf( "attempting to read: %s\n", filepath );

	FILE *file = fopen(filepath, "rb");
	if(!file) {
        perror("fopen() failed");
		return 0;
	}

	fseek(file, 0, SEEK_END);
	int	size = ftell(file);
	rewind(file);
    
    fprintf(stderr, "size=%i\n", size);
	
	/* do not continue if the size exceeds the maximum size */
    /*
	if(size > max_size) {
		fprintf( stderr, "binary_read_file(): size (%i) is greater than maxsize (%i)\n", size, max_size );
		fclose(file);
		return 0;
	}
    */
	
	if(fread(dest, sizeof(*dest), size, file) == 0) {
        perror("fread() failed");
		fclose(file);
		return 0;
	}
	
	fclose(file);
	return size;
	
}

unsigned long get_elapsed_usec( struct timeval time_start, struct timeval time_end )
{
	double elapsed = 0.0;
	if( time_end.tv_sec > time_start.tv_sec ) {
		elapsed += ( (double)( time_end.tv_sec - time_start.tv_sec - 1 ) ) * 1000000.0;
		elapsed += time_end.tv_usec + ( 1000000 - time_start.tv_usec );
	} else {
		elapsed = time_end.tv_usec - time_start.tv_usec;
	}
	return( (unsigned long)elapsed );
}

/*
 * Returns time difference between "timee" and "times" in micro-seconds (usec)
 
unsigned long comp_time (struct timeval times, struct timeval timee) {

    double elap = 0.0;

    if (timee.tv_sec > times.tv_sec) {
        elap += (((double)(timee.tv_sec - times.tv_sec -1))*1000000.0);
        elap += timee.tv_usec + (1000000-times.tv_usec);
    }
    else {
        elap = timee.tv_usec - times.tv_usec;
    }
    
    return ((unsigned long)(elap));
}
*/
