#include <Arduino.h>
#include <FS.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiManager.h>
#include "ESPTelnet.h" 
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTP.h> // install NTP by Stefan Staub
//
// information about NTP.h, see: https://platformio.org/lib/show/5438/NTP
//

/* ***********************************************************************
PLEASE READ THIS FIRST

Add a file to the include directory of this project with the 
following name:

.passwd.h

The content of this file should be:

#pragma once

// WiFi credentials
#define WIFI_NETWORK "YOUR_WIFI_NETWORK_NAME"
#define WIFI_PASSWD "YOUR_WIFI_NETWORK_PASSWORD"

#define FALLBACK_WIFI_NETWORK "Dummy"  // or chose any other name you like
#define FALLBACK_WIFI_PASSWD "12345678" // or take another password

// mqtt server
#define MY_MQTT_HOST "your_mqtt_host e.g. 10.10.10.10"
#define MY_MQTT_PORT "1883"
#define MY_MQTT_USERNAME "your_mqtt_server_username"
#define MY_MQTT_PASSWORD "your_mqtt_server.password"

// OTA credentials
#define OTA_PASSWORD "MyPassW00rd" // must be the same in platformio.ini

Also check in the platformio.ini file the upload_port set.

In the current file this port is set for a serial connection to a

Linux Ubuntu PC for a Windows PC running Visual Studio Code with
PlatformIO you should use something like:
upload_port = COM3
Or any other comport number which is used for the serial connection

For further information see the README.md in the repository on GitHub

*********************************************************************** */

// * Include settings
#include ".passwd.h"
#include "settingsMain.h"

// * include EEProm functions
#include "MyEEprom.h"
#define MODULE_NAME "P1PortModule"

#define USE_DEFAULT_WIFI_SETTINGS true

// * Initiate led blinker library
Ticker ticker;

// * Initiate WIFI client
WiFiClient espClient;

// WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// * Initiate telnet server
ESPTelnet telnet;

// * IP address module
IPAddress ip_addr;

// * for NTP
#define NTP_UPDATE_WINDOW (1000 * 60 * 60) // in ms = 1 hour

unsigned long lastNTPUpdateTime = 0;

WiFiUDP wifiUDP;

char topic[255];
char logMess[255];
bool firstStart = true;
unsigned long measureStartTime = 0;

// To test DSMRV2.2 functionality on > DSMRV2.2 smart meter, uncomment below statement
//#define TEST_DSMRV2_2

// In case DSMR V2.2 is used
bool DSMRV2_2IsUsed = false;

bool timeStampReceived = false;

int rndNumber;
int rndNumber2;
int rndNumber3;

// **********************************
// * Function definitions           *
// **********************************

void send_mqtt_message(const char *topic, const char *payload);

// **********************************
// * Ticker (System LED Blinker)    *
// **********************************

// * Blink on-board Led
void tick()
{
    // * Toggle state
    int state = digitalRead(LED_BUILTIN);    // * Get the current state of GPIO1 pin
    digitalWrite(LED_BUILTIN, !state);       // * Set pin to the opposite state
}

// **********************************
// * NTP                            *
// **********************************

NTP ntp(wifiUDP);

char ntpDateTimeStr[64];
char newDateTimeStr[64];
unsigned long NTPUpdatedTime = 0;
int currentHour;
int currentMinutes;
int currentSecs;
int currentDay;
int currentMonth;
int currentYear;

void initNTP() {
  ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120); // last sunday in march 2:00, timezone +120min (+1 GMT + 1h summertime offset)
  ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60); // last sunday in october 3:00, timezone +60min (+1 GMT)
  ntp.begin(); 
  ntp.update();

  currentHour = ntp.hours();
  currentMinutes = ntp.minutes();
  currentSecs = ntp.seconds();
  currentDay = ntp.day();
  currentMonth = ntp.month();
  currentYear = ntp.year();

  snprintf(ntpDateTimeStr, sizeof(ntpDateTimeStr), "Last boot: %4d-%02d-%02d %02d:%02d:%02d", currentYear, currentMonth, currentDay, currentHour, currentMinutes, currentSecs);

  ntp.updateInterval(NTP_UPDATE_WINDOW);
  ntp.update();
}

void getCurrentNTPTime() {
  currentHour = ntp.hours();
  currentMinutes = ntp.minutes();
  currentSecs = ntp.seconds();
  currentDay = ntp.day();
  currentMonth = ntp.month();
  currentYear = ntp.year();

  snprintf(newDateTimeStr, sizeof(newDateTimeStr), "%4d-%02d-%02d %02d:%02d:%02d", currentYear, currentMonth, currentDay, currentHour, currentMinutes, currentSecs);
}

// **********************************
// * telnet                           *
// **********************************

// Callback functions for telnet server
void onTelnetConnect(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(telnet.getIP());
    Serial.println(" connected");
    telnet.println("\n\rWelcome " + telnet.getIP() + "\r");
    telnet.println("\r");
    telnet.print(P1_MODULE_SOFTWARE_VERSION);
    telnet.println("\r");
    telnet.print(DATE_OFF_VERSION);
    telnet.println("\r");
    telnet.println("\r");
    telnet.println(ntpDateTimeStr);
    telnet.println("\r");
    telnet.println("Press q, followed by [Enter], to quit\r");
    telnet.println("\r");
}

