# SmartScale
## What is it
What is SmartScale? The idea for a "smart scale" came about when I was 
grinding coffee and got tired of moving the portafilter between my grinder
and scale to measure my exact amount. I figured it should be quite easy to
hook up an ESP8266 to a load cell for measuring the amount of coffee, and
control the grinder directly in some way. For this initial prototype I have
connected my ESP8266 to a 1 kg load cell via a cheap HX711 breakout board,
and to a relay for controlling the grinder. The way it operates is that
I will have to hold the manual grind button on the grinder, and the "SmartScale"
will cut the power to the grinder when the desired amount is measured. The 
mains power stuff is enclosed in a plastic box, and the load cell is mounted to
a very simple 3D-printed platform.

## Hardware hookup
Connect the ESP8266 to the SCK and DOUT pins on the HX711 breakout board (I use
[this](https://www.electrokit.com/produkt/hx711-lastcellsforstarkare-monterad-pa-kort/) 
particular board). The pins on the ESP8266 can be configured in the `config.h`
file. Connect another pin on the ESP8266 to a relay (I use something like 
[this](https://www.electrokit.com/produkt/relamodul-5v/)) along with power
and ground. Make sure the relay can be controlled by a 3.3V signal, if you 
intend to power it directly from the ESP8266.

Here is what the prototype on my desk looks like:
<img src="https://github.com/iceaway/SmartScale/blob/master/images/everything.jpg?raw=true" alt="components making up the smart scale project" width="50%">
1. NodeMCUv3 microcontroller
2. HX711 breakout board
3. 3D-printed scale platform with 1 kg load cell inside
4. Plastic box I had lying around for keeping the mains voltage safely away from poking fingers. The white cable has a regular power socket on the end for connecting the power plug of the grinder. On the side of the box (out of view) is a [C14](https://www.electrokit.com/produkt/natbrunn-c14-jordad-flatstift-6-3mm/) socket for connecting the mains voltage. Inside this box the relay is located.

The connections are as follows:
| MCU | HX177 | RELAY |
| --- | ----- | ----- |
| D1  | DOUT/DT | NC  |
| D2  | SCK   | NC    |
| D3  | NC    | S/Control |
| 3.3V | VCC | VCC |
| GND | GND | GND |

## Usage
When powered on the first time, no wifi-credentials will be stored in the
ESP8266, and it will start an Access Point called "SmartScale" that you can
connect to using your phone or tablet. Once connected navigate to 
`http://192.168.4.1` and configure the WiFi you want to use. After configuration
the scale will restart, and you can connect to it on your regular WiFi. For now,
mDNS is not supported, so you will have to check the IP-address on the serial
console or on your DHCP server. On the web page, it is possible to manually 
tare the scale, see the current weight, configure the target weight (at which
weight the relay will be toggled), and reset the relay for making another run.

The serial inteface has some basic configuration
options, including calibration, which is not available on the web page yet. 
When connected to the serial terminal (9600 baud), press 'h' to get some
information on the possible commands.

## Todo
### Code

- [ ] Enable calibration from the web page
- [ ] Impove web page design
- [x] Add support for mDNS

### Hardware

- [x] Upload FreeCad files for platform 
- [ ] Improve CAD design to fit the ESP8266 and HX711 board
- [ ] Design and print an enclosure for the mains power stuff and MCU
- [ ] Design a custom PCB for mounting the ESP8266 and HX711 parts

### Misc

- [ ] Add photos of the scale and how it is set up / connected
- [ ] Add screenshots
