# current-cost
Simple receiver for current_cost IAM watt data.

The Current Cost IAM (Individual Appliance Monitor) is a plug-in
energy monitor that sends Watt readings every 6 seconds.

It is well documented here: http://http://www.currentcost.com/
Although I think the company behind it went bust a while ago.

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

Program the fuses on your ATTiny85

```
./make_ccost write_fuses
```

Program the code on your ATTiny85
```
./make_ccost program
```

Clean out any generated files
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


David Whale

Thinking Binaries Ltd

August 2021.

