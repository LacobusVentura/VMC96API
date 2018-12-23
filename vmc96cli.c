/*!
	\file vmc96cli.c
	\brief VMC96 Board Vending Machine Controller Command Line Interface
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
#include <string.h>
#include <getopt.h>

#include "vmc96api.h"


/* ********************************************************************* */
/* *                              DEFINES                              * */
/* ********************************************************************* */

#define VMC96CLI_CONTROLLER_GLOBAL                        (1)
#define VMC96CLI_CONTROLLER_RELAY1                        (2)
#define VMC96CLI_CONTROLLER_RELAY2                        (3)
#define VMC96CLI_CONTROLLER_MOTOR_ARRAY                   (4)
#define VMC96CLI_CONTROLLER_INVALID                       (-1)
#define VMC96CLI_CONTROLLER_NOT_SPECIFIED                 (-2)

#define VMC96CLI_COMMAND_RESET                            (1)
#define VMC96CLI_COMMAND_PING                             (2)
#define VMC96CLI_COMMAND_VERSION                          (3)
#define VMC96CLI_COMMAND_RELAY_CONTROL                    (4)
#define VMC96CLI_COMMAND_OPTO_LINE_STATUS                 (5)
#define VMC96CLI_COMMAND_MOTOR_RUN                        (6)
#define VMC96CLI_COMMAND_MOTOR_RUN_PAIR                   (7)
#define VMC96CLI_COMMAND_MOTOR_STOP_ALL                   (8)
#define VMC96CLI_COMMAND_MOTOR_STATUS                     (9)
#define VMC96CLI_COMMAND_ARRAY_SCAN                       (10)
#define VMC96CLI_COMMAND_GIVE_PULSE                       (11)
#define VMC96CLI_COMMAND_INVALID                          (-1)
#define VMC96CLI_COMMAND_NOT_SPECIFIED                    (-2)

#define VMC96CLI_SUCCESS                                  (0)
#define VMC96CLI_ERROR_INVALID_ARGS                       (-1)
#define VMC96CLI_ERROR_COMMAND_FAILED                     (-2)
#define VMC96CLI_ERROR_ARGS_CONTROLLER_INVALID            (-3)
#define VMC96CLI_ERROR_ARGS_CONTROLLER_NOT_SPECIFIED      (-4)
#define VMC96CLI_ERROR_ARGS_COMMAND_INVALID               (-5)
#define VMC96CLI_ERROR_ARGS_COMMAND_NOT_SPECIFIED         (-6)
#define VMC96CLI_ERROR_ARGS_RELAY_STATE                   (-7)
#define VMC96CLI_ERROR_ARGS_MOTOR_ROW                     (-8)
#define VMC96CLI_ERROR_ARGS_MOTOR_COLUMN                  (-9)
#define VMC96CLI_ERROR_ARGS_MOTOR_COLUMN1                 (-10)
#define VMC96CLI_ERROR_ARGS_MOTOR_COLUMN2                 (-11)
#define VMC96CLI_ERROR_ARGS_DURATION                      (-12)

#define VMC96CLI_ARGUMENT_NOT_INITIALIZED                 (-1)


/* ********************************************************************* */
/* *                        STRUCTS AND DATA TYPES                     * */
/* ********************************************************************* */

typedef struct vmc96cli_arguments_s vmc96cli_arguments_t;

struct vmc96cli_arguments_s
{
	int controller;
	int command;
	int duration;
	int state;
	int col;
	int row;
	int col1;
	int col2;
};


/* ********************************************************************* */
/* *                             PROTOTYPES                            * */
/* ********************************************************************* */

static const char * vmc96cli_get_error_code_string( int cod );
static void vmc96cli_show_usage( void );
static int vmc96cli_get_cntrl_code( const char * cntrl );
static int vmc96cli_get_cmd_code( const char * cmd );
static int vmc96cli_execute( VMC96_t * vmc96, vmc96cli_arguments_t * args );
static int vmc96cli_proccess_arguments( int argc, char ** argv, vmc96cli_arguments_t * args );


/* ********************************************************************* */
/* *                          IMPLEMENTATION                           * */
/* ********************************************************************* */

