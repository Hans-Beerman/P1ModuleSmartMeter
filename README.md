**P1-module smart meter**

The P1-module is a small module, based on a Wemos D1 Mini (with an ESP8266) to collect information from the P1 port of a (Dutch) smart meter.

This module can be used for smart meters running DSMR Version 2.2 or higher.

For smart meters with DSMR V2.2 a jumper must be placed and the PCB must be powered by a 5V USB power supply. For more information, see the README.md in the KiCad directory of this repository

**Software for the P1-module**

The information collected from the smart meter is sent to a mqtt server of your own choice. The details about this server must be set in the .passwd.h file in the include directory of the PlatformIO project. This file must be added by yourself and should contain the following:

_/\* \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*_

_PLEASE READ THIS FIRST_

_Add a file to the include directory of this project with the_

_following name:_

_.passwd.h_

_The content of this file should be:_

_#pragma once_

_// WiFi credentials_

_#define WIFI\_NETWORK &quot;YOUR\_WIFI\_NETWORK\_NAME&quot;_

_#define WIFI\_PASSWD &quot;YOUR\_WIFI\_NETWORK\_PASSWORD&quot;_

_#define FALLBACK\_WIFI\_NETWORK &quot;Dummy&quot; // or chose any other name you like_

_#define FALLBACK\_WIFI\_PASSWD &quot;12345678&quot; // or take another password_

_// mqtt server_

_#define MY\_MQTT\_HOST &quot;your\_mqtt\_host e.g. 10.10.10.10&quot;_

_#define MY\_MQTT\_PORT &quot;1883&quot;_

_#define MY\_MQTT\_USERNAME &quot;your\_mqtt\_server\_username&quot;_

_#define MY\_MQTT\_PASSWORD &quot;your\_mqtt\_server.password&quot;_

_// OTA credentials_

_#define OTA\_PASSWORD &quot;MyPassW00rd&quot; // must be the same in platformio.ini_

_Also check in the platformio.ini file the upload\_port set._

_In the current file this port is set for a serial connection to a_

_Linux Ubuntu PC for a Windows PC running Visual Studio Code with_

_PlatformIO you should use something like:_

_upload\_port = COM3_

_Or any other comport number which is used for the serial connection_

_For further information see the README.md in the repository on GitHub_

_\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* \*/_

**WiFi network settings**

Default the P1-module uses the WiFi credentials as set in the .passw.h file, because in main.cpp the following #define is set:

_#define USE\_DEFAULT\_WIFI\_SETTINGS true_

If this #define is set to false:

_#define USE\_DEFAULT\_WIFI\_SETTINGS false_

In that case, if the P1-module is not able to connect to a WiFi network, the P1-module will set up a WiFi Access Point (AP) with the FALLBACK WiFi credentials as set in the .passwd.h file. Also a webserver (with IP address: 192.168.4.1) will be started.

If you connect, e.g. with a phone, a tablet or a PC to this AP, you will be able to select and configure a WiFi network and the mqtt server information, with the help of a web browser. The information set will be stored in eeprom.

**Overview of repository**

This repository has the following subdirs:

- **KiCad\_files:**

The design of the PCB used for the Node/Controller.

- **PlatformIO\_Files:**

The source code for the P1-module. For the development of this software PlatformIO is used, an Extension on Visual Studio Code.