void onTelnetDisconnect(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(ip);
    Serial.println(" disconnected");
}
void onTelnetReconnect(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(ip);
    Serial.println(" reconnected");
}

void onTelnetConnectionAttempt(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(telnet.getLastAttemptIP());
    Serial.println(" tried to connected");
}

void onInputRec(String aReceivedStr) {
    if ((aReceivedStr == "q") || (aReceivedStr == "Q")) {
        //disconnect the client
        telnet.disconnectClient();
    }
}

void setupTelnet() {
    if (telnet.begin()) {
        telnet.onConnect(onTelnetConnect);
        telnet.onConnectionAttempt(onTelnetConnectionAttempt);
        telnet.onReconnect(onTelnetReconnect);
        telnet.onDisconnect(onTelnetDisconnect);
        telnet.onInputReceived(onInputRec);
        Serial.println("- Telnet is running. Connect to Telnet using:");
        Serial.println("telnet " + ip_addr.toString());
    } else {
    // could not create server
        Serial.println("- Failed to create Telnet server. Is device connected to WiFi?");
        delay(10000);
        ESP.reset();
        delay(1000); 
    }
}

// **********************************
// * MQTT                           *
// **********************************

// * Send a message to a broker topic
void send_mqtt_message(const char *topic, const char *payload) {
    Serial.printf("MQTT Outgoing on %s: ", topic);
    Serial.println(payload);

    bool result = mqtt_client.publish(topic, payload, false);

    if (!result)
    {
        Serial.printf("MQTT publish to topic %s failed\n", topic);
    }
}

// * Reconnect to MQTT server and subscribe to in and out topics
bool mqtt_reconnect() {
    char myHostname[128];
    // make random hostname
    snprintf(myHostname, sizeof(myHostname), "%s-%d%d%d", HOSTNAME, rndNumber, rndNumber2, rndNumber3);

    // * Loop until we're reconnected
    int MQTT_RECONNECT_RETRIES = 0;

    while (!mqtt_client.connected() && MQTT_RECONNECT_RETRIES < MQTT_MAX_RECONNECT_TRIES)
    {
        MQTT_RECONNECT_RETRIES++;
        Serial.printf("MQTT connection attempt %d / %d ...\n", MQTT_RECONNECT_RETRIES, MQTT_MAX_RECONNECT_TRIES);
        snprintf(logMess, sizeof(logMess), "MQTT connection attempt %d / %d ...\n\r", MQTT_RECONNECT_RETRIES, MQTT_MAX_RECONNECT_TRIES);
        telnet.println(logMess);


        // * Attempt to connect
        if (mqtt_client.connect(myHostname, MQTT_USER, MQTT_PASS))
        {
            Serial.println(F("MQTT connected!"));
            telnet.println(F("MQTT connected!\r"));

            // * Once connected, publish an announcement...
            snprintf(logMess, sizeof(logMess), "p1 port module alive, hostname: %s", myHostname);
            mqtt_client.publish("Status", logMess);

            Serial.printf("MQTT root topic: %s\n", MQTT_ROOT_TOPIC);
            snprintf(logMess, sizeof(logMess), "MQTT root topic: %s\n\r", MQTT_ROOT_TOPIC);
            telnet.println(logMess);

        }
        else
        {
            Serial.print(F("MQTT Connection failed: rc="));
            Serial.println(mqtt_client.state());
            Serial.println(F(" Retrying in 2 seconds"));
            Serial.println("");
            snprintf(logMess, sizeof(logMess), "MQTT Connection failed: rc=%d\r", mqtt_client.state());
            telnet.println(logMess);
            telnet.println(F(" Retrying in 2 seconds\r"));
            telnet.println("\r");
            delay(2000);
        }
    }

    if (MQTT_RECONNECT_RETRIES >= MQTT_MAX_RECONNECT_TRIES)
    {
        Serial.printf("*** MQTT connection failed, giving up after %d tries ...\n", MQTT_RECONNECT_RETRIES);
        Serial.println(F(" Retrying in 10 seconds"));
        snprintf(logMess, sizeof(logMess), "*** MQTT connection failed, giving up after %d tries ...\n", MQTT_RECONNECT_RETRIES);
        telnet.println(logMess);
        telnet.println(F(" Retrying in 10 seconds\r"));
        return false;
    }

    return true;
}

void send_metric(const char * name, long metric)
{
    Serial.print(F("Sending metric to broker: "));
    Serial.print(name);
    Serial.print(F("="));
    Serial.println(metric);

    char output[22];
    ltoa(metric, output, 10);

    snprintf(topic, sizeof(topic), "%s/%s", MQTT_ROOT_TOPIC, name);
    send_mqtt_message(topic, output);
    yield();
}

