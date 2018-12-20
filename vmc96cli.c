/*!
	\file vmc96cli.c
	\brief VMC96 Board Vending Machine Controller Command Line Interface
	\author Tiago Ventura (tiago.ventura@gmail.com)
	\date Dec/2019

    VMC96 Module - http://www.moneyflex.net/vmc96/
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "vmc96api.h"


/* ********************************************************************* */
/* *                              DEFINES                              * */
/* ********************************************************************* */

/* CONTROLLERS */
#define VMC96CLI_CONTROLLER_GLOBAL                   (1)
#define VMC96CLI_CONTROLLER_RELAY1                   (2)
#define VMC96CLI_CONTROLLER_RELAY2                   (3)
#define VMC96CLI_CONTROLLER_MOTOR_ARRAY              (4)
#define VMC96CLI_CONTROLLER_INVALID                  (-1)
#define VMC96CLI_CONTROLLER_NOT_SPECIFIED            (-2)

/* COMMANDS */
#define VMC96CLI_COMMAND_RESET                       (1)
#define VMC96CLI_COMMAND_PING                        (2)
#define VMC96CLI_COMMAND_VERSION                     (3)
#define VMC96CLI_COMMAND_RELAY_CONTROL               (4)
#define VMC96CLI_COMMAND_OPTO_LINE_STATUS            (5)
#define VMC96CLI_COMMAND_MOTOR_RUN                   (6)
#define VMC96CLI_COMMAND_MOTOR_RUN_PAIR              (7)
#define VMC96CLI_COMMAND_MOTOR_STOP_ALL              (8)
#define VMC96CLI_COMMAND_MOTOR_STATUS                (9)
#define VMC96CLI_COMMAND_INVALID                     (-1)
#define VMC96CLI_COMMAND_NOT_SPECIFIED               (-2)

/* STATUS */
#define VMC96CLI_SUCCESS                             (0)
#define VMC96CLI_ERROR_CONTROLLER_INVALID            (-1)
#define VMC96CLI_ERROR_CONTROLLER_NOT_SPECIFIED      (-2)
#define VMC96CLI_ERROR_COMMAND_INVALID               (-3)
#define VMC96CLI_ERROR_COMMAND_NOT_SPECIFIED         (-4)
#define VMC96CLI_ERROR_COMMAND_FAILED                (-5)


/* ********************************************************************* */
/* *                        STRUCTS AND DATA TYPES                     * */
/* ********************************************************************* */

typedef struct vmc96cli_arguments_s vmc96cli_arguments_t;

struct vmc96cli_arguments_s
{
	int controller;
	int command;
	unsigned char state;
	unsigned char col;
	unsigned char row;
	unsigned char col1;
	unsigned char col2;
};


/* ********************************************************************* */
/* *                             PROTOTYPES                            * */
/* ********************************************************************* */

static const char * vmc96cli_get_error_code_string( int cod );
static void vmc96cli_show_usage( int argc, char ** argv );
static int vmc96cli_get_cntrl_code( const char * cntrl );
static int vmc96cli_get_cmd_code( const char * cmd );
static int vmc96cli_execute( VMC96_t * vmc96, vmc96cli_arguments_t * args );


/* ********************************************************************* */
/* *                          IMPLEMENTATION                           * */
/* ********************************************************************* */

static const char * vmc96cli_get_error_code_string( int cod )
{
	switch(cod)
	{
		case VMC96CLI_SUCCESS                        : return "Success."; break;
		case VMC96CLI_ERROR_CONTROLLER_INVALID       : return "Invalid Controller (--controller)."; break;
		case VMC96CLI_ERROR_COMMAND_INVALID          : return "Invalid Command (--command)."; break;
		case VMC96CLI_ERROR_CONTROLLER_NOT_SPECIFIED : return "Controller not specified. (--controller)"; break;
		case VMC96CLI_ERROR_COMMAND_NOT_SPECIFIED    : return "Command not especified (--command)."; break;
		case VMC96CLI_ERROR_COMMAND_FAILED           : return "Command failed."; break;
		default                                      : return "Unknown error."; break;
	}
}


static void vmc96cli_show_usage( int argc, char ** argv )
{
	(void) argc;

	printf( "Global Reset:\n\n" );
	printf( "	%s --controller=GLOBAL --command=RESET\n\n", argv[0] );
	printf( "Relay Ping:\n\n" );
	printf( "	%s --controller=[RELAY1|RELAY2] --command=PING\n\n", argv[0] );
	printf( "Relay Reset:\n\n" );
	printf( "	%s --controller=[RELAY1|RELAY2] --command=RESET\n\n", argv[0] );
	printf( "Relay Controller Version:\n\n" );
	printf( "	%s --controller=[RELAY1|RELAY2] --command=VERSION\n\n", argv[0] );
	printf( "Relay State Control:\n\n" );
	printf( "	%s --controller=[RELAY1|RELAY2] --command=CONTROL --state=[0|1]\n\n", argv[0] );
	printf( "Motor Array Ping:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=PING\n\n", argv[0] );
	printf( "Motor Array Reset:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=RESET\n\n", argv[0] );
	printf( "Motoer Array Controller Version:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=VERSION\n\n", argv[0] );
	printf( "Motor Array - Run Single Motor:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=RUN --row=[0-11] --column=[0-7]\n\n\n", argv[0] );
	printf( "Motor Array - Run Motor Pair:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=RUN_PAIR --row=[0-11] --column1=[0-7] --column2=[0-7]\n\n\n", argv[0] );
	printf( "Motor Array - Status:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=STATUS\n\n\n", argv[0] );
	printf( "Motor Array - Stop All Motors:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=STOP_ALL\n\n\n", argv[0] );
	printf( "Motor Array - Opto-Sensor Status:\n\n" );
	printf( "	%s --controller=MOTOR_ARRAY --command=OPTO_LINE_STATUS\n\n\n", argv[0] );
	printf( "Show Usage:\n\n" );
	printf( "	%s --help\n\n", argv[0] );
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
	else if( !strcasecmp( cmd, "" ) )
		return VMC96CLI_COMMAND_NOT_SPECIFIED;
	else
		return VMC96CLI_COMMAND_INVALID;
}


