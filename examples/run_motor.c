/*!
	\file run_motor.c
	\brief Example: Run Single Motor
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
	int ret = 0;
	VMC96_t * vmc96 = NULL;

	ret = vmc96_initialize( &vmc96 );

	if( ret != VMC96_SUCCESS )
		goto error;

	ret = vmc96_motor_run( vmc96, 0, 0 );

	if( ret != VMC96_SUCCESS )
		goto error;

	vmc96_finish( vmc96 );
	return EXIT_SUCCESS;

error:

	/* Display error details */
	fprintf( stderr, "Error: %s (Cod: %d)\n", vmc96_get_error_code_string(ret), ret );
	vmc96_finish( vmc96 );
	return EXIT_FAILURE;
}

/* eof */