void send_data_to_broker()
{
    send_metric("consumption_low_tarif", CONSUMPTION_LOW_TARIF);
    send_metric("consumption_high_tarif", CONSUMPTION_HIGH_TARIF);
    send_metric("delivery_low_tarif", DELIVERY_LOW_TARIF);
    send_metric("delivery_high_tarif", DELIVERY_HIGH_TARIF);
    send_metric("actual_consumption", ACTUAL_CONSUMPTION);
    send_metric("actual_delivery", ACTUAL_DELIVERY);
    ACTUAL_POWER = ACTUAL_CONSUMPTION - ACTUAL_DELIVERY;
    send_metric("actual_power", ACTUAL_POWER);
    send_metric("instant_power_usage", INSTANT_POWER_USAGE);
    send_metric("instant_power_current", INSTANT_POWER_CURRENT);
    send_metric("instant_voltage_l1", INSTANT_VOLTAGE_L1);
    send_metric("instant_voltage_l2", INSTANT_VOLTAGE_L2);
    send_metric("instant_voltage_l3", INSTANT_VOLTAGE_L3);
    send_metric("instant_current_l1", INSTANT_CURRENT_L1);
    send_metric("instant_current_l2", INSTANT_CURRENT_L2);
    send_metric("instant_current_l3", INSTANT_CURRENT_L3);
    send_metric("instant_active_power_consumption_l1", INSTANT_ACTIVE_POWER_CONSUMPTION_L1);
    send_metric("instant_active_power_consumption_l2", INSTANT_ACTIVE_POWER_CONSUMPTION_L2);
    send_metric("instant_active_power_consumption_l3", INSTANT_ACTIVE_POWER_CONSUMPTION_L3);
    send_metric("instant_active_power_delivery_l1", INSTANT_ACTIVE_POWER_DELIVERY_L1);
    send_metric("instant_active_power_delivery_l2", INSTANT_ACTIVE_POWER_DELIVERY_L2);
    send_metric("instant_active_power_delivery_l3", INSTANT_ACTIVE_POWER_DELIVERY_L3);
    ACTIVE_POWER_L1 = INSTANT_ACTIVE_POWER_CONSUMPTION_L1 - INSTANT_ACTIVE_POWER_DELIVERY_L1;
    ACTIVE_POWER_L2 = INSTANT_ACTIVE_POWER_CONSUMPTION_L2 - INSTANT_ACTIVE_POWER_DELIVERY_L2;
    ACTIVE_POWER_L3 = INSTANT_ACTIVE_POWER_CONSUMPTION_L3 - INSTANT_ACTIVE_POWER_DELIVERY_L3;
    if (INSTANT_VOLTAGE_L1 > 0) {
        ACTIVE_CURRENT_L1 = (long)round(((double)ACTIVE_POWER_L1 / ((double)INSTANT_VOLTAGE_L1 / 1000.0) * 1000.0)); 
    } else {
        ACTIVE_CURRENT_L1 = 0;
    }
    if (INSTANT_VOLTAGE_L2 > 0) {
        ACTIVE_CURRENT_L2 = (long)round(((double)ACTIVE_POWER_L2 / ((double)INSTANT_VOLTAGE_L2 / 1000.0) * 1000.0)); 
    } else {
        ACTIVE_CURRENT_L2 = 0;
    }
    if (INSTANT_VOLTAGE_L3 > 0) {
        ACTIVE_CURRENT_L3 = (long)round(((double)ACTIVE_POWER_L3 / ((double)INSTANT_VOLTAGE_L3 / 1000.0) * 1000.0)); 
    } else {
        ACTIVE_CURRENT_L3 = 0;
    }
    send_metric("active_power_l1", ACTIVE_POWER_L1);
    send_metric("active_power_l2", ACTIVE_POWER_L2);
    send_metric("active_power_l3", ACTIVE_POWER_L3);
    send_metric("active_current_l1", ACTIVE_CURRENT_L1);
    send_metric("active_current_l2", ACTIVE_CURRENT_L2);
    send_metric("active_current_l3", ACTIVE_CURRENT_L3);

    send_metric("gas_meter_m3", GAS_METER_M3);

    send_metric("actual_tarif_group", ACTUAL_TARIF);
    send_metric("short_power_outages", SHORT_POWER_OUTAGES);
    send_metric("long_power_outages", LONG_POWER_OUTAGES);
    send_metric("short_power_drops", SHORT_POWER_DROPS);
    send_metric("short_power_peaks", SHORT_POWER_PEAKS);
}

// **********************************
// * P1                             *
// **********************************

unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
	for (int pos = 0; pos < len; pos++)
    {
		crc ^= (unsigned int)buf[pos];    // * XOR byte into least sig. byte of crc
                                          // * Loop over each bit
        for (int i = 8; i != 0; i--)
        {
            // * If the LSB is set
            if ((crc & 0x0001) != 0)
            {
                // * Shift right and XOR 0xA001
                crc >>= 1;
				crc ^= 0xA001;
			}
            // * Else LSB is not set
            else
                // * Just shift right
                crc >>= 1;
		}
	}
	return crc;
}

bool isNumber(char *res, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
            return false;
    }
    return true;
}

int FindCharInArrayRev(char array[], char c, int len)
{
    for (int i = len - 1; i >= 0; i--)
    {
        if (array[i] == c)
            return i;
    }
    return -1;
}

