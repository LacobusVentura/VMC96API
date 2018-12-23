/*!
	\file vmc96api.c
	\brief VMC96 Board Vending Machine Application Programming Interface API
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

#ifdef __linux__
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#else
#error "Unexpected System."
#endif

#include <libftdi1/ftdi.h>

#include "vmc96api.h"


/* ********************************************************************* */
/* *                              DEFINES                              * */
/* ********************************************************************* */

/* VMC96 DEVICE */
#define VMC96_DEVICE_VENDOR_ID                            (0x0CE5)
#define VMC96_DEVICE_PRODUCT_ID                           (0x0023)

/* K1 PROTOCOL SPECIFICS */
#define VMC96_K1_MESSAGE_STX                              (0x35)
#define VMC96_K1_MESSAGE_MAX_LEN                          (255)
#define VMC96_K1_MESSAGE_MIN_LEN                          (5)
#define VMC96_K1_MESSAGE_DATA_MAX_LEN                     (250)
#define VMC96_K1_RESPONSE_POSITIVE_ACK                    (0x00)

/* K1 PROTOCOL REPONSE TYPES */
#define VMC96_K1_RESPONSE_TYPE_INVALID                    (-1)
#define VMC96_K1_RESPONSE_TYPE_ACK                        (1)
#define VMC96_K1_RESPONSE_TYPE_DATA                       (2)

/* DEVICE */
#define VMC96_DEFAULT_RESPONSE_DELAY_MS                   (10)
#define VMC96_MOTOR_MAX_CURRENT_READING_MA                (500)

/* VMC96 AVAILABLE CONTROLLERS */
#define VMC96_CONTROLLER_GLOBAL_BROADCAST                 (0x00)
#define VMC96_CONTROLLER_RELAY_BASE_ADDRESS               (0x26)
#define VMC96_CONTROLLER_RELAY_1                          (0x26)
#define VMC96_CONTROLLER_RELAY_2                          (0x27)
#define VMC96_CONTROLLER_MOTOR_ARRAY                      (0x30)

/* VMC96 GLOBAL COMMANDS */
#define VMC96_COMMAND_SIMPLE_PING                         (0x00)
#define VMC96_COMMAND_GLOBAL_RESET                        (0x01)
#define VMC96_COMMAND_KERNEL_VERSION                      (0x02)
#define VMC96_COMMAND_RESET                               (0x05)

/* VMC96 MOTOR ARRAY COMMANDS */
#define VMC96_COMMAND_MOTOR_RESET                         (0x05)
#define VMC96_COMMAND_MOTOR_STATUS_REQUEST                (0x10)
#define VMC96_COMMAND_MOTOR_SCAN_ARRAY                    (0x11)
#define VMC96_COMMAND_MOTOR_STOP_ALL                      (0x12)
#define VMC96_COMMAND_MOTOR_RUN                           (0x13)
#define VMC96_COMMAND_MOTOR_GIVE_PULSE                    (0x14)
#define VMC96_COMMAND_MOTOR_OPTO_LINE_STATUS              (0x15)

/* VMC96 GENERAL PURPOSE RELAYS COMMANDS */
#define VMC96_COMMAND_RELAY_FUNCTION                      (0x11)

/* HELPERS */
#define VMC96_GET_MOTOR_ID( _row, _col )                  (((_row + 1) << 4) + (_col + 1))
#define VMC96_GET_MOTOR_ROW( _mid )                       ( ( (_mid & 0xF0) >> 4 ) - 1 )
#define VMC96_GET_MOTOR_COL( _mid )                       ( ( _mid & 0x0F ) - 1 )
#define VMC96_GET_MOTOR_CURRENT_MA( _val )                (( VMC96_MOTOR_MAX_CURRENT_READING_MA * _val) / 255 )
#define VMC96_VALIDATE_MOTOR_COORDINATE( _row, _col )     ((_row < VMC96_MOTOR_ARRAY_ROWS_COUNT) && (_col < VMC96_MOTOR_ARRAY_COLUMNS_COUNT))

/* SLEEP/DELAY */
#ifdef __linux__
#define VMC96_SLEEP_MS( _t )    usleep( _t / 1000L )
#elif _WIN32
#define VMC96_SLEEP_MS( _t )    Sleep( _t )
#else
#define VMC96_SLEEP_MS( _t )
#endif

