/*
	 MoneyFlex.net

	 VMC96 Module - http://www.moneyflex.net/vmc96/

	 Device Control Library

	 Author: Tiago Ventura
	 Contact: tiago.ventura@gmail.com
	 Date: Dec/2019

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "vmc96.h"

#include <libftdi1/ftdi.h>


/* ********************************************************************* */
/* *                              DEFINES                              * */
/* ********************************************************************* */

/* VMC96 DEVICE */
#define VMC96_DEVICE_VENDOR_ID                            (0x0CE5)
#define VMC96_DEVICE_PRODUCT_ID                           (0x0023)

/* K1 PROTOCOL SPECIFICS */
#define VMC96_MESSAGE_STX                                 (0x35)
#define VMC96_MESSAGE_MAX_LEN                             (255)
#define VMC96_MESSAGE_DATA_MAX_LEN                        (250)
#define VMC96_POSITIVE_ACK_CODE                           (0x00)
#define VMC96_NEGATIVE_ACK_CODE                           (0xFF)
#define VMC96_DEFAULT_RESPONSE_DELAY_US                   (20000L)  /* 20 ms*/
#define VMC96_LONG_RESPONSE_DELAY_US                      (300000L) /* 300 ms */

/* VMC96 AVAILABLE CONTROLLERS */
#define VMC96_CONTROLLER_GLOBAL_BROADCAST                 (0x00)
#define VMC96_CONTROLLER_RELAY_BASE_ADDRESS               (0x26)
#define VMC96_CONTROLLER_RELAY_1                          (0x26)
#define VMC96_CONTROLLER_RELAY_2                          (0x27)
#define VMC96_CONTROLLER_MOTOR_ARRAY                      (0x30)

/* GLOBAL COMMANDS */
#define VMC96_COMMAND_SIMPLE_PING                         (0x00)
#define VMC96_COMMAND_GLOBAL_RESET                        (0x01)
#define VMC96_COMMAND_KERNEL_VERSION                      (0x02)
#define VMC96_COMMAND_GLOBAL_SUSPEND                      (0x03)
#define VMC96_COMMAND_RESET                               (0x05)
#define VMC96_COMMAND_ADDRESS_CLASH                       (0x05)

/* GENERAL PURPOSE RELAY COMMANDS */
#define VMC96_COMMAND_RELAY_FUNCTION                      (0x11)

/* MOTOR ARRAY COMMANDS */
#define VMC96_COMMAND_MOTOR_DRIVER_SETUP                  (0x04)
#define VMC96_COMMAND_MOTOR_RESET                         (0x05)
#define VMC96_COMMAND_MOTOR_STATUS_REQUEST                (0x10)
#define VMC96_COMMAND_MOTOR_SCAN_ARRAY                    (0x11)
#define VMC96_COMMAND_MOTOR_STOP_ALL                      (0x12)
#define VMC96_COMMAND_MOTOR_RUN                           (0x13)
#define VMC96_COMMAND_MOTOR_GIVE_PULSE                    (0x14)
#define VMC96_COMMAND_MOTOR_OPTO_LINE_STATUS              (0x15)


/* ********************************************************************* */
/* *                        STRUCTS AND DATA TYPES                     * */
/* ********************************************************************* */

typedef struct vmc96_message_s vmc96_message_t;


struct vmc96_message_s
{
	unsigned char id_controller;
	unsigned char command;
	unsigned char data[ VMC96_MESSAGE_DATA_MAX_LEN ];
	unsigned char data_length;
	unsigned char raw[ VMC96_MESSAGE_MAX_LEN ];
	unsigned char raw_length;
};


struct VMC96_s
{
	struct ftdi_context * ftdi;
	struct ftdi_version_info ftdi_version;
	unsigned long response_delay_us;
	vmc96_message_t message;
	vmc96_message_t response;
};


/* ********************************************************************* */
/* *                        PRIVATE PROTOTYPES                         * */
/* ********************************************************************* */

static void vmc96_dump_buffer( FILE * fp, const char * desc, unsigned char * buf, size_t len );
static char vmc96_calculate_checksum( unsigned char * buf, size_t buflen );
static int vmc96_send_raw_message( VMC96_t * vmc96 );
static int vmc96_prepare_raw_message( VMC96_t * vmc96 );
static int vmc96_parse_raw_response( VMC96_t * vmc96 );
static int vmc96_send_message( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd );
static int vmc96_send_message_ex( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd, unsigned char * data, unsigned char datalen, unsigned long wait_delay );


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

