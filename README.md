# VMC96 APPLICATION PROGRAMMING INTERFACE (API)

A very simple Application Programming Interface (API) to control **VMC96** Vending Machine Controller Boards written in C language.

<p align="center"><img alt="VMC96 Board" src="https://raw.githubusercontent.com/LacobusVentura/vmc96/master/images/VMC96-Front.jpg" width="60%" height="60%"></p>

**VMC96** is a gateway to conventional vending components and let them to be controlled by your application. It can drive 96 dispense motors in a 8x12 matrix and it is “Opto-eye Ready” for users to build their “Sure Vend” style of optical detection. It gives machine owners an edge to modernize their old machines. (See more: http://www.moneyflex.net/vmc96/)

## Interface Functions

```C
int vmc96_initialize( VMC96_t ** vmc96 );

void vmc96_finish( VMC96_t * vmc96 );

const char * vmc96_get_error_code_string( int cod );

int vmc96_global_reset( VMC96_t * vmc96 );

int vmc96_relay_ping( VMC96_t * vmc96, unsigned char id );

int vmc96_relay_get_version( VMC96_t * vmc96, unsigned char id, char * version );

int vmc96_relay_reset( VMC96_t * vmc96, unsigned char id );

int vmc96_relay_control( VMC96_t * vmc96, unsigned char id, unsigned char state );

int vmc96_motor_ping( VMC96_t * vmc96 );

int vmc96_motor_get_version( VMC96_t * vmc96, char * version );

int vmc96_motor_reset( VMC96_t * vmc96 );

int vmc96_motor_get_status( VMC96_t * vmc96, VMC96_motor_array_status_t * status );

int vmc96_motor_stop_all( VMC96_t * vmc96 );

int vmc96_motor_run( VMC96_t * vmc96, unsigned char row, unsigned char col );

int vmc96_motor_pair_run( VMC96_t * vmc96, unsigned char row, unsigned char col1, unsigned char col2 );

int vmc96_motor_opto_line_status( VMC96_t * vmc96, VMC96_opto_line_sample_block_t * status );

int vmc96_motor_scan_array( VMC96_t * vmc96, VMC96_motor_array_scan_result_t * result );

int vmc96_motor_give_pulse( VMC96_t * vmc96, unsigned char row, unsigned char col, unsigned char duration_ms );
```

# VMC96 Command Line Interface (CLI)

A Command Line Interface (CLI) utility to control VMC96 Vending Machine Controller Boards.

## Compiling

**Release Mode:**
``` 
$ make
```
**Debug Mode:**
```
$ make DEBUG=1
```

## Command Syntax

**Global Reset:**
```
$ vmc96cli --controller=GLOBAL --command=RESET
```
**General Purpose Relays / Ping:**
```
$ vmc96cli --controller=[RELAY1|RELAY2] --command=PING
```
**General Purpose Relays / Reset:**
```
$ vmc96cli --controller=[RELAY1|RELAY2] --command=RESET
```
**General Purpose Relays / Controller Version:**
```
$ vmc96cli --controller=[RELAY1|RELAY2] --command=VERSION
```
**General Purpose Relays  / State Control:**
```
$ vmc96cli --controller=[RELAY1|RELAY2] --command=CONTROL --state=[0|1]
```
**Motor Array / Ping:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=PING
```
**Motor Array / Reset:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=RESET
```
**Motor Array / Get Controller Version:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=VERSION
```
**Motor Array / Run Single Motor:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=RUN --row=[0-11] --column=[0-7]
```
**Motor Array / Run Motor Pair:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=RUN_PAIR --row=[0-11] --column1=[0-7] --column2=[0-7]
```
**Motor Array / Scan Array:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=SCAN
```
**Motor Array / Give Pulse:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=GIVE_PULSE --row=[0-11] --column=[0-7] --duration=[1-255]
```
**Motor Array / Get Status:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=STATUS
```
**Motor Array / Stop All Motors:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=STOP_ALL
```
**Motor Array / Opto-Sensor Status:**
```
$ vmc96cli --controller=MOTOR_ARRAY --command=OPTO_LINE_STATUS
```
**Show Usage:**
```
$ vmc96cli --help
```
## Author

 This project was written and is maintained by Tiago Ventura (*tiago.ventura(at)gmail.com*).
 
 ## License
 ```
 Copyright (c) 2018 - Tiago Ventura

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
```