/* ********************************************************************* */
/* *                        STRUCTS AND DATA TYPES                     * */
/* ********************************************************************* */

typedef struct vmc96_message_s vmc96_message_t;


struct vmc96_message_s
{
	unsigned char id_controller;
	unsigned char command;
	unsigned char data[ VMC96_K1_MESSAGE_DATA_MAX_LEN ];
	unsigned char data_length;
	unsigned char k1[ VMC96_K1_MESSAGE_MAX_LEN ];
	unsigned char k1_length;
};


struct VMC96_s
{
	struct ftdi_context * ftdi;
	struct ftdi_version_info ftdi_version;
	vmc96_message_t message;
	vmc96_message_t response;
};


/* ********************************************************************* */
/* *                        PRIVATE PROTOTYPES                         * */
/* ********************************************************************* */

/*!
	\brief Dump Buffer
	\param fp
	\param desc
	\param buf
	\param len
	\return
*/
static void vmc96_dump_buffer( FILE * fp, const char * desc, unsigned char * buf, size_t len );

/*!
	\brief Calculate K1 Message Checksum
	\param vmc96
	\param buf
	\param buflen
	\return
*/
static char vmc96_calculate_checksum( unsigned char * buf, size_t buflen );

/*!
	\brief Send K1 Message
	\param vmc96
	\return
*/
static int vmc96_send_k1_message( VMC96_t * vmc96 );

/*!
	\brief Prepare K1 Message to send
	\param vmc96
	\return
*/
static int vmc96_prepare_k1_message( VMC96_t * vmc96 );

/*!
	\brief Parse K1 Message Response
	\param vmc96
	\return
*/
static int vmc96_parse_k1_response( VMC96_t * vmc96 );

/*!
	\brief Parse K1 Message Response Type
	\param vmc96
	\return
*/
static int vmc96_k1_parse_response_type( VMC96_t * vmc96 );

/*!
	\brief Send Message
	\param vmc96
	\return
*/
static int vmc96_send_message( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd );

/*!
	\brief Send Message With Data
	\param vmc96
	\return
*/
static int vmc96_send_message_ex( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd, unsigned char * data, unsigned char datalen );


/* ********************************************************************* */
/* *                             DEBUG                                 * */
/* ********************************************************************* */

static void vmc96_dump_buffer( FILE * fp, const char * desc, unsigned char * buf, size_t len )
{
#ifdef _DEBUG
	size_t i = 0;

	fprintf( fp, "[DEBUG] %s (%ld): ", desc, len );

	for( i = 0; i < len; i++ )
		fprintf( fp, "0x%02X ", buf[i] );

	fprintf( fp, "\n" );
#else
	(void) fp;
	(void) desc;
	(void) buf;
	(void) len;
#endif
}


/* ********************************************************************* */
/* *                        ERROR CONTROL                              * */
/* ********************************************************************* */

const char * vmc96_get_error_code_string( int cod )
{
	switch(cod)
	{
		case VMC96_SUCCESS                            : return "Success."; break;
		case VMC96_ERROR_OUT_OF_MEMORY                : return "Out of memory."; break;
		case VMC96_ERROR_FTDI_INITIALIZE              : return "Can not initialize libftdi."; break;
		case VMC96_ERROR_FTDI_SET_INTERFACE           : return "libftdi can not de interface."; break;
		case VMC96_ERROR_FTDI_OPEN_USB_DEVICE         : return "libftdi can not open USB device (not found or permission denied)."; break;
		case VMC96_ERROR_FTDI_RESET_USB               : return "libftdi can not reset USB."; break;
		case VMC96_ERROR_FTDI_SET_BAUDRATE            : return "libftdi can not set baud rate."; break;
		case VMC96_ERROR_FTDI_SET_LINE_PROPS          : return "libftdi can not set line properties"; break;
		case VMC96_ERROR_FTDI_SET_NO_FLOW             : return "libftdi can not set line in no flow mode."; break;
		case VMC96_ERROR_FTDI_WRITE_DATA              : return "libftdi can not write data to device."; break;
		case VMC96_ERROR_FTDI_READ_DATA               : return "libftdi can not read data from device."; break;
		case VMC96_ERROR_FTDI_PURGE_BUFFERS           : return "libftdi can not purge RX/TX buffers."; break;
		case VMC96_ERROR_K1_RESPONSE_INVALID_CHECKSUM : return "Response invalid checksum."; break;
		case VMC96_ERROR_K1_RESPONSE_NEGATIVE_ACK     : return "Response negative acknowledgement."; break;
		case VMC96_ERROR_K1_RESPONSE_MALFORMED        : return "Response malformed."; break;
		case VMC96_ERROR_K1_RESPONSE_INVALID_SOURCE   : return "Invalid response source."; break;
		case VMC96_ERROR_K1_RESPONSE_INVALID_LENGTH   : return "Invalid response length."; break;
		case VMC96_ERROR_INVALID_MOTOR_COORDINATES    : return "Invalid motor coordinates."; break;
		default                                       : return "Unknown error."; break;

	}
}


