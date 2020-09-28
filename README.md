# ESP32-Serial-Bridge

This is a simplified WifiBridge with a Webserver allowing for webconfiguration of the ports. The settings are stored in SPIFFS. It is based on AlphaLima's WifiBridge Project. Simplified means, it only supports AP mode and no Bluetooth.
The rest is analogous to his project: https://github.com/AlphaLima/ESP32-Serial-Bridge

The .ino file is the code for the ESP32. Use Arduino IDE for ESP32 to compile and upload it to the ESP32.

Default Parameters for the Accesspoint
IPAdress: 192.168.4.1                                           
AP SSID: XCSoar                                                   
AP Password: Flightcomputer                                       
Used Ports:                                                                                                          
192.168.4.1:4352  <-> COM0                                     
192.168.4.1:4353  <-> COM1                                     
192.168.4.1:2000  <-> COM2                                     

These values can all be changed on the web interface. Just switch off mobile data on your mobile phone and connect throught the browser to http://192.168.4.1
Press "save" then "restart"

===============================================================

Used Libraries: (must be installed in the arduino IDE, directly throug the IDE):

https://github.com/espressif/arduino-esp32

- webconfig (version 1.2.0), 
--> https://www.arduinolibraries.info/libraries/web-config
--> ArduinoJson, WebServer and SPIFFS are required by WebConfig

===============================================================

The code works perfectly on a NodeMCU development board; If you have such a Board, select "ESP32 Dev Module" in the Boardmanager.



