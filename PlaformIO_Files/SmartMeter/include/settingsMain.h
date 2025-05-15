/*
      Copyright 2021      Hans Beerman <hans.beerman@xs4all.nl>
                          
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

// software version
#define P1_MODULE_SOFTWARE_VERSION "P1 Port Smart Meter Version 1.0.3.1  Copyright Hans Beerman [2020 - 2025]"
#define DATE_OFF_VERSION "Date: 2025-05-15\n\r"

// * Baud rate for both hardware and software serial
//#define BAUD_RATE 115200 // baudrate for DSMR > V2.2
#define BAUD_RATE 115200 // baudrate for DSMR > V2.2
// settings for DSMR V2.2
#define BAUD_RATE_DSMR_2_2 9600
#define SERIAL_PORT_SETTINGS_DSMR_2_2 SERIAL_7E1

// * P1 Meter RX pin
//#define P1_SERIAL_RX D2

// DSMR version jumper. If jumper is installed, serial port settings are 9600 baud 7 bits even parity 1 stopbit
// if not serial port settings are 115200 baud 8 bits no parity 1 stopbit
#define PIN_DSMR_V2_2 D1

// * Max telegram length
#define P1_MAXLINELENGTH 200

// * The hostname of our little creature
#define HOSTNAME "p1-port-smart-meter"

// * Wifi timeout in milliseconds
#define WIFI_TIMEOUT 30000
// * WiFiManager timeout in seconds
#define WIFI_MANAGER_TIMEOUT 120 // 2 minutes

// * MQTT network settings
#define MQTT_MAX_RECONNECT_TRIES 10

// * MQTT root topic
#define MQTT_ROOT_TOPIC "sensors/power/p1meter"

// * MQTT Last reconnection counter
unsigned long LAST_RECONNECT_ATTEMPT = 0;

// * To be filled with EEPROM data

char MQTT_HOST[64] = MY_MQTT_HOST;
char MQTT_PORT[6]  = MY_MQTT_PORT;
char MQTT_USER[32] = MY_MQTT_USERNAME;
char MQTT_PASS[32] = MY_MQTT_PASSWORD;

// * Set to store received telegram
char telegram[P1_MAXLINELENGTH + 2];

// * set if P1 message is completely decoded
// bool P1_Message_Is_Decoded = false;
// bool previous_P1_Message_Is_Decoded = false;

// Valid/invalid messages count
int VALID_MESSAGES = 0;
int INVALID_MESSAGES = 0;

// * Set to store date - time value P1 message
char DATE_TIME_P1[25];
int P1_HOURS = -1;
int P1_MINUTES = -1;
char PREVIOUS_DATE_TIME_P1[25];
int PREVIOUS_P1_HOURS = -1;
int PREVIOUS_P1_MINUTES = -1;

// * Set to store the data values read
long CONSUMPTION_LOW_TARIF;
long CONSUMPTION_HIGH_TARIF;
long DELIVERY_LOW_TARIF;
long DELIVERY_HIGH_TARIF;
long ACTUAL_CONSUMPTION;
long ACTUAL_POWER;
long ACTUAL_DELIVERY;
long INSTANT_POWER_CURRENT;
long INSTANT_POWER_USAGE;
unsigned long INSTANT_VOLTAGE_L1 = 0;
unsigned long INSTANT_VOLTAGE_L2 = 0;
unsigned long INSTANT_VOLTAGE_L3 = 0;
unsigned long INSTANT_CURRENT_L1 = 0;
unsigned long INSTANT_CURRENT_L2 = 0;
unsigned long INSTANT_CURRENT_L3 = 0;
long INSTANT_ACTIVE_POWER_CONSUMPTION_L1 = 0;
long INSTANT_ACTIVE_POWER_CONSUMPTION_L2 = 0;
long INSTANT_ACTIVE_POWER_CONSUMPTION_L3 = 0;
long INSTANT_ACTIVE_POWER_DELIVERY_L1 = 0;
long INSTANT_ACTIVE_POWER_DELIVERY_L2 = 0;
long INSTANT_ACTIVE_POWER_DELIVERY_L3 = 0;
long ACTIVE_POWER_L1 = 0;
long ACTIVE_POWER_L2 = 0;
long ACTIVE_POWER_L3 = 0;
long ACTIVE_CURRENT_L1 = 0;
long ACTIVE_CURRENT_L2 = 0;
long ACTIVE_CURRENT_L3 = 0;

long SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L1 = 0;
long SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L2 = 0;
long SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L3 = 0;
long SUM_INSTANT_ACTIVE_POWER_DELIVERY_L1 = 0;
long SUM_INSTANT_ACTIVE_POWER_DELIVERY_L2 = 0;
long SUM_INSTANT_ACTIVE_POWER_DELIVERY_L3 = 0;

long GAS_METER_M3;
bool L1_DETECTED = false;
bool L2_DETECTED = false;
bool L3_DETECTED = false;

long PEAK_CONSUMPTION = 0;
long PEAK_DELIVERY = 0;

long PREVIOUS_CONSUMPTION_LOW_TARIF;
long PREVIOUS_CONSUMPTION_HIGH_TARIF;
long PREVIOUS_DELIVERY_LOW_TARIF;
long PREVIOUS_DELIVERY_HIGH_TARIF;
long PREVIOUS_ACTUAL_CONSUMPTION;
long PREVIOUS_ACTUAL_DELIVERY;
long PREVIOUS_INSTANT_POWER_CURRENT;
long PREVIOUS_INSTANT_POWER_USAGE;
long PREVIOUS_GAS_METER_M3;

// Set to store data counters read
long ACTUAL_TARIF;
long SHORT_POWER_OUTAGES;
long LONG_POWER_OUTAGES;
long SHORT_POWER_DROPS;
long SHORT_POWER_PEAKS;

long PREVIOUS_ACTUAL_TARIF;
long PREVIOUS_SHORT_POWER_OUTAGES;
long PREVIOUS_LONG_POWER_OUTAGES;
long PREVIOUS_SHORT_POWER_DROPS;
long PREVIOUS_SHORT_POWER_PEAKS;


// * Set during CRC checking
int currentCRC = 0;