static const char * vmc96cli_get_error_code_string( int cod )
{
	switch(cod)
	{
		case VMC96CLI_SUCCESS                             : return "Success."; break;
		case VMC96CLI_ERROR_INVALID_ARGS                  : return "Invalid arguments."; break;
		case VMC96CLI_ERROR_COMMAND_FAILED                : return "Command failed."; break;
		case VMC96CLI_ERROR_ARGS_CONTROLLER_INVALID       : return "Invalid Controller (--controller)."; break;
		case VMC96CLI_ERROR_ARGS_COMMAND_INVALID          : return "Invalid Command (--command)."; break;
		case VMC96CLI_ERROR_ARGS_CONTROLLER_NOT_SPECIFIED : return "Controller not specified. (--controller)"; break;
		case VMC96CLI_ERROR_ARGS_COMMAND_NOT_SPECIFIED    : return "Command not especified (--command)."; break;
		case VMC96CLI_ERROR_ARGS_RELAY_STATE              : return "Relay state not especified (--state)."; break;
		case VMC96CLI_ERROR_ARGS_MOTOR_ROW                : return "Motor row coordinate not especified (--row)."; break;
		case VMC96CLI_ERROR_ARGS_MOTOR_COLUMN             : return "Motor column coordinate not especified (--column)."; break;
		case VMC96CLI_ERROR_ARGS_MOTOR_COLUMN1            : return "Motor pair first column coordinate not especified (--column1)."; break;
		case VMC96CLI_ERROR_ARGS_MOTOR_COLUMN2            : return "Motor pair second column coordinate not especified (--column2)."; break;
		case VMC96CLI_ERROR_ARGS_DURATION                 : return "Pulse duration not especified (--duration)."; break;
		default                                           : return "Unknown error."; break;
	}
}


static void vmc96cli_show_usage( void )
{
	printf( "GLOBAL RESET:\n\n" );
	printf( "	vmc96cli --controller=GLOBAL --command=RESET\n\n" );
	printf( "GENERAL PURPOSE RELAY - PING:\n\n" );
	printf( "	vmc96cli --controller=[RELAY1|RELAY2] --command=PING\n\n" );
	printf( "GENERAL PURPOSE RELAY - RESET:\n\n" );
	printf( "	vmc96cli --controller=[RELAY1|RELAY2] --command=RESET\n\n" );
	printf( "GENERAL PURPOSE RELAY - Get Version:\n\n" );
	printf( "	vmc96cli --controller=[RELAY1|RELAY2] --command=VERSION\n\n" );
	printf( "GENERAL PURPOSE RELAY - State Control:\n\n" );
	printf( "	vmc96cli --controller=[RELAY1|RELAY2] --command=CONTROL --state=[0|1]\n\n" );
	printf( "MOTOR ARRAY - PING:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=PING\n\n" );
	printf( "MOTOR ARRAY - RESET:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=RESET\n\n" );
	printf( "MOTOR ARRAY - GET VERSION:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=VERSION\n\n" );
	printf( "MOTOR ARRAY - RUN SINGLE MOTOR:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=RUN --row=[0-11] --column=[0-7]\n\n" );
	printf( "MOTOR ARRAY - RUN MOTOR PAIR:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=RUN_PAIR --row=[0-11] --column1=[0-7] --column2=[0-7]\n\n" );
	printf( "MOTOR ARRAY - SCAN ARRAY:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=SCAN\n\n" );
	printf( "MOTOR ARRAY - GIVE PULSE:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=GIVE_PULSE --row=[0-11] --column=[0-7] --duration=[1-255]\n\n" );
	printf( "MOTOR ARRAY - GET STATUS:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=STATUS\n\n" );
	printf( "MOTOR ARRAY - STOP ALL MOTORS:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=STOP_ALL\n\n" );
	printf( "MOTOR ARRAY - GET OPTO-SENSOR STATUS:\n\n" );
	printf( "	vmc96cli --controller=MOTOR_ARRAY --command=OPTO_LINE_STATUS\n\n" );
	printf( "SHOW USAGE:\n\n" );
	printf( "	vmc96cli --help\n\n" );
}