static int vmc96cli_execute( VMC96_t * vmc96, vmc96cli_arguments_t * args )
{
	int ret = 0;

	if( args->controller == VMC96CLI_CONTROLLER_NOT_SPECIFIED )
		return VMC96CLI_ERROR_CONTROLLER_NOT_SPECIFIED;

	if( args->controller == VMC96CLI_CONTROLLER_INVALID )
		return VMC96CLI_ERROR_CONTROLLER_INVALID;

	if( args->command == VMC96CLI_COMMAND_NOT_SPECIFIED )
		return VMC96CLI_ERROR_COMMAND_NOT_SPECIFIED;

	if( args->command == VMC96CLI_COMMAND_INVALID )
		return VMC96CLI_ERROR_COMMAND_INVALID;

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
					return VMC96CLI_ERROR_COMMAND_INVALID;
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
					return VMC96CLI_ERROR_COMMAND_INVALID;
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
					int i = 0;
					VMC96_motor_array_status_t status;

					ret = vmc96_motor_get_status( vmc96, &status );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					fprintf( stdout, "Active Motors Count: %d\n", status.active_count );
					fprintf( stdout, "Total Current Drained: %dmA\n", status.current_ma );

					if( status.active_count > 0 )
						fprintf( stdout, "Current Drained per Motor: %dmA\n", status.current_ma / status.active_count );

					for( i = 0; i < status.active_count; i++ )
						fprintf( stdout, "Motor(%d): col=%d row=%d\n", i, status.motors[i].col, status.motors[i].row );

					return VMC96CLI_SUCCESS;
				}

				case VMC96CLI_COMMAND_OPTO_LINE_STATUS:
				{
					int i = 0;
					uint32_t status = 0;

					ret = vmc96_motor_opto_line_status( vmc96, &status );

					if( ret != VMC96_SUCCESS )
					{
						fprintf( stderr, "Error: (%d) %s\n" , ret, vmc96_get_error_code_string(ret) );
						return VMC96CLI_ERROR_COMMAND_FAILED;
					}

					fprintf( stdout, "Opto Line Sensor Status:\n\n");

					fprintf( stdout, "	Samples per block: %d\n", VMC96_OPTO_LINE_SAMPLES_PER_BLOCK );
					fprintf( stdout, "	Total Samples: %d\n", VMC96_OPTO_LINE_SAMPLES_PER_BLOCK  );
					fprintf( stdout, "	Time per Sample: %gms\n", VMC96_OPTO_LINE_SAMPLE_LENGTH_US / 1000.0 );
					fprintf( stdout, "	Time per Block: %gs\n", VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_US / 1000000.0 );
					fprintf( stdout, "	Total time: %gs\n\n", (VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_US / 1000000.0));

					fprintf( stdout, "	Hexadecimal:\n" );
					fprintf( stdout, "		0x%08X\n",  status );
					fprintf( stdout, "	Binary:\n");

					fprintf( stdout, "		");

					for( i = 0; i < 32; i++ )
					{
						if( (i > 0) && (i % 8 == 0) )
							fprintf( stdout, "." );

						fprintf( stdout, "%d",  (status >> i) & 0x1 );
					}

					fprintf( stdout, "\n\n" );

					return VMC96CLI_SUCCESS;
				}

				default:
				{
					return VMC96CLI_ERROR_COMMAND_INVALID;
				}
			}
		}

		default:
		{
			return VMC96CLI_ERROR_CONTROLLER_INVALID;
		}
	}
}


/* ********************************************************************* */
/* *                                MAIN                               * */
/* ********************************************************************* */
int main( int argc, char ** argv )
{
	int ret = 0;
	int index = 0;
	vmc96cli_arguments_t args;
	VMC96_t * vmc96 = NULL;

	static struct option options[] =
	{
		{"controller",  required_argument, 0,  'a' },
		{"command",     required_argument, 0,  'b' },
		{"state",       required_argument, 0,  'c' },
		{"row",         required_argument, 0,  'd' },
		{"column",      required_argument, 0,  'e' },
		{"column1",     required_argument, 0,  'f' },
		{"column2",     required_argument, 0,  'g' },
		{"help",        no_argument,       0,  'h' },
		{0,             0,                 0,   0  }
	};

	memset( &args, 0, sizeof(vmc96cli_arguments_t) );
	args.controller = vmc96cli_get_cntrl_code("");
	args.command = vmc96cli_get_cmd_code("");

	while(true)
	{
		ret = getopt_long( argc, argv, "a:b:c:d:e:f:g:h", options, &index );

		if( ret == -1 )
			break;

		switch( ret )
		{
			case 'a' : args.controller = vmc96cli_get_cntrl_code( optarg ); break;
			case 'b' : args.command = vmc96cli_get_cmd_code( optarg ); break;
			case 'c' : args.state = atoi( optarg ); break;
			case 'd' : args.row = atoi( optarg ); break;
			case 'e' : args.col = atoi( optarg ); break;
			case 'f' : args.col1 = atoi( optarg ); break;
			case 'g' : args.col2 = atoi( optarg ); break;

			case 'h' :
				vmc96cli_show_usage( argc, argv );
				return EXIT_SUCCESS;

			default:
				vmc96cli_show_usage( argc, argv );
				return EXIT_FAILURE;
		}
	}

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
