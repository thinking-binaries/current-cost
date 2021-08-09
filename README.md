# current-cost
Simple receiver for current_cost IAM watt data.

The Current Cost IAM (Individual Appliance Monitor) is a plug-in
energy monitor that sends Watt readings every 6 seconds.

It is well documented here: http://www.currentcost.com/
although I think the company behind it went bust a while ago.

This repository provides code that runs on an ATTiny85 from Atmel,
that talks to a HopeRFM69CW module and receives and decodes the
payloads, dumping them in simple CSV format on a serial port
with a baud rate of 9600bps.

The CSV has 5 columns, namely:
Message type,ID(hex),Watt1,Watt2,Watt3.

The Current Cost meter only outputs into the Watt1 column.
It will normally send a METER message.
If you press the pairing button, it will assign a new random ID and
start sending (fast) PAIR messages for a couple of minutes. If the
load on the plug increases, it sends these PAIR messages even faster.
The id is a 12 bit Hex number, randomly chosen at pairing time.

Watt readings are 15 bit numbers, in whole Watts. The high bit is
used to mark valid and invalid readings, and the sample decoder
provided already checks this bit for you and suppresses any readings
that are not valid.

## Parts you need:

This is not a blow-by-blow tutorial on how to build this project,
I assume you know what you are doing and can work most of it out
yourself.

ATTiny85 from Atmel. 8 pin DIP or 8 pin SMD device (such as an Adafruit Gemma).

SparkFun RFM69HCW module, 433MHz version: https://www.sparkfun.com/products/12823

173mm of wire, to build an aerial.
 

## How to build and flash code

This code is written 'to the metal' and as such although it uses
the Arduino IDE compilers, it does not use the IDE or the libraries.
The code is quite small and could easily be ported to the standard
Arduino IDE with SPI and Serial libraries if you wanted (see below).

make sure there is a build/tmp directory.
```
cd build
mkdir tmp
```

make sure the TOOLDIR constant in the build/makefile is correct for your
system. For the Mac and the Adafruit Arduino IDE 1.0.5 this is as follows:

```
TOOLDIR = /Applications/AdafruitArduino1.0.5.app/Contents/Resources/Java
```

For another IDE, or for Windows or Linux, you need to put the full path
to the directory above the 'bin' dir (the bin dir has all the tools
in it). For later IDE's you may have to fiddle with AVRDIR and BINDIR
also, as some distributions of the embedded WinAVR and MacAVR toolsets
move things around.

Make the code

```
./make_ccost
```

Program the fuses on your ATTiny85, so it uses the RC oscillator
at 8MHz (instead of the factory default 1MHz) to get the correct
timing for the serial port. You only have to do this once for
any new chip, as it retains the values across flash cycles.

```
./make_ccost set-fuses
```

Program the code on your ATTiny85. I use an Arduino UNO in ArduinoISP mode
and connect the 6 wires to the ATTiny85 (reset, gnd, 3v3, MOSI, MISO SCLK)
as detailed in the 'Arduino as ISP' sketch).

```
./make_ccost program
```

Clean out any generated files, before you check this code back into a
source code repository.
```
./make_ccost clean
```

## Porting to Arduino IDE

If, for example, you want to run this code on an ESP to bridge data
to WiFi and/or MQTT, you will probably want the code in a standard
IDE build.

The SPI included is a software SPI driver, so it can easily map to any
pins. You could replace it with the standard Arduino SPI library. Apart
from in setup(), all the accesses to SPI are inside the ism.c module,
where it writes to the register set of the RFM69 radio chip.

The UART included is a software UART running at 9600bps, again so it can
easily be mapped to any pin. You could easily change this to be replaced
with the standard Arduino Serial library. The only real use of this is
for displaying the decoded payloads, inside the ccost.c application.

Pin mappings are defined with macros inside port.h, so they can easily
be re-assigned to any pin. If porting to the Arduino libraries, you probably
just want to ditch this file, and use the standard Arduino ports that are
used with the Arduino SPI and Serial libraries.


## Warning about different RFM69 radio types

The RFM69 chip from HopeRF (and modules and breakout boards) come in
various shapes and sizes. Some run at 3V3, some run at 5V. The 5V
ones have a level shifter on so you can use them with a real Arduino.
I power my ATTiny85 from two AA cells at 3V, but beware that the RFM
chip is 3V3 device. I also use a FTDI-TTL232-3V3-WE serial/USB lead
from EasySync to get serial data into my Mac - make sure you use the
correct voltage level for the radio board you have.

The RFM69HCW part (specifically as fitted to the SparkFun breakout
board listed above) is a high power part (H means High Power) and
uses a different output path for transmits. You won't use transmit
on this project, but if you further use the radio for other
transmit uses, note that there are comments in the ism.c file
showing registers you change for the low power or high power
radios (as you won't get any transmit energy into the aerial
if you use the wrong configuration for that radio/module/board).
Basically, the HCW part transmits via the PA_BOOST pin.

David Whale

Thinking Binaries Ltd

August 2021.

