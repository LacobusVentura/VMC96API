/*!
	\file basic_vending.c
	\brief Example: Basic Vending Flow
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
#include <unistd.h>

#include "vmc96api.h"

#define VEND_ERROR       (-2)
#define VEND_TIMEOUT     (-1)
#define VEND_OK          (0)

int vend( VMC96_t * vmc96, unsigned int mrow, unsigned char mcol )
{
	int ret = 0;
	int i = 0;
	int detected = 0;
	int trials = 0;
	VMC96_opto_line_sample_block_t opto_line;

	/* Reset Motor Array */
	ret = vmc96_motor_reset( vmc96 );

	if( ret != VMC96_SUCCESS )
		return VEND_ERROR;

	while(1)
	{
		/* Run Desired Product Motor */
		ret = vmc96_motor_run( vmc96, mrow, mcol );

		if( ret != VMC96_SUCCESS )
			return VEND_ERROR;

		/* Read Opto Line Status */
		ret = vmc96_motor_opto_line_status( vmc96, &opto_line );

		if( ret != VMC96_SUCCESS )
			return VEND_ERROR;

		/* Check Block Samples */
		for( i = 0; i < VMC96_OPTO_LINE_SAMPLES_PER_BLOCK; i++ )
		{
			if( opto_line.sample[i] )
			{
				detected = 1;
				break;
			}
		}

		trials++;

		if( detected )
		{
			ret = VEND_OK;
			break;
		}

		if( trials == 5 )
		{
			ret = VEND_TIMEOUT;
			break;
		}

		/* Waitin for the next optical line sample */
		usleep( VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_MS * 1000L );
	}

	/* Stop All Motors */
	vmc96_motor_stop_all( vmc96 );

	/* Return Status */
	return ret;
}


int main( int argc, char ** argv )
{
	int ret = 0;
	VMC96_t * vmc96 = NULL;

	ret = vmc96_initialize( &vmc96 );

	if( ret != VMC96_SUCCESS )
	{
		fprintf( stderr, "Error: %s (Cod: %d)\n", vmc96_get_error_code_string(ret), ret );
		return EXIT_FAILURE;
	};

	ret = vend( vmc96, 0, 0 );

	if( ret != VEND_OK )
	{
		if( ret == VEND_TIMEOUT )
		{
			fprintf( stderr, "Vend Timeout!\n");
		}
		else
		{
			fprintf( stderr, "Vend Error!\n");
		}

		vmc96_finish( vmc96 );
		return EXIT_SUCCESS;
	}

	fprintf( stderr, "Vend OK!\n");

	vmc96_finish( vmc96 );
	return EXIT_SUCCESS;
}

/* eof */
