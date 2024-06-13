## Overview
This guide will provide instructions on setting up your LED Sectional map, as well as how to update the software on your LED Sectional Map, should you ever wish to. 

The Map has a number of RGB LEDs installed, co-located with airports. The color of the LED changes based on real-time weather for that location. 

The Map requires a WiFi connection in order to get the real-time weather from the server on the public internet.  Without this connection, the Map will not display anything. 

## Contents
* [Setup](#Setup)
* [Usage](#Usage)
* [Updating Software](#Software)


## Setup
The Map has a 5V power plug which plugs into the socket at the bottom of the frame. Be careful when pulling the plug out of the frame so that the socket doesn't come out of the frame. 
The Map comes with a [mounting kit](https://amzn.to/4bOnr7P) that you can use to either hang the frame with a wire strung across the back of the frame, or by a hanging sawtooth that screws into the top of the frame and hangs on a nail or screw in your wall. 

When you first power on the Map, you will need to connect it to your WiFi. Once it is plugged in to power, use your smartphone to set the WiFi up.

1. Open the WiFi Settings on your smartphone and scan for networks. Look for a network called "LEDWeatherMap" can click on it.
1. A screen should open on your phone with a couple of blue buttons. Click the first button titled "Configure WiFi" (or "Configuration" for some devices).
1. On the next screen that appears, the WiFi networks in range will appear. Click the one you want to use, enter the password and click "Save".
1. Your Map will automatically reboot and attempt to connect to that WiFi network. If it works you should very quickly see the LEDs start to illuminate.
1. If it does not work, go back to step one and try again.

<img src="https://www.martyncurrey.com/wp-content/uploads/2017/10/ESP8266_wifiManager_002.png" width="800">

## Usage
When connected to the internet, the Map will cycle through each LED and set the color value. The initial LEDs are automatically set to the color corresponding to the Key/Legend.  The other LEDs will each light up the color corresponding to the weather data retrieved from Aviation Weather.
It takes about 1-2 minutes to complete all of the LEDs.  Once the LEDs are lit, the Map will wait for about 5 minutes before cycling through them and updating as needed based on changing weather conditions.

NOTE: Sometimes Aviation Weather will not return the weather for an airport. When this happens the LED will turn off. If you see off LEDs, just wait a few minutes, as typically Aviation Weather will provide the weather for that airport again eventually.
(Interestingly, this might be because the control tower at that airport is in the process of updating the weather, or the mechanism by which the weather automatically forwards on from the airport to Aviation Weather might be inoperative.)
You can always go to [metars.com](https://metars.com) to check the status of the weather at an airport. 


## Software

The Map uses an "Internet-of-Things" development board called a [Wemos d1 Mini](https://amzn.to/3VbZ6mU). This can be connected to your computer via a USB-C cable to update the embedded software. 
The software consists of TWO files:
* LED_Sectional_ME.ino
* airports50PWMTAC.h

The first file contains the program that the Map uses to connect to the internet and retreieve the weather info. The second file contains the information about the Map: how many LEDs are installed and which airports have LEDs. 

Not all airports are supported. The Map uses the Aviation Weather API which provides METAR information for many but not all US airports. So it is important to confirm that an airport is supported in the API before installing it on the Map and updating the file.


The following steps can be followed to update the embedded software on the Wemos included in your Map. Open the back of the map and look for the little circuit board. ***IMPORTANT*** Make sure you disconnect the external 5V power before connecting the board to your computer.

1. Install Arduino
1. Add Libraries
1. Add WeMos Device
1. Edit sketch
1. Upload to WeMos

### Install Arduino
1. Download and install the latest version of Arduino from the [Arduino Software](https://www.arduino.cc/en/software/) page.
1. [Optional] [Learn more](https://docs.arduino.cc/learn/starting-guide/getting-started-arduino/) about Arduino devices and programming

### Add Libraries
1. From the **Sketch** menu, > **Board** > **Board Manager**...  In the text input box type in "**WeMos**". Look for "**ESP8266 by ESP8266**" and select the latest version by clicking on the popup menu next to the **Install **button. Then click on the Install button. After it's installed, you can click the "close" button.

### Add WeMos Device
1. From the **Tools** menu, > **Include Library** > **Manage Libraries**...  In the text input box type in "**ESP8266**". Look for "**Adafruit ESP8266**" and select the latest version by clicking on the popup menu next to the **Install** button. Then click on the Install button. Also install any dependant libraries it requires at the same time. After it's installed, you can click the "close" button.
1. Do the same thing for the following other libraries: **FastLED** & **WiFiManager**

### Edit Sketch
1. Download the collection from [GitHub ](https://github.com/alliefm/LED_Weather_Map_ME/)as a ZIP file and unzip into a folder, noting the destination folder address.
1. Open Arduino and from the File menu select Open and select the folder you unzipped to. Select “LED_Sectional_ME.ino” and click Open. Edit the file as you wish.
1. Edit “airports50PWMTAC.h” as required.

### Upload to WeMos
1. Plug your WeMos module into your computer using the USB micro cable.
1. Use the drop-down list at the top of the screen and select “Select other board and port”. Select “LOLIN (WEMOS) D1 mini (clone) from the BOARDS list on the left and the PORT being used by your WeMos module on the right and click OK.
1. Click the Upload button 
1. The program will compile and then upload to your WeMos module. 

Once the software for the module has been updated it will automatically restart and try to connect.