long getValue(char *buffer, int maxlen, char startchar, char endchar)
{
    int s = FindCharInArrayRev(buffer, startchar, maxlen - 2);
    int l = FindCharInArrayRev(buffer, endchar, maxlen - 2) - s - 1;

    char res[16];
    memset(res, 0, sizeof(res));

    if (strncpy(res, buffer + s + 1, l))
    {
        if (endchar == '*')
        {
            if (isNumber(res, l))
                // * Lazy convert float to long
                return (1000 * atof(res));
        }
        else if (endchar == ')')
        {
            if (isNumber(res, l))
                return atof(res);
        }
    }
    return 0;
}

void getDateTimeP1(char *buffer, int maxlen, char startchar, char endchar)
{
    char tmpStr[10];
    int s = FindCharInArrayRev(buffer, startchar, maxlen);
    int l = FindCharInArrayRev(buffer, endchar, maxlen) - s - 1;
    
    memset(DATE_TIME_P1, 0, sizeof(DATE_TIME_P1));

    if (strncpy(DATE_TIME_P1, buffer + s + 1, l))
    {
        DATE_TIME_P1[l] = 0;
        if (l == 13) {
            tmpStr[0] = DATE_TIME_P1[6];
            tmpStr[1] = DATE_TIME_P1[7];
            tmpStr[2] = 0;
            P1_HOURS = atoi(tmpStr);
            tmpStr[0] = DATE_TIME_P1[8];
            tmpStr[1] = DATE_TIME_P1[9];
            tmpStr[2] = 0;
            P1_MINUTES = atoi(tmpStr);
        }
    } else {
        DATE_TIME_P1[0] = 0;
        P1_HOURS = -1;
        P1_MINUTES = -1;
    }
}

void getDateTimeNTP(void) {
    int year = ntp.year();
    int month = ntp.month();
    int day = ntp.day();
    int seconds = ntp.seconds();
    
    year = year - 2000;
    P1_HOURS = ntp.hours();
    P1_MINUTES = ntp.minutes();
    if (ntp.isDST() == 0) {
        snprintf(DATE_TIME_P1, sizeof(DATE_TIME_P1), "%02d%02d%02d%02d%02d%02d%c", year, month, day, P1_HOURS, P1_MINUTES, seconds, 'W');
    } else {
        snprintf(DATE_TIME_P1, sizeof(DATE_TIME_P1), "%02d%02d%02d%02d%02d%02d%c", year, month, day, P1_HOURS, P1_MINUTES, seconds, 'S');
    }
#ifdef TEST_DSMRV2_2    
    telnet.println(DATE_TIME_P1);
#endif    
}

