# VMC96 APPLICATION PROGRAMMING INTERFACE (API)

A very simple Application Programming Interface (API) to control **VMC96** Vending Machine Controller Boards written in C language.

<p align="center"><img alt="VMC96 Board" src="https://raw.githubusercontent.com/LacobusVentura/vmc96/master/images/VMC96-Front.jpg" width="60%" height="60%"></p>

**VMC96** is a gateway to conventional vending components and let them to be controlled by your application. It can drive 96 dispense motors in a 8x12 matrix and it is “Opto-eye Ready” for users to build their “Sure Vend” style of optical detection. It gives machine owners an edge to modernize their old machines.

See more: http://www.moneyflex.net/vmc96/

# VMC96 Command Line Interface (CLI)

A Command Line Interface (CLI) utility to control VMC96 Vending Machine Controller Boards.

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