/* ********************************************************************* */
/* *               GENERAL PURPOSE RELAYS CONTROL FUNCTIONS            * */
/* ********************************************************************* */

int vmc96_relay_ping( VMC96_t * vmc96, unsigned char id )
{
	return vmc96_send_message( vmc96, VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id, VMC96_COMMAND_SIMPLE_PING );
}


int vmc96_relay_get_version( VMC96_t * vmc96, unsigned char id, char * version )
{
	int ret = 0;

	*version = '\0';

	ret = vmc96_send_message( vmc96, VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id, VMC96_COMMAND_KERNEL_VERSION );

	if( ret != VMC96_SUCCESS )
		return ret;

	if( vmc96->response.data_length > 0 )
	{
		memcpy( version, vmc96->response.data + 1, vmc96->response.data_length - 1 );
		version[ vmc96->response.data_length - 1 ] = '\0';
	}

	return VMC96_SUCCESS;
}


int vmc96_relay_reset( VMC96_t * vmc96, unsigned char id )
{
	return vmc96_send_message( vmc96, VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id, VMC96_COMMAND_RESET );
}


int vmc96_relay_control( VMC96_t * vmc96, unsigned char id, unsigned char state )
{
	unsigned char data = ( state ) ? 1 : 0;
	return vmc96_send_message_ex( vmc96, VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id, VMC96_COMMAND_RELAY_FUNCTION, &data, 1 );
}


/* ********************************************************************* */
/* *                  MOTOR ARRAY CONTROL FUNCTIONS                    * */
/* ********************************************************************* */

int vmc96_motor_ping( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_SIMPLE_PING );
}


int vmc96_motor_get_version( VMC96_t * vmc96, char * version )
{
	int ret = 0;

	*version = '\0';

	ret = vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_KERNEL_VERSION );

	if( ret != VMC96_SUCCESS )
		return ret;

	if( vmc96->response.data_length > 0 )
	{
		memcpy( version, vmc96->response.data + 1, vmc96->response.data_length - 1 );
		version[ vmc96->response.data_length - 1 ] = '\0';
	}

	return VMC96_SUCCESS;
}


int vmc96_motor_reset( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_RESET );
}


int vmc96_motor_get_status( VMC96_t * vmc96, VMC96_motor_array_status_t * status )
{
	int ret = 0;
	int i = 0;

	memset( status, 0, sizeof(VMC96_motor_array_status_t) );

	ret = vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_STATUS_REQUEST );

	if( ret != VMC96_SUCCESS )
		return ret;

	if( vmc96->response.data_length >= 2 )
	{
		if( vmc96->response.data[0] != VMC96_COMMAND_MOTOR_STATUS_REQUEST )
			return VMC96_ERROR_K1_RESPONSE_INVALID_SOURCE;

		status->current_ma = VMC96_GET_MOTOR_CURRENT_MA( vmc96->response.data[1] );

		status->active_count = vmc96->response.data_length - 2;

		memset( &status->array, 0, sizeof(VMC96_motor_array_t) );

		for( i = 0; i < vmc96->response.data_length - 2; i++ )
		{
			unsigned char row = VMC96_GET_MOTOR_ROW( vmc96->response.data[ i + 2 ] );
			unsigned char col = VMC96_GET_MOTOR_COL( vmc96->response.data[ i + 2 ] );

			status->array.motor[ row ][ col ] = 1;
		}
	}

	return VMC96_SUCCESS;
}