bool decode_telegram(int len)
{
    int startChar = FindCharInArrayRev(telegram, '/', len);
    int endChar = FindCharInArrayRev(telegram, '!', len);
    bool validCRCFound = false;
    bool messageComplete = false;
    int cnt;

    for (cnt = 0; cnt < len; cnt++) {
        Serial.print(telegram[cnt]);
#ifdef DEBUGIT        
        logMess[cnt] = telegram[cnt];
#endif        
    }
    Serial.println("\r");
#ifdef DEBUGIT        
    logMess[cnt] = 0;
    telnet.println(logMess);
#endif    
    if (DSMRV2_2IsUsed) {
        if (endChar >= 0) {
            messageComplete = true;
            // In DSMRV2.2 no timestamp is given, but check anyhow
            if (!timeStampReceived) {
                // use NTP time instead
                getDateTimeNTP();
            }
        }
    } else {

        if (startChar >= 0) {
            // * Start found. Reset CRC calculation
            currentCRC = CRC16(0x0000, (unsigned char *)telegram + startChar, len - startChar);
        } else if (endChar >= 0) {
            // In DSMR V4 and higher normaly a timestamp is given, but for test purposes it is not used
            if (!timeStampReceived) {
                // use NTP time instead
                getDateTimeNTP();
            }

            // * Add to crc calc
            currentCRC = CRC16(currentCRC, (unsigned char*)telegram + endChar, 1);

            char messageCRC[5];
            strncpy(messageCRC, telegram + endChar + 1, 4);

            messageCRC[4] = 0;   // * Thanks to HarmOtten (issue 5)
            validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);

            if (validCRCFound) {
                Serial.println(F("CRC Valid!\r"));
                VALID_MESSAGES++;
#ifdef DEBUGIT        
                telnet.println(F("CRC Valid!\r"));
#endif            
                messageComplete = true;
            } else {
                Serial.println(F("CRC Invalid!\r"));
                INVALID_MESSAGES++;
                snprintf(logMess, sizeof(logMess), "CRC Invalid! [total = %d]\r", INVALID_MESSAGES);
                telnet.println(logMess);
                messageComplete = false;
            }

            currentCRC = 0;
        } else {
            currentCRC = CRC16(currentCRC, (unsigned char*) telegram, len);
        }
    }

    // 0-0:1.0.0.255(201113173500W)
    // 0-0:1.0.0.255 = Date - time stamp P1 message
    if (strncmp(telegram, "0-0:1.0.0", strlen("0-0:1.0.0")) == 0) {
        timeStampReceived = true;
#ifdef TEST_DSMRV2_2
        timeStampReceived = false;
        return messageComplete;
#endif        
        // get DATE_TIME_P1
        getDateTimeP1(telegram, len, '(', ')');
        return messageComplete;
    }

    // 1-0:1.8.1(000992.992*kWh)
    // 1-0:1.8.1 = Elektra verbruik laag tarief (DSMR v5.0.2)
    if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0) {
        CONSUMPTION_LOW_TARIF = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:1.8.2(000560.157*kWh)
    // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v5.0.2)
    if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0) {
        CONSUMPTION_HIGH_TARIF = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:2.8.1(000000.000*kWh)
    // 1-0:2.8.1 = Elektra levering laag tarief (DSMR v5.0.2)
    if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0) {
        DELIVERY_LOW_TARIF = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:2.8.2(000000.000*kWh)
    // 1-0:2.8.2 = Elektra levering hoog tarief (DSMR v5.0.2)
    if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0) {
        DELIVERY_HIGH_TARIF = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:1.7.0(00.000*kW) Actueel verbruik
    // 1-0:1.7.0 = Electricity consumption actual usage (DSMR v5.0.2)
    if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0) {
        ACTUAL_CONSUMPTION = getValue(telegram, len, '(', '*');
        if (ACTUAL_CONSUMPTION > PEAK_CONSUMPTION) {
            PEAK_CONSUMPTION = ACTUAL_CONSUMPTION;
        }
        return messageComplete;
    }

    // 1-0:2.7.0(00.000*kW) Actuele teruglevering
    // 1-0:2.7.0 = Electricity consumption actual delivery (DSMR v5.0.2)
    if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0) {
        ACTUAL_DELIVERY = getValue(telegram, len, '(', '*');
        if (ACTUAL_DELIVERY > PEAK_DELIVERY) {
            PEAK_DELIVERY = ACTUAL_DELIVERY;
        }
        return messageComplete;
    }

    // 0-1:24.2.1(150531200000S)(00811.923*m3)
    // 0-1:24.2.1 = Gas (DSMR v5.0.2)
    if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0) {
        GAS_METER_M3 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 0-0:96.14.0(0001)
    // 0-0:96.14.0 = Actual Tarif
    if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0) {
        ACTUAL_TARIF = getValue(telegram, len, '(', ')');
        return messageComplete;
    }

    // 0-0:96.7.21(00003)
    // 0-0:96.7.21 = Aantal onderbrekingen Elektriciteit
    if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0) {
        SHORT_POWER_OUTAGES = getValue(telegram, len, '(', ')');
        return messageComplete;
    }

    // 0-0:96.7.9(00001)
    // 0-0:96.7.9 = Aantal lange onderbrekingen Elektriciteit
    if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0) {
        LONG_POWER_OUTAGES = getValue(telegram, len, '(', ')');
        return messageComplete;
    }

    // 1-0:32.32.0(00000)
    // 1-0:32.32.0 = Aantal korte spanningsdalingen Elektriciteit in fase 1
    if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0) {
        SHORT_POWER_DROPS = getValue(telegram, len, '(', ')');
        return messageComplete;
    }

    // 1-0:32.36.0(00000)
    // 1-0:32.36.0 = Aantal korte spanningsstijgingen Elektriciteit in fase 1
    if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0) {
        SHORT_POWER_PEAKS = getValue(telegram, len, '(', ')');
        return messageComplete;
    }

    // 1-0:32.7.0(000.0*V)
    // 1-0:32.7.0 = Instantaneous voltage L1 in V resolution
    if (strncmp(telegram, "1-0:32.7.0", strlen("1-0:32.7.0")) == 0) {
        L1_DETECTED = true;
        INSTANT_VOLTAGE_L1 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:52.7.0(000.0*V)
    // 1-0:52.7.0 = Instantaneous voltage L2 in V resolution
    if (strncmp(telegram, "1-0:52.7.0", strlen("1-0:52.7.0")) == 0) {
        L2_DETECTED = true;
        INSTANT_VOLTAGE_L2 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:72.7.0(000.0*V)
    // 1-0:72.7.0 = Instantaneous voltage L3 in V resolution
    if (strncmp(telegram, "1-0:72.7.0", strlen("1-0:72.7.0")) == 0) {
        L3_DETECTED = true;
        INSTANT_VOLTAGE_L3 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:31.7.0(000*A)
    // 1-0:31.7.0 = Instantaneous current L1 in A resolution
    if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0) {
        L1_DETECTED = true;
        INSTANT_CURRENT_L1 = getValue(telegram, len, '(', '*');
        INSTANT_POWER_CURRENT = INSTANT_CURRENT_L1;
        return messageComplete;
    }

    // 1-0:51.7.0(000*A)
    // 1-0:51.7.0 = Instantaneous current L2 in A resolution
    if (strncmp(telegram, "1-0:51.7.0", strlen("1-0:51.7.0")) == 0) {
        L2_DETECTED = true;
        INSTANT_CURRENT_L2 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:71.7.0(000*A)
    // 1-0:71.7.0 = Instantaneous current L3 in A resolution
    if (strncmp(telegram, "1-0:71.7.0", strlen("1-0:71.7.0")) == 0) {
        L3_DETECTED = true;
        INSTANT_CURRENT_L3 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:21.7.0(00.000*kW)
    // 1-0:21.7.0 = Actuele verbruik L1
    if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0) {
        L1_DETECTED = true;
        INSTANT_ACTIVE_POWER_CONSUMPTION_L1 = getValue(telegram, len, '(', '*');
        INSTANT_POWER_USAGE = INSTANT_ACTIVE_POWER_CONSUMPTION_L1;
        return messageComplete;
    }

    // 1-0:41.7.0(00.000*kW)
    // 1-0:41.7.0 = Actuele verbruik L2
    if (strncmp(telegram, "1-0:41.7.0", strlen("1-0:41.7.0")) == 0) {
        L2_DETECTED = true;
        INSTANT_ACTIVE_POWER_CONSUMPTION_L2 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:61.7.0(00.000*kW)
    // 1-0:61.7.0 = Actuele verbruik L3
    if (strncmp(telegram, "1-0:61.7.0", strlen("1-0:61.7.0")) == 0) {
        L3_DETECTED = true;
        INSTANT_ACTIVE_POWER_CONSUMPTION_L3 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:22.7.0(00.000*kW)
    // 1-0:22.7.0 = Actuele opbrengst L1
    if (strncmp(telegram, "1-0:22.7.0", strlen("1-0:22.7.0")) == 0) {
        L1_DETECTED = true;
        INSTANT_ACTIVE_POWER_DELIVERY_L1 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:42.7.0(00.000*kW)
    // 1-0:42.7.0 = Actuel opbrengst L2
    if (strncmp(telegram, "1-0:42.7.0", strlen("1-0:42.7.0")) == 0) {
        L2_DETECTED = true;
        INSTANT_ACTIVE_POWER_DELIVERY_L2 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    // 1-0:62.7.0(00.000*kW)
    // 1-0:62.7.0 = Actuele opbrengst L3
    if (strncmp(telegram, "1-0:62.7.0", strlen("1-0:62.7.0")) == 0) {
        L3_DETECTED = true;
        INSTANT_ACTIVE_POWER_DELIVERY_L3 = getValue(telegram, len, '(', '*');
        return messageComplete;
    }

    return messageComplete;
}

void read_p1_serial()
{
    if (Serial.available())
    {
#ifdef DEBUGIT
        char line[256];
#endif
        memset(telegram, 0, sizeof(telegram));

        while (Serial.available())
        {
            ESP.wdtDisable();
            int len = Serial.readBytesUntil('\n', telegram, P1_MAXLINELENGTH);
            ESP.wdtEnable(1);
            telegram[len] = '\n';
            telegram[len + 1] = 0;
#ifdef DEBUGIT
            snprintf(line, sizeof(line), "New telegram [%d] = ", len);
            telnet.print(line);
            telnet.println(telegram);
            telnet.print("\r");
#endif

            yield();

            bool result = decode_telegram(len + 1);
            if (result) {
                if (firstStart) {
                    measureStartTime = millis();
                    INVALID_MESSAGES = 0;
                    VALID_MESSAGES = 1;
                    firstStart = false;
                    PREVIOUS_P1_HOURS = P1_HOURS;
                    PREVIOUS_CONSUMPTION_LOW_TARIF = CONSUMPTION_LOW_TARIF;
                    PREVIOUS_CONSUMPTION_HIGH_TARIF = CONSUMPTION_HIGH_TARIF;
                    PREVIOUS_DELIVERY_LOW_TARIF = DELIVERY_LOW_TARIF;
                    PREVIOUS_DELIVERY_HIGH_TARIF = DELIVERY_HIGH_TARIF;
                }

                if (L1_DETECTED) {
                    SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L1 = SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L1 + INSTANT_ACTIVE_POWER_CONSUMPTION_L1;
                    SUM_INSTANT_ACTIVE_POWER_DELIVERY_L1 = SUM_INSTANT_ACTIVE_POWER_DELIVERY_L1 + INSTANT_ACTIVE_POWER_DELIVERY_L1;
                }
                if (L2_DETECTED) {
                    SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L2 = SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L2 + INSTANT_ACTIVE_POWER_CONSUMPTION_L2;
                    SUM_INSTANT_ACTIVE_POWER_DELIVERY_L2 = SUM_INSTANT_ACTIVE_POWER_DELIVERY_L2 + INSTANT_ACTIVE_POWER_DELIVERY_L2;
                }
                if (L3_DETECTED) {
                    SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L3 = SUM_INSTANT_ACTIVE_POWER_CONSUMPTION_L3 + INSTANT_ACTIVE_POWER_CONSUMPTION_L3;
                    SUM_INSTANT_ACTIVE_POWER_DELIVERY_L3 = SUM_INSTANT_ACTIVE_POWER_DELIVERY_L3 + INSTANT_ACTIVE_POWER_DELIVERY_L3;
                }

                send_data_to_broker();
            }
        }
    }
}

// **********************************
// * EEPROM helpers                 *
// **********************************

String read_eeprom(int offset, int len)
{
    Serial.print(F("read_eeprom()"));

    String res = "";
    for (int i = 0; i < len; ++i)
    {
        res += char(EEPROM.read(i + offset));
    }
    return res;
}

void write_eeprom(int offset, int len, String value)
{
    Serial.println(F("write_eeprom()"));
    for (int i = 0; i < len; ++i)
    {
        if ((unsigned)i < value.length())
        {
            EEPROM.write(i + offset, value[i]);
        }
        else
        {
            EEPROM.write(i + offset, 0);
        }
    }
}

// **********************************
// * WiFi                           *
// **********************************

// WiFiClient espClient;
// PubSubClient mqtt_client(espClient);

// WiFiUDP wifiUDP;

// * Gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println(F("Entered config mode"));
  Serial.println(WiFi.softAPIP());

  // * If you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

// * Callback for saving WIFI config
bool shouldSaveConfig = false;

// * Callback notifying us of the need to save config
void save_wifi_config_callback () {
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

// * Setup WiFi function
void setup_WiFi() {
  if (USE_DEFAULT_WIFI_SETTINGS) {
    // to reset the WiFi credentials, uncomment next line:
    // WiFi.disconnect(true);

    WiFi.begin(WIFI_NETWORK, WIFI_PASSWD);
    // Try up to del seconds to get a WiFi connection; and if that fails,setup AP
    // with a bit of a delay.
    //
    const int del = WIFI_TIMEOUT; // milliseconds.
    unsigned long start = millis();
    Serial.print("Connecting..");
    while ((WiFi.status() != WL_CONNECTED) && (millis() - start < del)) {
      delay(500);
      Serial.print(",");
    };
    Serial.print("\n\r");
  }

  if (!USE_DEFAULT_WIFI_SETTINGS || (WiFi.status() != WL_CONNECTED)) {
    // * WiFiManager local initialization. Once its business is done, there is no need to keep it around

    // * Get MQTT Server settings
    String settings_available = read_eeprom(134, 1);

    if (settings_available == "1") {
        read_eeprom(0, 64).toCharArray(MQTT_HOST, 64);   // * 0-63
        read_eeprom(64, 6).toCharArray(MQTT_PORT, 6);    // * 64-69
        read_eeprom(70, 32).toCharArray(MQTT_USER, 32);  // * 70-101
        read_eeprom(102, 32).toCharArray(MQTT_PASS, 32); // * 102-133
    }

    WiFiManagerParameter CUSTOM_MQTT_HOST("host", "MQTT hostname", MQTT_HOST, 64);
    WiFiManagerParameter CUSTOM_MQTT_PORT("port", "MQTT port",     MQTT_PORT, 6);
    WiFiManagerParameter CUSTOM_MQTT_USER("user", "MQTT user",     MQTT_USER, 32);
    WiFiManagerParameter CUSTOM_MQTT_PASS("pass", "MQTT pass",     MQTT_PASS, 32);

    WiFiManager wifiManager;
    // * Reset settings - uncomment for testing
    // wifiManager.resetSettings();


    // * Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);

    // * Set timeout
    wifiManager.setConfigPortalTimeout(WIFI_MANAGER_TIMEOUT);

    // * Set save config callback
    wifiManager.setSaveConfigCallback(save_wifi_config_callback);

        // * Add all your parameters here
    wifiManager.addParameter(&CUSTOM_MQTT_HOST);
    wifiManager.addParameter(&CUSTOM_MQTT_PORT);
    wifiManager.addParameter(&CUSTOM_MQTT_USER);
    wifiManager.addParameter(&CUSTOM_MQTT_PASS);

    wifiManager.setTimeout(WIFI_MANAGER_TIMEOUT);

    // * Fetches SSID and pass and tries to connect
    // * Reset when no connection after 10 seconds
    if (!wifiManager.autoConnect(FALLBACK_WIFI_NETWORK, FALLBACK_WIFI_PASSWD)) {
      Serial.printf("No connection after %d seconds (ssid=%s). Rebooting.\n\r", WIFI_TIMEOUT, WiFi.SSID().c_str());
      Serial.print("Rebooting...\n\r");
      delay(1000);
    // * Reset and try again, or maybe put it to deep sleep
      ESP.reset();
    }
    // WiFi connection established
    // * Save the custom parameters to FS
    // * Read updated parameters
    snprintf(MQTT_HOST, sizeof(MQTT_HOST), "%s", CUSTOM_MQTT_HOST.getValue());
    snprintf(MQTT_PORT, sizeof(MQTT_PORT), "%s", CUSTOM_MQTT_PORT.getValue());
    snprintf(MQTT_USER, sizeof(MQTT_USER), "%s", CUSTOM_MQTT_USER.getValue());
    snprintf(MQTT_PASS, sizeof(MQTT_PASS), "%s", CUSTOM_MQTT_PASS.getValue());

    // * Save the custom parameters to FS
    if (shouldSaveConfig) {
      Serial.println(F("Saving WiFiManager config"));

      write_eeprom(0, 64, MQTT_HOST);   // * 0-63
      write_eeprom(64, 6, MQTT_PORT);   // * 64-69
      write_eeprom(70, 32, MQTT_USER);  // * 70-101
      write_eeprom(102, 32, MQTT_PASS); // * 102-133
      write_eeprom(134, 1, "1");        // * 134 --> always "1"
      EEPROM.commit();
    }
  }

  // * If you get here you have connected to the WiFi
  Serial.println(F("Connected to WIFI..."));
  if(WiFi.SSID().c_str()) {
    Serial.printf("SSID = <%s>\n\r", WiFi.SSID().c_str());
  }

  ip_addr = WiFi.localIP();

  // * If you get here you have connected to the WiFi
  snprintf(logMess, sizeof(logMess), "IP address = %s\n\r", ip_addr.toString().c_str());
  Serial.print(logMess);

}

// **********************************
// * Setup OTA                      *
// **********************************

void setup_ota()
{
    Serial.println(F("Arduino OTA activated."));

    // * Port defaults to 8266
    ArduinoOTA.setPort(8266);

    // * Set hostname for OTA
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]()
    {
        Serial.println(F("Arduino OTA: Start"));
    });

    ArduinoOTA.onEnd([]()
    {
        Serial.println(F("Arduino OTA: End (Running reboot)"));
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        Serial.printf("Arduino OTA Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Serial.printf("Arduino OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Serial.println(F("Arduino OTA: Auth Failed"));
        else if (error == OTA_BEGIN_ERROR)
            Serial.println(F("Arduino OTA: Begin Failed"));
        else if (error == OTA_CONNECT_ERROR)
            Serial.println(F("Arduino OTA: Connect Failed"));
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println(F("Arduino OTA: Receive Failed"));
        else if (error == OTA_END_ERROR)
            Serial.println(F("Arduino OTA: End Failed"));
    });

    ArduinoOTA.begin();
    Serial.println(F("Arduino OTA finished"));
}

// **********************************
// * Setup MDNS discovery service   *
// **********************************

void setup_mdns()
{
    Serial.println(F("Starting MDNS responder service"));

    bool mdns_result = MDNS.begin(HOSTNAME);
    if (mdns_result)
    {
        MDNS.addService("http", "tcp", 80);
    }
}

// **********************************
// * Setup Main                     *
// **********************************

void setup()
{
    // * Configure Serial
    pinMode(PIN_DSMR_V2_2, INPUT_PULLUP);
    if (digitalRead(PIN_DSMR_V2_2) == HIGH) {
        Serial.begin(BAUD_RATE); // Setting for DSMR V2.2 jumper not installed
        DSMRV2_2IsUsed = false;
    } else {
        Serial.begin(BAUD_RATE_DSMR_2_2, SERIAL_7E1);
        DSMRV2_2IsUsed = true;
    }

    Serial.printf("\n\r\n\r");
    Serial.print(P1_MODULE_SOFTWARE_VERSION);
    Serial.println("\r");
    Serial.printf("\n\r\n\rSerial port = %d b/s\n\r", Serial.baudRate());

    // generate random numberfor MQTT Id
    srand(time(NULL));
    rndNumber = rand();
    rndNumber2 = rand();
    rndNumber3 = rand();

    // * Configure EEPROM
    EEPROM.begin(512);
    
    // * Set led pin as output
    pinMode(LED_BUILTIN, OUTPUT);

    // * Start ticker with 0.5 because we start in AP mode and try to connect
    ticker.attach(0.5, tick);


    // to reset the WiFi credentials, uncomment next line:
    // WiFi.disconnect(true);

    setup_WiFi();

    ip_addr = WiFi.localIP();

    // * Keep LED on
    ticker.detach();
    digitalWrite(LED_BUILTIN, LOW);

    setupTelnet();

    // * Start ticker with 1.0 after connection
    // ticker.attach(1.0, tick);

    // * Configure OTA
    setup_ota();

    // * Startup MDNS Service
    setup_mdns();

    // * Setup MQTT
    Serial.printf("MQTT connecting to: %s:%s\n", MQTT_HOST, MQTT_PORT);

    mqtt_client.setServer(MQTT_HOST, atoi(MQTT_PORT));

    initNTP();
}


void ntpUpdateLoop(void) {
    if ((millis() - lastNTPUpdateTime) > NTP_UPDATE_WINDOW) {
        lastNTPUpdateTime = millis();
        if (!ntp.update()) { // error updating ntp time
            snprintf(logMess, sizeof(logMess), "Error: unable to update NTP time!\n\r");
            Serial.print(logMess);
            telnet.print(logMess);
        }
    }
}

// **********************************
// * Loop                           *
// **********************************

unsigned long timerms = 0;

char telnetMess[255];

bool firstTime = true;

unsigned long now;

bool MQTTFirstTime = true;

void loop()
{
    read_p1_serial();

    ArduinoOTA.handle();

    telnet.loop();

    ntpUpdateLoop();

    if (!mqtt_client.connected()) {
        now = millis();

        if (((now - LAST_RECONNECT_ATTEMPT) > 30000) || MQTTFirstTime) {
            MQTTFirstTime = false;
            LAST_RECONNECT_ATTEMPT = now;
            Serial.println("mqtt client not connected\r");
            telnet.println("mqtt client not connected\r");

            if (mqtt_reconnect())
            {
                MQTTFirstTime = true;
            }
        }
    } else {
        if (firstTime) {
            Serial.println("First time = true \r");
            firstTime = false;
            snprintf(logMess, sizeof(logMess), "p1 port module alive, hostname: %s", HOSTNAME);
            mqtt_client.publish("Status", logMess);
            telnet.print(logMess);
            telnet.print("\n\r");
        }
    }
    mqtt_client.loop();
    yield();
}
