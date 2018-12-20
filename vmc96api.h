/*!
	\file vmc96api.h
	\brief VMC96 Board Vending Machine Application Programming Interface API
	\author Tiago Ventura (tiago.ventura@gmail.com)
	\date Dec/2019

	VMC96 Module - http://www.moneyflex.net/vmc96/
*/

#ifndef __VMC96_H__
#define __VMC96_H__

#include <stdint.h>
#include <stdbool.h>

#define VMC96_SUCCESS                              (0)
#define VMC96_ERROR_OUT_OF_MEMORY                  (1)
#define VMC96_ERROR_FTDI_INITIALIZE                (101)
#define VMC96_ERROR_FTDI_SET_INTERFACE             (102)
#define VMC96_ERROR_FTDI_OPEN_USB_DEVICE           (103)
#define VMC96_ERROR_FTDI_RESET_USB                 (104)
#define VMC96_ERROR_FTDI_SET_BAUDRATE              (105)
#define VMC96_ERROR_FTDI_SET_LINE_PROPS            (106)
#define VMC96_ERROR_FTDI_SET_NO_FLOW               (107)
#define VMC96_ERROR_FTDI_WRITE_DATA                (108)
#define VMC96_ERROR_FTDI_READ_DATA                 (109)
#define VMC96_ERROR_FTDI_PURGE_BUFFERS             (110)
#define VMC96_ERROR_RESPONSE_INVALID_CHECKSUM      (201)
#define VMC96_ERROR_RESPONSE_NEGATIVE_ACK          (202)
#define VMC96_ERROR_RESPONSE_MALFORMED             (203)
#define VMC96_ERROR_RESPONSE_INVALID_SOURCE        (204)
#define VMC96_ERROR_RESPONSE_INVALID_LENGTH        (205)
#define VMC96_ERROR_INVALID_MOTOR_COORDINATES      (301)

#define VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_US     (1280000L) /* 1.28s block*/
#define VMC96_OPTO_LINE_SAMPLE_LENGTH_US           (40000L)   /* 40ms sample */
#define VMC96_OPTO_LINE_SAMPLES_PER_BLOCK          (32)       /* 32 samples  per block */
#define VMC96_VERSION_STRING_MAX_LEN               (32)
#define VMC96_MOTOR_ARRAY_ROWS_COUNT               (8)
#define VMC96_MOTOR_ARRAY_COLUMNS_COUNT            (12)


typedef struct VMC96_s VMC96_t;
typedef struct VMC96_motor_array_status_s VMC96_motor_array_status_t;
typedef struct VMC96_motor_coordinates_s VMC96_motor_coordinates_t;


struct VMC96_motor_coordinates_s
{
	unsigned char col;
	unsigned char row;
};


struct VMC96_motor_array_status_s
{
	VMC96_motor_coordinates_t motors[ VMC96_MOTOR_ARRAY_ROWS_COUNT ];
	unsigned char active_count;
	unsigned int current_ma;
};


#ifdef __cplusplus
extern "C"
{
#endif

	int vmc96_initialize( VMC96_t ** vmc96 );
	void vmc96_finish( VMC96_t * vmc96 );

	const char * vmc96_get_error_code_string( int cod );

	int vmc96_global_reset( VMC96_t * vmc96 );

	int vmc96_relay_ping( VMC96_t * vmc96, unsigned char	 id );
	int vmc96_relay_get_version( VMC96_t * vmc96, unsigned char id, char * version );
	int vmc96_relay_reset( VMC96_t * vmc96, unsigned char id );
	int vmc96_relay_control( VMC96_t * vmc96, unsigned char id, bool state );

	int vmc96_motor_ping( VMC96_t * vmc96 );
	int vmc96_motor_get_version( VMC96_t * vmc96, char * version );
	int vmc96_motor_reset( VMC96_t * vmc96 );
	int vmc96_motor_get_status( VMC96_t * vmc96, VMC96_motor_array_status_t * status );
	int vmc96_motor_stop_all( VMC96_t * vmc96 );
	int vmc96_motor_run( VMC96_t * vmc96, unsigned char row, unsigned char col );
	int vmc96_motor_pair_run( VMC96_t * vmc96, unsigned char row, unsigned char col1, unsigned char col2 );

	int vmc96_motor_opto_line_status( VMC96_t * vmc96, uint32_t * status );


#ifdef __cplusplus
}
#endif

#endif

/* eof */