int vmc96_motor_stop_all( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_STOP_ALL );
}


int vmc96_motor_run( VMC96_t * vmc96, unsigned char row, unsigned char col )
{
	unsigned char data = VMC96_GET_MOTOR_ID( row, col );

	if( !VMC96_VALIDATE_MOTOR_COORDINATE( row, col) )
		return VMC96_ERROR_INVALID_MOTOR_COORDINATES;

	return vmc96_send_message_ex( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_RUN, &data, 1 );
}


int vmc96_motor_pair_run( VMC96_t * vmc96, unsigned char row, unsigned char col1, unsigned char col2 )
{
	unsigned char data[2] = { VMC96_GET_MOTOR_ID( row, col1 ), VMC96_GET_MOTOR_ID( row, col2 ) };

	if( !VMC96_VALIDATE_MOTOR_COORDINATE( row, col1 ) || !VMC96_VALIDATE_MOTOR_COORDINATE( row, col2 ) )
		return VMC96_ERROR_INVALID_MOTOR_COORDINATES;

	return vmc96_send_message_ex( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_RUN, data, 2 );
}


int vmc96_motor_opto_line_status( VMC96_t * vmc96, VMC96_opto_line_sample_block_t * status_block )
{
	int ret = 0;
	int i = 0;
	int j = 0;
	int k = 0;

	memset( status_block, 0, sizeof(VMC96_opto_line_sample_block_t) );

	ret = vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_OPTO_LINE_STATUS );

	if( ret != VMC96_SUCCESS )
		return ret;

	if( vmc96->response.data_length == 5 )
	{
		for( i = 0; i < 4; i++ )
		{
			for( j = 0; j < 8; j++ )
			{
				status_block->sample[ k++ ] = (vmc96->response.data[ i + 1 ] >> j) & 0x01;
			}
		}
	}

	return VMC96_SUCCESS;
}


int vmc96_motor_scan_array( VMC96_t * vmc96, VMC96_motor_array_scan_result_t * result )
{
	int ret = 0;
	unsigned char row = 0;
	unsigned char col = 0;

	ret = vmc96_send_message( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_SCAN_ARRAY );

	if( ret != VMC96_SUCCESS )
		return ret;

	if( vmc96->response.data_length >= 2 )
	{
		if( vmc96->response.data[0] != VMC96_COMMAND_MOTOR_SCAN_ARRAY )
			return VMC96_ERROR_K1_RESPONSE_INVALID_SOURCE;

		result->count = vmc96->response.data_length - 2;

		memset( &result->array, 0, sizeof(VMC96_motor_array_t) );

		for( row = 0; row < VMC96_MOTOR_ARRAY_ROWS_COUNT; row++ )
			for( col = 0; col < VMC96_MOTOR_ARRAY_COLUMNS_COUNT; col++ )
				result->array.motor[ row ][ col ] = ( vmc96->response.data[ 2 + row ] >> col ) & 0x1;
	}

	return VMC96_SUCCESS;
}


int vmc96_motor_give_pulse( VMC96_t * vmc96, unsigned char row, unsigned char col, unsigned char duration_ms )
{
	unsigned char data[2] = { VMC96_GET_MOTOR_ID( row, col ), duration_ms };

	if( !VMC96_VALIDATE_MOTOR_COORDINATE( row, col) )
		return VMC96_ERROR_INVALID_MOTOR_COORDINATES;

	return vmc96_send_message_ex( vmc96, VMC96_CONTROLLER_MOTOR_ARRAY, VMC96_COMMAND_MOTOR_GIVE_PULSE, data, 2 );
}


/* ********************************************************************* */
/* *                 GLOBAL COMMANDS CONTROL FUNCTION                  * */
/* ********************************************************************* */

int vmc96_global_reset( VMC96_t * vmc96 )
{
	unsigned char data = 0xFF;
	return vmc96_send_message_ex( vmc96, VMC96_CONTROLLER_GLOBAL_BROADCAST, VMC96_COMMAND_GLOBAL_RESET, &data, 1 );
}