static int vmc96cli_get_cntrl_code( const char * cntrl )
{
	if( !strcasecmp( cntrl, "GLOBAL" ) )
		return VMC96CLI_CONTROLLER_GLOBAL;
	else if( !strcasecmp( cntrl, "RELAY1" ) )
		return VMC96CLI_CONTROLLER_RELAY1;
	else if( !strcasecmp( cntrl, "RELAY2" ) )
		return VMC96CLI_CONTROLLER_RELAY2;
	else if( !strcasecmp( cntrl, "MOTOR_ARRAY" ) )
		return VMC96CLI_CONTROLLER_MOTOR_ARRAY;
	else if( !strcasecmp( cntrl, "" ) )
		return VMC96CLI_CONTROLLER_NOT_SPECIFIED;
	else
		return VMC96CLI_CONTROLLER_INVALID;
}


static int vmc96cli_get_cmd_code( const char * cmd )
{
	if( !strcasecmp( cmd, "RESET" ) )
		return VMC96CLI_COMMAND_RESET;
	else if( !strcasecmp( cmd, "PING" ) )
		return VMC96CLI_COMMAND_PING;
	else if( !strcasecmp( cmd, "VERSION" ) )
		return VMC96CLI_COMMAND_VERSION;
	else if( !strcasecmp( cmd, "CONTROL" ) )
		return VMC96CLI_COMMAND_RELAY_CONTROL;
	else if( !strcasecmp( cmd, "RUN" ) )
		return VMC96CLI_COMMAND_MOTOR_RUN;
	else if( !strcasecmp( cmd, "RUN_PAIR" ) )
		return VMC96CLI_COMMAND_MOTOR_RUN_PAIR;
	else if( !strcasecmp( cmd, "STOP_ALL" ) )
		return VMC96CLI_COMMAND_MOTOR_STOP_ALL;
	else if( !strcasecmp( cmd, "STATUS" ) )
		return VMC96CLI_COMMAND_MOTOR_STATUS;
	else if( !strcasecmp( cmd, "OPTO_LINE_STATUS" ) )
		return VMC96CLI_COMMAND_OPTO_LINE_STATUS;
	else if( !strcasecmp( cmd, "SCAN" ) )
		return VMC96CLI_COMMAND_ARRAY_SCAN;
	else if( !strcasecmp( cmd, "GIVE_PULSE" ) )
		return VMC96CLI_COMMAND_GIVE_PULSE;
	else if( !strcasecmp( cmd, "" ) )
		return VMC96CLI_COMMAND_NOT_SPECIFIED;
	else
		return VMC96CLI_COMMAND_INVALID;
}


