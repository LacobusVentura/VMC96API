/*!
	\file opto_line_sensor.c
	\brief Example: Read Status from Opto Line Sensor
	\author Tiago Ventura (tiago.ventura@gmail.com)
	\date Dec.2018

	Copyright (c) 2018 Tiago Ventura (tiago.ventura@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/


#include <stdio.h>
#include <stdlib.h>

#include "vmc96api.h"

int main( int argc, char ** argv )
{
	int i = 0;
	int ret = 0;
	VMC96_t * vmc96 = NULL;
	VMC96_opto_line_sample_block_t block;

	ret = vmc96_initialize( &vmc96 );

	if( ret != VMC96_SUCCESS )
		goto error;

	ret = vmc96_motor_opto_line_status( vmc96, &block );

	if( ret != VMC96_SUCCESS )
		goto error;

	fprintf( stdout, "OPTO LINE SENSOR STATUS:\n\n");
	fprintf( stdout, "	Samples per block: %d\n", VMC96_OPTO_LINE_SAMPLES_PER_BLOCK );
	fprintf( stdout, "	Total Samples: %d\n", VMC96_OPTO_LINE_SAMPLES_PER_BLOCK  );
	fprintf( stdout, "	Time per Sample: %dms\n", VMC96_OPTO_LINE_SAMPLE_LENGTH_MS );
	fprintf( stdout, "	Time per Block: %.02fs\n", VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_MS / 1000.0 );
	fprintf( stdout, "	Total time: %.02fs\n\n", VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_MS / 1000.0 );

	fprintf( stdout, "	Status:\n");

	fprintf( stdout, "		");

	for( i = 0; i < VMC96_OPTO_LINE_SAMPLES_PER_BLOCK; i++ )
	{
		if( (i > 0) && (i % 8 == 0) )
			fprintf( stdout, "." );

		fprintf( stdout, "%d",  ( block.sample[i] ) ? 1 : 0  );
	}

	fprintf( stdout, "\n\n" );

	fprintf( stdout, "	Signal (%.02fs period):\n", VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_MS / 1000.0 );

	fprintf( stdout, "		");

	for( i = 0; i < VMC96_OPTO_LINE_SAMPLES_PER_BLOCK; i++ )
		fprintf( stdout, "%c",  ( block.sample[i] ) ? '-' : '_'  );

	fprintf( stdout, "\n\n" );

	vmc96_finish( vmc96 );
	return EXIT_SUCCESS;

error:

	/* Display error details */
	fprintf( stderr, "Error: %s (Cod: %d)\n", vmc96_get_error_code_string(ret), ret );
	vmc96_finish( vmc96 );
	return EXIT_FAILURE;
}

/* eof */