/* ********************************************************************* */
/* *                    MESSAGE CONTROL FUNCTIONS                      * */
/* ********************************************************************* */

static char vmc96_calculate_checksum( unsigned char * buf, size_t buflen )
{
	char sum = 0;
	unsigned long i = 0;

	for( i = 0; i < buflen; i++ )
		sum ^= buf[i];

	return sum;
}


static int vmc96_prepare_k1_message( VMC96_t * vmc96 )
{
	vmc96->message.k1_length = vmc96->message.data_length + VMC96_K1_MESSAGE_MIN_LEN;

	/* K1 Message STX Header Field */
	vmc96->message.k1[0] = VMC96_K1_MESSAGE_STX;

	/* K1 Message: Controller Address/ID Field */
	vmc96->message.k1[1] = vmc96->message.id_controller;

	/* K1 Message: Total Length Field */
	vmc96->message.k1[2] = vmc96->message.k1_length;

	/* K1 Message: Command Code Field */
	vmc96->message.k1[3] = vmc96->message.command;

	/* K1 Message: Data Field */
	if( vmc96->message.data_length > 0 )
		memcpy( &vmc96->message.k1[4], vmc96->message.data, vmc96->message.data_length );

	/* K1 Message: Checksum Field */
	vmc96->message.k1[ vmc96->message.k1_length - 1 ] = vmc96_calculate_checksum( vmc96->message.k1, vmc96->message.k1_length - 1 );

	return VMC96_SUCCESS;
}


static int vmc96_k1_parse_response_type( VMC96_t * vmc96 )
{
	switch( vmc96->message.id_controller )
	{
		case VMC96_CONTROLLER_GLOBAL_BROADCAST:
		{
			switch( vmc96->message.command )
			{
				case VMC96_COMMAND_GLOBAL_RESET : return VMC96_K1_RESPONSE_TYPE_ACK;
				default                         : return VMC96_K1_RESPONSE_TYPE_INVALID;
			}
		}

		case VMC96_CONTROLLER_RELAY_1 :
		case VMC96_CONTROLLER_RELAY_2 :
		{
			switch( vmc96->message.command )
			{
				case VMC96_COMMAND_RESET          : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_SIMPLE_PING    : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_KERNEL_VERSION : return VMC96_K1_RESPONSE_TYPE_DATA;
				case VMC96_COMMAND_RELAY_FUNCTION : return VMC96_K1_RESPONSE_TYPE_ACK;
				default                           : return VMC96_K1_RESPONSE_TYPE_INVALID;
			}
		}

		case VMC96_CONTROLLER_MOTOR_ARRAY:
		{
			switch( vmc96->message.command )
			{
				case VMC96_COMMAND_RESET                  : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_SIMPLE_PING            : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_KERNEL_VERSION         : return VMC96_K1_RESPONSE_TYPE_DATA;
				case VMC96_COMMAND_MOTOR_RUN              : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_MOTOR_STOP_ALL         : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_MOTOR_STATUS_REQUEST   : return VMC96_K1_RESPONSE_TYPE_DATA;
				case VMC96_COMMAND_MOTOR_OPTO_LINE_STATUS : return VMC96_K1_RESPONSE_TYPE_DATA;
				case VMC96_COMMAND_MOTOR_GIVE_PULSE       : return VMC96_K1_RESPONSE_TYPE_ACK;
				case VMC96_COMMAND_MOTOR_SCAN_ARRAY       : return VMC96_K1_RESPONSE_TYPE_DATA;
				default                                   : return VMC96_K1_RESPONSE_TYPE_INVALID;
			}
		}

		default:
		{
			return VMC96_K1_RESPONSE_TYPE_INVALID;
		}
	}
}