static int vmc96cli_execute( VMC96_t * vmc96, vmc96cli_arguments_t * args )
{
	int ret = 0;

	if( args->controller == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
		return VMC96CLI_ERROR_ARGS_CONTROLLER_NOT_SPECIFIED;

	if( args->command == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
		return VMC96CLI_ERROR_ARGS_COMMAND_NOT_SPECIFIED;

	switch( args->controller )
	{
		/* ************************************************************** */
		/* *                     ALL CONTROLLERS                        * */
		/* ************************************************************** */
		case VMC96CLI_CONTROLLER_GLOBAL :
		{
			switch( args->command )
			{
				case VMC96CLI_COMMAND_RESET :
				{
					ret = vmc96_global_reset( vmc96 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				default:
				{
					return VMC96CLI_ERROR_ARGS_COMMAND_INVALID;
				}
			}
		}

		/* ************************************************************** */
		/* *              GENERAL PURPOSE RELAY CONTROLLERS             * */
		/* ************************************************************** */
		case VMC96CLI_CONTROLLER_RELAY1 :
		case VMC96CLI_CONTROLLER_RELAY2 :
		{
			switch(args->command)
			{
				case VMC96CLI_COMMAND_RESET :
				{
					ret = vmc96_relay_reset( vmc96, (args->controller == VMC96CLI_CONTROLLER_RELAY1) ? 0 : 1 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_PING :
				{
					ret = vmc96_relay_ping( vmc96, (args->controller == VMC96CLI_CONTROLLER_RELAY1) ? 0 : 1 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					fprintf( stdout, "PONG!\n" );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_VERSION :
				{
					char version[ VMC96_VERSION_STRING_MAX_LEN + 1 ] = {0};

					ret = vmc96_relay_get_version( vmc96, (args->controller == VMC96CLI_CONTROLLER_RELAY1) ? 0 : 1, version );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;;
					}

					fprintf( stdout, "Version: %s\n", version );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_RELAY_CONTROL :
				{
					if( args->state == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_RELAY_STATE;

					ret = vmc96_relay_control( vmc96, (args->controller == VMC96CLI_CONTROLLER_RELAY1) ? 0 : 1, args->state );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;;
					}

					return VMC96CLI_SUCCESS;
				}

				default:
				{
					return VMC96CLI_ERROR_ARGS_COMMAND_INVALID;
				}
			}
		}


		/* ************************************************************** */
		/* *                    MOTOR ARRAY CONTROLLER                  * */
		/* ************************************************************** */
		case VMC96CLI_CONTROLLER_MOTOR_ARRAY :
		{
			switch( args->command )
			{
				case VMC96CLI_COMMAND_RESET :
				{
					ret = vmc96_motor_reset( vmc96 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_PING :
				{
					ret = vmc96_motor_ping( vmc96 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					fprintf( stdout, "PONG!\n" );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_VERSION :
				{
					char version[ VMC96_VERSION_STRING_MAX_LEN + 1 ] = {0};

					ret = vmc96_relay_get_version( vmc96, (args->controller == VMC96CLI_CONTROLLER_RELAY1) ? 0 : 1, version );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;;
					}

					fprintf( stdout, "Version: %s\n", version );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_MOTOR_RUN :
				{
					if( args->row == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_ROW;

					if( args->col == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_COLUMN;

					ret = vmc96_motor_run( vmc96, args->row, args->col );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_MOTOR_RUN_PAIR :
				{
					if( args->row == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_ROW;

					if( args->col1 == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_COLUMN1;

					if( args->col2 == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_COLUMN2;

					ret = vmc96_motor_pair_run( vmc96, args->row, args->col1, args->col2 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_MOTOR_STOP_ALL :
				{
					ret = vmc96_motor_stop_all( vmc96 );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_MOTOR_STATUS:
				{
					unsigned char row = 0;
					unsigned char col = 0;
					VMC96_motor_array_status_t status;

					ret = vmc96_motor_get_status( vmc96, &status );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					fprintf( stdout, "MOTOR ARRAY STATUS:\n\n");
					fprintf( stdout, "	Active Motors Count: %d\n", status.active_count );
					fprintf( stdout, "	Total Current Drained: %dmA\n\n", status.current_ma );
					fprintf( stdout, "	Array:\n" );

					for( row = 0; row < VMC96_MOTOR_ARRAY_ROWS_COUNT; row++ )
					{
						printf("		");

						for( col = 0; col < VMC96_MOTOR_ARRAY_COLUMNS_COUNT; col++ )
							fprintf( stdout, "%c ", (status.array.motor[row][col]) ? 'M' : '*' );

						printf("\n");
					}

					fprintf( stdout, "\n" );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_OPTO_LINE_STATUS:
				{
					int i = 0;
					VMC96_opto_line_sample_block_t block;

					ret = vmc96_motor_opto_line_status( vmc96, &block );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

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

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_ARRAY_SCAN :
				{
					int ret = 0;
					unsigned char row = 0;
					unsigned char col = 0;
					VMC96_motor_array_scan_result_t result;

					ret = vmc96_motor_scan_array( vmc96, &result );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					fprintf( stdout, "MOTOR ARRAY SCAN RESULTS:\n\n");
					fprintf( stdout, "	Motors Count: %d\n\n", result.count );
					fprintf( stdout, "	Motor Array:\n" );

					for( row = 0; row < VMC96_MOTOR_ARRAY_ROWS_COUNT; row++ )
					{
						printf("		");

						for( col = 0; col < VMC96_MOTOR_ARRAY_COLUMNS_COUNT; col++ )
							fprintf( stdout, "%c ", (result.array.motor[row][col]) ? 'M' : '*' );

						printf("\n");
					}

					fprintf( stdout, "\n" );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_GIVE_PULSE :
				{
					int ret = 0;

					if( args->row == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_ROW;

					if( args->col == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_MOTOR_COLUMN;

					if( args->duration == VMC96CLI_ARGUMENT_NOT_INITIALIZED )
						return VMC96CLI_ERROR_ARGS_DURATION;

					ret = vmc96_motor_give_pulse( vmc96, args->row, args->col, args->duration );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					return VMC96CLI_SUCCESS;
				}

				default:
				{
					return VMC96CLI_ERROR_ARGS_COMMAND_INVALID;
				}
			}
		}

		default:
		{
			return VMC96CLI_ERROR_ARGS_CONTROLLER_INVALID;
		}
	}
}


static int vmc96cli_proccess_arguments( int argc, char ** argv, vmc96cli_arguments_t * args )
{
	int ret = 0;
	int index = 0;

	static struct option options[] =
	{
		{ "controller",  required_argument, 0,  'a' },
		{ "cntlr",       required_argument, 0,  'a' },
		{ "command",     required_argument, 0,  'b' },
		{ "cmd",         required_argument, 0,  'b' },
		{ "state",       required_argument, 0,  'c' },
		{ "duration",    required_argument, 0,  'd' },
		{ "row",         required_argument, 0,  'e' },
		{ "col",         required_argument, 0,  'f' },
		{ "column",      required_argument, 0,  'f' },
		{ "col1",        required_argument, 0,  'g' },
		{ "column1",     required_argument, 0,  'g' },
		{ "col2",        required_argument, 0,  'h' },
		{ "column2",     required_argument, 0,  'h' },
		{ "help",        no_argument,       0,  'i' },
		{ NULL,          no_argument,       0,   0  }
	};

	args->controller = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->command = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->state = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->row = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->col = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->col1 = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->col2 = VMC96CLI_ARGUMENT_NOT_INITIALIZED;
	args->duration = VMC96CLI_ARGUMENT_NOT_INITIALIZED;

	while(1)
	{
		ret = getopt_long( argc, argv, "a:b:c:d:e:f:g:h:i", options, &index );

		if( ret == -1 )
			return VMC96CLI_SUCCESS;

		switch( ret )
		{
			case 'a' : args->controller = vmc96cli_get_cntrl_code( optarg ); break;
			case 'b' : args->command = vmc96cli_get_cmd_code( optarg ); break;
			case 'c' : args->state = atoi( optarg ); break;
			case 'd' : args->duration = atoi( optarg ); break;
			case 'e' : args->row = atoi( optarg ); break;
			case 'f' : args->col = atoi( optarg ); break;
			case 'g' : args->col1 = atoi( optarg ); break;
			case 'h' : args->col2 = atoi( optarg ); break;

			case 'i' :
				vmc96cli_show_usage();
				return VMC96CLI_ERROR_INVALID_ARGS;

			default :
				return VMC96CLI_ERROR_INVALID_ARGS;
		}
	}
}


/* ********************************************************************* */
/* *                                MAIN                               * */
/* ********************************************************************* */
int main( int argc, char ** argv )
{
	int ret = 0;
	vmc96cli_arguments_t args;
	VMC96_t * vmc96 = NULL;

	ret = vmc96cli_proccess_arguments( argc, argv, &args );

	if( ret != VMC96CLI_SUCCESS )
		return EXIT_FAILURE;

	ret = vmc96_initialize( &vmc96 );

	if( ret != VMC96_SUCCESS )
	{
		fprintf( stderr, "Error: %s (Cod: %d)\n", vmc96_get_error_code_string(ret), ret );
		return EXIT_FAILURE;
	}

	ret = vmc96cli_execute( vmc96, &args );

	vmc96_finish( vmc96 );

	if( ret != VMC96CLI_SUCCESS )
	{
		fprintf( stderr, "Error: %s\n", vmc96cli_get_error_code_string(ret) );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* eof */