static void vmc96_dump_bit_buffer( FILE * fp, const char * desc, unsigned char * buf, size_t len )
{
#ifdef _DEBUG
	size_t i = 0;
	unsigned char j = 0;

	fprintf( fp, "[DEBUG] %s (%ld): ", desc, len );

	for( i = 0; i < len; i++ )
		for( j = 0; j < 8; j++ )
			fprintf( fp, "%d", ( buf[i] << j ) ? 1 : 0 );

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
		case VMC96_SUCCESS                         : return "Success."; break;
		case VMC96_ERROR_OUT_OF_MEMORY             : return "Out of memory."; break;
		case VMC96_ERROR_FTDI_INITIALIZE           : return "Can not initialize libftdi."; break;
		case VMC96_ERROR_FTDI_SET_INTERFACE        : return "libftdi can not de inteface."; break;
		case VMC96_ERROR_FTDI_OPEN_USB_DEVICE      : return "libftdi can not open USB device (not found or permission denied)."; break;
		case VMC96_ERROR_FTDI_RESET_USB            : return "libftdi can not reset USB."; break;
		case VMC96_ERROR_FTDI_SET_BAUDRATE         : return "libftdi can not set baud rate."; break;
		case VMC96_ERROR_FTDI_SET_LINE_PROPS       : return "libftdi can not set line properties"; break;
		case VMC96_ERROR_FTDI_SET_NO_FLOW          : return "libftdi can not set line in no flow mode."; break;
		case VMC96_ERROR_FTDI_WRITE_DATA           : return "libftdi can not write data to device."; break;
		case VMC96_ERROR_FTDI_READ_DATA            : return "libftdi can not read data from device."; break;
		case VMC96_ERROR_FTDI_PURGE_BUFFERS        : return "libftdi can not purge rx/tx buffers."; break;
		case VMC96_ERROR_RESPONSE_INVALID_CHECKSUM : return "Response invalid checksum."; break;
		case VMC96_ERROR_RESPONSE_NEGATIVE_ACK     : return "Response negative acknowledgement."; break;
		case VMC96_ERROR_RESPONSE_MALFORMED        : return "Response malformed."; break;
		case VMC96_ERROR_RESPONSE_INVALID_SOURCE   : return "Invalid response source."; break;
		case VMC96_ERROR_RESPONSE_INVALID_LENGTH   : return "Invalid response length."; break;
		default                                    : return "Unknown error."; break;
	}
}


/* ********************************************************************* */
/* *                     RELAY CONTROLLER                              * */
/* ********************************************************************* */

int vmc96_relay_ping( VMC96_t * vmc96, unsigned char id )
{
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id,
	                           VMC96_COMMAND_SIMPLE_PING );
}


int vmc96_relay_get_version( VMC96_t * vmc96, unsigned char id, char * version )
{
	int ret = 0;

	ret = vmc96_send_message( vmc96,
	                          VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id,
	                          VMC96_COMMAND_KERNEL_VERSION );

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
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id,
	                           VMC96_COMMAND_RESET );
}


int vmc96_relay_control( VMC96_t * vmc96, unsigned char id, unsigned char state )
{
	return vmc96_send_message_ex( vmc96,
	                              VMC96_CONTROLLER_RELAY_BASE_ADDRESS + id,
	                              VMC96_COMMAND_RELAY_FUNCTION,
	                              &state,
	                              1,
	                              VMC96_DEFAULT_RESPONSE_DELAY_US );
}


/* ********************************************************************* */
/* *                     MOTOR CONTROLLER                              * */
/* ********************************************************************* */

int vmc96_motor_ping( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_MOTOR_ARRAY,
	                           VMC96_COMMAND_SIMPLE_PING );
}


int vmc96_motor_get_version( VMC96_t * vmc96, char * version )
{
	int ret = 0;

	ret = vmc96_send_message( vmc96,
	                          VMC96_CONTROLLER_MOTOR_ARRAY,
	                          VMC96_COMMAND_KERNEL_VERSION );

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
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_MOTOR_ARRAY,
	                           VMC96_COMMAND_RESET );
}


int vmc96_motor_driver_setup( VMC96_t * vmc96, unsigned char mode )
{
	return vmc96_send_message_ex( vmc96,
	                              VMC96_CONTROLLER_MOTOR_ARRAY,
	                              VMC96_COMMAND_MOTOR_DRIVER_SETUP,
	                              &mode,
	                              1,
	                              VMC96_DEFAULT_RESPONSE_DELAY_US );
}


