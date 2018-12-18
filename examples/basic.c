#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmc96.h"

int main(void)
{
	int i = 0;
	int ret = 0;
	VMC96_t * vmc96 = NULL;
	char buf[32] = {0};
	//unsigned char len;
	unsigned int  status = 0;
	
	ret = vmc96_initialize( &vmc96 );
	
	if( ret != VMC96_SUCCESS )
	{
		fprintf( stderr, "[FATAL] Error initializing VMC96 Device: (%d) %s\n", ret, vmc96_get_error_code_string(ret) );
		return EXIT_FAILURE;
	}

	ret = vmc96_motor_reset( vmc96 );

	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	ret = vmc96_motor_ping( vmc96 );

	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	ret = vmc96_motor_get_version( vmc96, buf );

	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	fprintf( stderr, "Motor Array Version: %s\n", buf );

	ret = vmc96_motor_pulse( vmc96, 0x11, 200 );
	
	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	ret = vmc96_motor_pulse( vmc96, 0x12, 200 );
	
	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	ret = vmc96_motor_stop_all( vmc96 );

	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	ret = vmc96_motor_opto_line_status( vmc96, &status );

	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );
		
	ret = vmc96_motor_run( vmc96, 0x0 );

	if( ret != VMC96_SUCCESS )
		fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );

	for( i = 128; i < 255; i++ )
	{
		ret = vmc96_motor_run( vmc96, i );

		if( ret != VMC96_SUCCESS )
			fprintf( stderr, "[ERROR] %d - %s\n", ret, vmc96_get_error_code_string(ret) );
			
		usleep( 500000L );
	}

	vmc96_finish( vmc96 );
	 	
    return EXIT_SUCCESS;
}

/* eof */