static int vmc96_parse_k1_response( VMC96_t * vmc96 )
{
	switch( vmc96_k1_parse_response_type( vmc96 ) )
	{
		case VMC96_K1_RESPONSE_TYPE_ACK:
		{
			/* K1 Response: Validate Positive ACK Message Len */
			if( vmc96->response.k1_length != VMC96_K1_MESSAGE_MIN_LEN )
				return VMC96_ERROR_K1_RESPONSE_INVALID_LENGTH;

			/* K1 Response: Parse Source Controller ID/Address Field */
			vmc96->response.id_controller = vmc96->response.k1[1];

			/* K1 Response: Data Field Empty */
			memset( vmc96->response.data, 0, VMC96_K1_MESSAGE_DATA_MAX_LEN );
			vmc96->response.data_length = 0;

			/* K1 Response: Validating STX Header Field */
			if( vmc96->response.k1[0] != VMC96_K1_MESSAGE_STX )
				return VMC96_ERROR_K1_RESPONSE_MALFORMED;

			/* K1 Response: Validate Source Controller ID/Address Field */
			if( vmc96->response.k1[1] != vmc96->message.id_controller )
				return VMC96_ERROR_K1_RESPONSE_INVALID_SOURCE;

			/* K1 Response: Validate Positive ACK Message Len */
			if( vmc96->response.k1[2] != VMC96_K1_MESSAGE_MIN_LEN )
				return VMC96_ERROR_K1_RESPONSE_INVALID_LENGTH;

			/* K1 Response: Validate Checksum */
			if( vmc96->response.k1[4] != vmc96_calculate_checksum( vmc96->response.k1, vmc96->response.k1_length - 1 ) )
				return VMC96_ERROR_K1_RESPONSE_INVALID_CHECKSUM;

			/* K1 Response: Validate Positive ACK Field */
			if( vmc96->response.k1[3] != VMC96_K1_RESPONSE_POSITIVE_ACK )
				return VMC96_ERROR_K1_RESPONSE_NEGATIVE_ACK;

			return VMC96_SUCCESS;
		}

		case VMC96_K1_RESPONSE_TYPE_DATA:
		{
			/* K1 Response: Validating Message Length */
			if( vmc96->response.k1_length < VMC96_K1_MESSAGE_MIN_LEN )
				return VMC96_ERROR_K1_RESPONSE_INVALID_LENGTH;

			/* K1 Response: Parse Source Controller ID/Address Field */
			vmc96->response.id_controller = vmc96->response.k1[1];

			/* K1 Response: Parse Total Data Length Field */
			vmc96->response.data_length = vmc96->response.k1[2] - 4;

			/* K1 Response: Parse Data Field */
			memcpy( vmc96->response.data, &vmc96->response.k1[3], vmc96->response.data_length );

			/* K1 Response: Validating STX Header Field */
			if( vmc96->response.k1[0] != VMC96_K1_MESSAGE_STX )
				return VMC96_ERROR_K1_RESPONSE_MALFORMED;

			/* K1 Response: Validate Source Controller ID/Address Field */
			if( vmc96->response.k1[1] != vmc96->message.id_controller )
				return VMC96_ERROR_K1_RESPONSE_INVALID_SOURCE;

			/* K1 Response: Validate Total Length Field */
			if( vmc96->response.k1[2] != vmc96->response.k1_length )
				return VMC96_ERROR_K1_RESPONSE_INVALID_LENGTH;

			/* K1 Response: Validate Checksum */
			if( vmc96->response.k1[ vmc96->response.k1_length - 1 ] != vmc96_calculate_checksum( vmc96->response.k1, vmc96->response.k1_length - 1 ) )
				return VMC96_ERROR_K1_RESPONSE_INVALID_CHECKSUM;

			return VMC96_SUCCESS;
		}

		default:
		{
			return VMC96_ERROR_K1_RESPONSE_MALFORMED;
		}
	}
}


static int vmc96_send_k1_message( VMC96_t * vmc96 )
{
	int ret = 0;

	ret = ftdi_usb_purge_buffers( vmc96->ftdi );

	if( ret < 0 )
		return VMC96_ERROR_FTDI_PURGE_BUFFERS;

	ret = ftdi_write_data( vmc96->ftdi, vmc96->message.k1, vmc96->message.k1_length );

	if( ret < 0 )
		return VMC96_ERROR_FTDI_WRITE_DATA;

	VMC96_SLEEP_MS( VMC96_DEFAULT_RESPONSE_DELAY_MS );

	ret = ftdi_read_data( vmc96->ftdi, vmc96->response.k1, VMC96_K1_MESSAGE_MAX_LEN );

	if( ret < 0 )
		return VMC96_ERROR_FTDI_READ_DATA;

	vmc96->response.k1_length = ret;

	return VMC96_SUCCESS;
}