int vmc96_motor_status_request( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_MOTOR_ARRAY,
	                           VMC96_COMMAND_MOTOR_STATUS_REQUEST );
}


int vmc96_motor_scan_array( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_MOTOR_ARRAY,
	                           VMC96_COMMAND_MOTOR_SCAN_ARRAY );
}


int vmc96_motor_stop_all( VMC96_t * vmc96 )
{
	return vmc96_send_message( vmc96,
	                           VMC96_CONTROLLER_MOTOR_ARRAY,
	                           VMC96_COMMAND_MOTOR_STOP_ALL );
}


int vmc96_motor_run( VMC96_t * vmc96, unsigned char id )
{
	return vmc96_send_message_ex( vmc96,
	                              VMC96_CONTROLLER_MOTOR_ARRAY,
	                              VMC96_COMMAND_MOTOR_RUN,
	                              &id,
	                              1,
	                              VMC96_DEFAULT_RESPONSE_DELAY_US );
}


int vmc96_motor_pulse( VMC96_t * vmc96, unsigned char id, unsigned char duration_ms )
{
	unsigned char data[2] = { id, duration_ms };
	
	return vmc96_send_message_ex( vmc96,
	                              VMC96_CONTROLLER_MOTOR_ARRAY,
	                              VMC96_COMMAND_MOTOR_GIVE_PULSE,
	                              data,
	                              2,
	                              VMC96_DEFAULT_RESPONSE_DELAY_US );
}


int vmc96_motor_opto_line_status( VMC96_t * vmc96, unsigned int * status  )
{
	int ret = 0;
	
	*status = 0;
	
	ret = vmc96_send_message( vmc96,
	                          VMC96_CONTROLLER_MOTOR_ARRAY,
	                          VMC96_COMMAND_MOTOR_OPTO_LINE_STATUS );
	                                                    
	if( ret != VMC96_SUCCESS )
		return ret;
		
	if( vmc96->response.data_length == 5 )
		memcpy( status, &vmc96->response.data[1], vmc96->response.data_length - 1 );
		
	vmc96_dump_bit_buffer( stdout, "OPTO LINE STATUS", (unsigned char*)status, sizeof(unsigned int) );
	
	return VMC96_SUCCESS;
}


/* ********************************************************************* */
/* *                       GLOBAL COMMANDS                             * */
/* ********************************************************************* */

int vmc96_global_reset( VMC96_t * vmc96 )
{
	unsigned char data = 0xFF;

	return vmc96_send_message_ex( vmc96,
	                              VMC96_CONTROLLER_GLOBAL_BROADCAST,
	                              VMC96_COMMAND_GLOBAL_RESET,
	                              &data,
	                              1,
	                              VMC96_LONG_RESPONSE_DELAY_US );
}


int vmc96_global_address_clash( VMC96_t * vmc96, char * listaddr, unsigned char * count )
{
	int ret = 0;

	ret = vmc96_send_message_ex( vmc96,
	                             VMC96_CONTROLLER_GLOBAL_BROADCAST,
	                             VMC96_COMMAND_ADDRESS_CLASH,
	                             NULL,
	                             0,
	                             VMC96_LONG_RESPONSE_DELAY_US );

	if( ret != VMC96_SUCCESS )
		return ret;

	memcpy( listaddr, vmc96->response.data, vmc96->response.data_length );
	*count = vmc96->response.data_length;

	return VMC96_SUCCESS;
}


/* ********************************************************************* */
/* *                        MESSAGE CONTROL                            * */
/* ********************************************************************* */

static char vmc96_calculate_checksum( unsigned char * buf, size_t buflen )
{
	char sum = 0;
	unsigned long i = 0;

	for( i = 0; i < buflen; i++ )
		sum ^= buf[i];

	return sum;
}


static int vmc96_prepare_raw_message( VMC96_t * vmc96 )
{
	vmc96->message.raw_length = vmc96->message.data_length + 5;

	vmc96->message.raw[0] = VMC96_MESSAGE_STX;
	vmc96->message.raw[1] = vmc96->message.id_controller;
	vmc96->message.raw[2] = vmc96->message.raw_length;
	vmc96->message.raw[3] = vmc96->message.command;

	if( vmc96->message.data_length > 0 )
		memcpy( &vmc96->message.raw[4], vmc96->message.data, vmc96->message.data_length );

	vmc96->message.raw[ vmc96->message.raw_length - 1 ] = vmc96_calculate_checksum( vmc96->message.raw, vmc96->message.raw_length - 1 );

	return VMC96_SUCCESS;
}


