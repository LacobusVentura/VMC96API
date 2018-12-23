/*!
	\file vmc96api.h
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

#ifndef __VMC96_H__
#define __VMC96_H__


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
#define VMC96_ERROR_K1_RESPONSE_INVALID_CHECKSUM   (201)
#define VMC96_ERROR_K1_RESPONSE_NEGATIVE_ACK       (202)
#define VMC96_ERROR_K1_RESPONSE_MALFORMED          (203)
#define VMC96_ERROR_K1_RESPONSE_INVALID_SOURCE     (204)
#define VMC96_ERROR_K1_RESPONSE_INVALID_LENGTH     (205)
#define VMC96_ERROR_K1_RESPONSE_TIMEOUT            (206)
#define VMC96_ERROR_INVALID_MOTOR_COORDINATES      (301)

#define VMC96_OPTO_LINE_SAMPLE_BLOCK_LENGTH_MS     (1280)  /* 1.28s block */
#define VMC96_OPTO_LINE_SAMPLE_LENGTH_MS           (40)    /* 40ms sample */
#define VMC96_OPTO_LINE_SAMPLES_PER_BLOCK          (32)    /* 32 samples per block */
#define VMC96_VERSION_STRING_MAX_LEN               (32)
#define VMC96_MOTOR_ARRAY_ROWS_COUNT               (8)
#define VMC96_MOTOR_ARRAY_COLUMNS_COUNT            (12)


typedef struct VMC96_s                         VMC96_t;
typedef struct VMC96_motor_array_s             VMC96_motor_array_t;
typedef struct VMC96_motor_array_scan_result_s VMC96_motor_array_scan_result_t;
typedef struct VMC96_motor_array_status_s      VMC96_motor_array_status_t;
typedef struct VMC96_opto_line_sample_block_s  VMC96_opto_line_sample_block_t;


/*!
	\brief Represents a Motor Array
*/
struct VMC96_motor_array_s
{
	unsigned char motor[ VMC96_MOTOR_ARRAY_ROWS_COUNT ][ VMC96_MOTOR_ARRAY_COLUMNS_COUNT ]; /*!< Motor Array */
};


/*!
	\brief Represents an Opto Line Sample Block
*/
struct VMC96_opto_line_sample_block_s
{
	unsigned char sample[ VMC96_OPTO_LINE_SAMPLES_PER_BLOCK ];  /*!< Sample Block */
};


/*!
	\brief Represents a Motor Array Status Object
*/
struct VMC96_motor_array_status_s
{
	VMC96_motor_array_t array;      /*!< Motor Array */
	unsigned char active_count;     /*!< Active Motors Count */
	unsigned int current_ma;        /*!< Total Current Drained in Milliamperes */
};


/*!
	\brief Represents a Motor Array Scan Result Object
*/
struct VMC96_motor_array_scan_result_s
{
	VMC96_motor_array_t array;    /*!< Motor Array */
	unsigned char count;          /*!< Motors Count */
};


#ifdef __cplusplus
extern "C"
{
#endif

	/*!
		\brief Create a VMC96 Context Object.
		\param vmc96 VMC96 Context Object To be Created.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_initialize( VMC96_t ** vmc96 );

	/*!
		\brief Destroy a VMC96 Context Object.
		\param vmc96 Pointer to VMC96 context object to be destroyed.
		\return void
	*/
	void vmc96_finish( VMC96_t * vmc96 );

	/*!
		\brief Translate an error code to a human readable string.
		\param cod Error code to translate.
		\return Returns a pointer to a zero terminated string.
	*/
	const char * vmc96_get_error_code_string( int cod );

	/*!
		\brief Global Reset (All Controllers).
		\param vmc96 Pointer to VMC96 Context Object.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_global_reset( VMC96_t * vmc96 );

	/*!
		\brief Ping General Purpose Relay Controller.
		\param vmc96 Pointer to VMC96 Context Object.
		\param id Relay ID.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_relay_ping( VMC96_t * vmc96, unsigned char id );

	/*!
		\brief Retrieve Motor Array Controller Version.
		\param vmc96 Pointer to VMC96 Context Object.
		\param id Relay ID.
		\param version Buffer to store version string
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_relay_get_version( VMC96_t * vmc96, unsigned char id, char * version );

	/*!
		\brief Reset General Purpose Relay Controller.
		\param vmc96 Pointer to VMC96 Context Object.
		\param id Relay ID.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_relay_reset( VMC96_t * vmc96, unsigned char id );

	/*!
		\brief Set General Purpose Relay State (ON/OFF).
		\param vmc96 Pointer to VMC96 Context Object.
		\param id Relay ID.
		\param state
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_relay_control( VMC96_t * vmc96, unsigned char id, unsigned char state );

	/*!
		\brief Ping Motor Array Controller.
		\param vmc96 Pointer to VMC96 Context Object.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_ping( VMC96_t * vmc96 );

	/*!
		\brief Retrieve Motor Array Controller Version.
		\param vmc96 Pointer to VMC96 Context Object.
		\param version Buffer to store version string
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_get_version( VMC96_t * vmc96, char * version );

	/*!
		\brief Reset Motor Array Controller.
		\param vmc96 Pointer to VMC96 Context Object.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_reset( VMC96_t * vmc96 );

	/*!
		\brief Retrieve Running Motors Status.
		\param vmc96 Pointer to VMC96 Context Object.
		\param status
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_get_status( VMC96_t * vmc96, VMC96_motor_array_status_t * status );

	/*!
		\brief Stop All Running Motors.
		\param vmc96 Pointer to VMC96 Context Object.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_stop_all( VMC96_t * vmc96 );

	/*!
		\brief Run Single Motor.
		\param vmc96 Pointer to VMC96 Context Object.
		\param row Motor Array Row Coordinate.
		\param col Motor Array Column Coordinate.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_run( VMC96_t * vmc96, unsigned char row, unsigned char col );

	/*!
		\brief Run a pair of motors in the same row.
		\param vmc96 Pointer to VMC96 Context Object.
		\param row Motor Array Row Coordinate.
		\param col1 Motor Array First Column Coordinate.
		\param col2 Motor Array Second Column Coordinate.
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_pair_run( VMC96_t * vmc96, unsigned char row, unsigned char col1, unsigned char col2 );

	/*!
		\brief Retrieve Opto Line Status.
		\param vmc96 Pointer to VMC96 Context Object.
		\param status
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_opto_line_status( VMC96_t * vmc96, VMC96_opto_line_sample_block_t * status );

	/*!
		\brief Scan Motor Array.
		\param vmc96 Pointer to VMC96 Context Object.
		\param result
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_scan_array( VMC96_t * vmc96, VMC96_motor_array_scan_result_t * result );

	/*!
		\brief Motor Array Give Pulse
		\param vmc96 Pointer to VMC96 Context Object
		\param row Motor Array Row Coordinate
		\param col Motor Array Column Coordinate
		\param duration_ms Pulse duration in milliseconds (1 to 255 ms)
		\return Returns VMC96_SUCCESS in case of success.
	*/
	int vmc96_motor_give_pulse( VMC96_t * vmc96, unsigned char row, unsigned char col, unsigned char duration_ms );

#ifdef __cplusplus
}
#endif

#endif

/* eof */
