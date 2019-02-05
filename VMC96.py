#
#	\file VMC96.py
#	\brief VMC96 Board Application Programming Interface API
#	\author Tiago Ventura (tiago.ventura@gmail.com)
#	\date Jan.2019
#
#
#	Copyright (c) 2018 Tiago Ventura (tiago.ventura@gmail.com)
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in
#	all copies or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#	THE SOFTWARE.
#

import time
import usb.core
import pyftdi.ftdi as ftdi


class VMC96( object ):

	# FTDI Device Specs
	_FTDI_VENDOR_ID                     = 0x0ce5
	_FTDI_PRODUCT_ID                    = 0x0023
	_FTDI_BAUD_RATE                     = 19200

	# K1 Message Specs
	_MESSAGE_HEADER                     = 0x35
	_MESSAGE_MIN_LENGTH                 = 5
	_MESSAGE_MAX_LENGTH                 = 255
	_MESSAGE_RESPONSE_DELAY             = 0.01  #10ms
	_MESSAGE_READ_MAX_RETRY             = 100

	# Device Controllers
	_CONTROLLER_RELAY                   = 0x26
	_CONTROLLER_MOTOR                   = 0x30

	# Controller Commands
	_COMMAND_RELAY_CONTROL              = 0x11
	_COMMAND_RELAY_RESET                = 0x05
	_COMMAND_MOTOR_RUN                  = 0x13
	_COMMAND_MOTOR_STOP_ALL             = 0x12
	_COMMAND_MOTOR_RESET                = 0x05
	_COMMAND_MOTOR_OPTO_SENSOR_STATUS   = 0x15
	_COMMAND_MOTOR_SCAN_ARRAY           = 0x11

	# Message Parser Results
	_RESPONSE_VALID                     =  0
	_ERR_RESPONSE_MALFORMED             = -1
	_ERR_RESPONSE_INVALID_CHECKSUM      = -2
	_ERR_RESPONSE_INVALID_LENGTH        = -3
	_ERR_RESPONSE_NEGATIVE_ACK          = -4
	_ERR_RESPONSE_UNEXPECTED_CONTROLLER = -5

	# Log Callback Function
	on_log = None

	def __init__( self ):
		try:
			self.vmc = ftdi.Ftdi()
			self.vmc.open( self._FTDI_VENDOR_ID, self._FTDI_PRODUCT_ID )
			self.vmc.set_baudrate( self._FTDI_BAUD_RATE )
			self.vmc.set_line_property( 8, 1, 'N' )
			self.vmc.set_flowctrl('')
		except usb.core.USBError as e:
			raise RuntimeError("Error initializing VMC96 Device: " + str(e) )
		except:
			raise RuntimeError("Error initializing VMC96 Device")


	def __str__( self ):
		return str("VMC96")


	def _log( self, msg ):
		if( self.on_log != None ):
			self.on_log( msg )


	def _request_to_string( self, req ):
		if( len(req) < self._MESSAGE_MIN_LENGTH ):
			return str( ["0x{:02X}".format(i) for i in req] )
		return("[ hdr='0x{:02X}', cntrl='0x{:02X}', len='0x{:02X}', cmd='0x{:02X}', data={}, chksum='0x{:02X}' ]".format( req[0], req[1], req[2], req[3], str(["0x{:02X}".format(i) for i in req[4:-1]]), req[-1] ))


	def _response_to_string( self, resp ):
		if( len(resp) < self._MESSAGE_MIN_LENGTH ):
			return str( ["0x{:02X}".format(i) for i in resp] )
		return( "[ hdr='0x{:02X}', cntrl='0x{:02X}', len='0x{:02X}', data={}, chksum='0x{:02X}' ]".format( resp[0], resp[1], resp[2], str(["0x{:02X}".format(i) for i in resp[3:-1]]), resp[-1] ) )


	def _error_to_string( self, err ):
		if( err == self._RESPONSE_VALID ):
			return "Valid"
		elif( err == self._ERR_RESPONSE_MALFORMED ):
			return "Malformed"
		elif( err == self._ERR_RESPONSE_INVALID_CHECKSUM ):
			return "Invalid Checksum"
		elif( err == self._ERR_RESPONSE_INVALID_LENGTH ):
			return "Invalid Length"
		elif( err == self._ERR_RESPONSE_NEGATIVE_ACK ):
			return "Negative Acknowledgement"
		elif( err == self._ERR_RESPONSE_UNEXPECTED_CONTROLLER ):
			return "Unexpected Controller"
		else:
			return "Unknown Error"


	def _checksum( self, buf ):
		chksum = 0;
		for byte in buf:
			chksum ^= byte
		return chksum


	def _prepare_request( self, cntrl, cmd, args ):
		req = []
		req.append( self._MESSAGE_HEADER )
		req.append( cntrl )
		req.append( len(args) + self._MESSAGE_MIN_LENGTH )
		req.append( cmd )
		req += args
		req.append( self._checksum( req ) )
		return req


	def _parse_response( self, cntrl, resp ):
		if( len(resp) < self._MESSAGE_MIN_LENGTH ):
			return self._ERR_RESPONSE_INVALID_LENGTH, resp
		if( resp[0] != self._MESSAGE_HEADER ):
			return self._ERR_RESPONSE_MALFORMED, resp
		if( resp[1] != cntrl ):
			return self._ERR_RESPONSE_UNEXPECTED_CONTROLLER, resp
		if( resp[2] != len(resp) ):
			return self._ERR_RESPONSE_INVALID_LENGTH, resp
		if( resp[-1] != self._checksum(resp[:-1]) ):
			return self._ERR_RESPONSE_INVALID_CHECKSUM, resp
		return self._RESPONSE_VALID, resp[3:-1]


	def _send_request( self, req ):
		self._log( "VMC96 Request: " + self._request_to_string(req) )
		self.vmc.write_data( req )
		for trial in range( 0, self._MESSAGE_READ_MAX_RETRY  ):
			time.sleep( self._MESSAGE_RESPONSE_DELAY )
			resp = self.vmc.read_data_bytes( size=self._MESSAGE_MAX_LENGTH, attempt=1 )
			if( len(resp) > 0 ):
				break
		self._log( "VMC96 Response: " + self._response_to_string(resp) )
		return resp


	def _execute_command( self, cntrl, cmd, args=[] ):
		req = self._prepare_request( cntrl, cmd, args )
		resp = self._send_request( req )
		ret, data = self._parse_response( cntrl, resp )
		if( ret != self._RESPONSE_VALID ):
			raise RuntimeError( "Invalid Response: " + self._error_to_string(ret) )
		return data


	def motor_run( self, motor_id ):
		return self._execute_command( self._CONTROLLER_MOTOR, self._COMMAND_MOTOR_RUN, [motor_id] )


	def motor_stop_all( self ):
		return self._execute_command( self._CONTROLLER_MOTOR, self._COMMAND_MOTOR_STOP_ALL )


	def motor_reset( self ):
		return self._execute_command( self._CONTROLLER_MOTOR, self._COMMAND_MOTOR_RESET )


	def relay_reset( self, relay_id ):
		return self._execute_command( self._CONTROLLER_RELAY, self.COMMAND_RELAY_RESET )


	def relay_set_state( self, relay_id, state ):
		return self._execute_command( self._CONTROLLER_RELAY + relay_id, self._COMMAND_RELAY_CONTROL, [state] )


	def opto_sensor_read( self ):
		ret = []
		data = self._execute_command( self._CONTROLLER_MOTOR, self._COMMAND_MOTOR_OPTO_SENSOR_STATUS )
		if( len(data) != 5 ):
			raise RuntimeError( "Invalid Opto Sensor Response: " + self._error_to_string(self._ERR_RESPONSE_INVALID_LENGTH) )
		if( data[0] != self._COMMAND_MOTOR_OPTO_SENSOR_STATUS ):
			raise RuntimeError( "Invalid Opto Sensor Response: " + self._error_to_string(self._ERR_RESPONSE_MALFORMED) )
		for byte in data[1:]:
			for bit in range( 0, 8 ):
				ret.append( (byte >> bit) & 0x01 )
		return ret


	def motor_scan_array( self ):
		data = self._execute_command( self._CONTROLLER_MOTOR, self._COMMAND_MOTOR_SCAN_ARRAY )
		if( len(data) < 2 ):
			raise RuntimeError( "Invalid Motor Array Scan Response: " + self._error_to_string(self._ERR_RESPONSE_INVALID_LENGTH) )
		if( data[0] != self._COMMAND_MOTOR_SCAN_ARRAY ):
			raise RuntimeError( "Invalid Motor Array Scan Response: " + self._error_to_string(self._ERR_RESPONSE_MALFORMED) )

		motor_array = []
		for byte in data[1:]:
			cols = []
			for bit in range( 0, 8 ):
				cols.append( (byte >> bit) & 0x01 )
			motor_array.append(cols)

		status = []
		for idx_row, row in enumerate(motor_array):
			for idx_col, col in enumerate(row):
				if( motor_array[idx_row][idx_col] != 0 ):
					status.append( "0x{X}{X}".format( idx_row + 1, idx_col + 1 ) )

		current_ma = float(( 500.0 * int(data[1]) ) / 255.0)

		return { "current_ma": current_ma, "available_motors": status }

# end-of-file #