static int vmc96_parse_raw_response( VMC96_t * vmc96 )
{
	switch( vmc96->message.id_controller )
	{
		case VMC96_CONTROLLER_GLOBAL_BROADCAST:
		{
			if( vmc96->response.raw_length <= 0 )
				return VMC96_ERROR_RESPONSE_INVALID_LENGTH;

			vmc96->response.data_length = vmc96->response.raw_length;
			memcpy( vmc96->response.data, &vmc96->response.raw, vmc96->response.raw_length );

			break;
		}

		case VMC96_CONTROLLER_RELAY_1 :
		case VMC96_CONTROLLER_RELAY_2 :
		case VMC96_CONTROLLER_MOTOR_ARRAY:
		{
			if( vmc96->response.raw_length < 5 )
				return VMC96_ERROR_RESPONSE_INVALID_LENGTH;

			vmc96->response.id_controller = vmc96->response.raw[1];
			vmc96->response.data_length = vmc96->response.raw[2] - 4;
			memcpy( vmc96->response.data, &vmc96->response.raw[3], vmc96->response.data_length );

			if( vmc96->response.raw[0] != VMC96_MESSAGE_STX )
				return VMC96_ERROR_RESPONSE_MALFORMED;

			if( vmc96->response.raw[1] != vmc96->message.id_controller )
				return VMC96_ERROR_RESPONSE_INVALID_SOURCE;

			if( vmc96->response.raw[2] != vmc96->response.raw_length )
				return VMC96_ERROR_RESPONSE_INVALID_LENGTH;

			//if( vmc96->response.raw[3] != VMC96_POSITIVE_ACK_CODE )
			//	return VMC96_ERROR_RESPONSE_NEGATIVE_ACK;

			if( vmc96->response.raw[ vmc96->response.raw_length - 1 ] != vmc96_calculate_checksum( vmc96->response.raw, vmc96->response.raw_length - 1 ) )
				return VMC96_ERROR_RESPONSE_INVALID_CHECKSUM;

			break;
		}

		default:
		{
			break;
		}
	}

	return VMC96_SUCCESS;
}


static int vmc96_send_raw_message( VMC96_t * vmc96 )
{
	int ret = 0;

	ret = ftdi_usb_purge_buffers( vmc96->ftdi );

	if( ret < 0 )
		return VMC96_ERROR_FTDI_PURGE_BUFFERS;

	ret = ftdi_write_data( vmc96->ftdi, vmc96->message.raw, vmc96->message.raw_length );

	if( ret < 0 )
		return VMC96_ERROR_FTDI_WRITE_DATA;

	usleep( vmc96->response_delay_us );

	ret = ftdi_read_data( vmc96->ftdi, vmc96->response.raw, VMC96_MESSAGE_MAX_LEN );

	if( ret < 0 )
		return VMC96_ERROR_FTDI_READ_DATA;

	vmc96->response.raw_length = ret;

	return VMC96_SUCCESS;
}


static int vmc96_send_message( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd )
{
	return vmc96_send_message_ex( vmc96, id_cntlr, cmd, NULL, 0, VMC96_DEFAULT_RESPONSE_DELAY_US );
}


static int vmc96_send_message_ex( VMC96_t * vmc96, unsigned char id_cntlr, unsigned char cmd, unsigned char * data, unsigned char datalen, unsigned long resp_delay )
{
	int ret = 0;

	vmc96->message.id_controller = id_cntlr;
	vmc96->message.command = cmd;
	vmc96->response_delay_us = resp_delay;

	if( (data != NULL) && (datalen > 0) )
	{
		memcpy( vmc96->message.data, data, datalen );
		vmc96->message.data_length = datalen;
	}
	else
	{
		memset( vmc96->message.data, 0, VMC96_MESSAGE_DATA_MAX_LEN );
		vmc96->message.data_length = 0;
	}

	ret = vmc96_prepare_raw_message( vmc96 );

	if( ret != VMC96_SUCCESS )
		return ret;

	vmc96_dump_buffer( stdout, "MESSAGE", vmc96->message.raw, vmc96->message.raw_length );

	ret = vmc96_send_raw_message( vmc96 );

	if( ret != VMC96_SUCCESS )
		return ret;

	vmc96_dump_buffer( stdout, "\tRESPONSE", vmc96->response.raw, vmc96->response.raw_length );

	return vmc96_parse_raw_response( vmc96 );
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