static int vmc96_send_message( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd )
{
	return vmc96_send_message_ex( vmc96, id_cntlr, cmd, NULL, 0 );
}


static int vmc96_send_message_ex( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd, unsigned char * data, unsigned char datalen )
{
	int ret = 0;

	vmc96->message.id_controller = id_cntlr;
	vmc96->message.command = cmd;

	memset( vmc96->message.data, 0, VMC96_K1_MESSAGE_DATA_MAX_LEN );
	vmc96->message.data_length = 0;

	if( (data != NULL) && (datalen > 0) )
	{
		memcpy( vmc96->message.data, data, datalen );
		vmc96->message.data_length = datalen;
	}

	ret = vmc96_prepare_k1_message( vmc96 );

	if( ret != VMC96_SUCCESS )
		return ret;

	vmc96_dump_buffer( stdout, "K1-MESSAGE", vmc96->message.k1, vmc96->message.k1_length );

	ret = vmc96_send_k1_message( vmc96 );

	if( ret != VMC96_SUCCESS )
		return ret;

	vmc96_dump_buffer( stdout, "K1-RESPONSE", vmc96->response.k1, vmc96->response.k1_length );

	return vmc96_parse_k1_response( vmc96 );
}


/* ********************************************************************* */
/* *                      CONSTRUCTOR/DESTRUCTOR                       * */
/* ********************************************************************* */

void vmc96_finish( VMC96_t * vmc96 )
{
	ftdi_usb_close( vmc96->ftdi );
	ftdi_free( vmc96->ftdi );
	free( vmc96 );
}


int vmc96_initialize( VMC96_t ** ppvmc96 )
{
	int ret = 0;
	VMC96_t * vmc96 = NULL;

	vmc96 = (VMC96_t*) calloc( 1, sizeof(VMC96_t) );

	if( !vmc96 )
		return VMC96_ERROR_OUT_OF_MEMORY;

	vmc96->ftdi = ftdi_new();

	if( !vmc96->ftdi )
	{
		ret = VMC96_ERROR_FTDI_INITIALIZE;
		goto error_cleanup;
	}

	vmc96->ftdi_version = ftdi_get_library_version();

	ret = ftdi_set_interface( vmc96->ftdi, INTERFACE_ANY );

	if( ret < 0 )
	{
		ret = VMC96_ERROR_FTDI_SET_INTERFACE;
		goto error_cleanup;
	}

	ret = ftdi_usb_open( vmc96->ftdi, VMC96_DEVICE_VENDOR_ID, VMC96_DEVICE_PRODUCT_ID );

	if( ret < 0 )
	{
		ret = VMC96_ERROR_FTDI_OPEN_USB_DEVICE;
		goto error_cleanup;
	}

	ret = ftdi_usb_reset( vmc96->ftdi );

	if( ret < 0 )
	{
		ret = VMC96_ERROR_FTDI_RESET_USB;
		goto error_cleanup;
	}

	ret = ftdi_set_baudrate( vmc96->ftdi, 19200 );

	if( ret < 0 )
	{
		ret = VMC96_ERROR_FTDI_SET_BAUDRATE;
		goto error_cleanup;
	}

	ret = ftdi_set_line_property( vmc96->ftdi, 8, STOP_BIT_1, NONE );

	if( ret < 0 )
	{
		ret = VMC96_ERROR_FTDI_SET_LINE_PROPS;
		goto error_cleanup;
	}

	ret = ftdi_setflowctrl( vmc96->ftdi, SIO_DISABLE_FLOW_CTRL );

	if( ret < 0 )
	{
		ret = VMC96_ERROR_FTDI_SET_NO_FLOW;
		goto error_cleanup;
	}

	*ppvmc96 = vmc96;

	return VMC96_SUCCESS;

error_cleanup:

	*ppvmc96 = NULL;

	ftdi_usb_close( vmc96->ftdi );
	ftdi_free( vmc96->ftdi );
	free( vmc96 );

	return ret;
}

/* eof */
