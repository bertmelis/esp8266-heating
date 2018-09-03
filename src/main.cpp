/*
    copyright 2018 Bert Melis
    License: MIT
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <VitoWiFi.h>
#include <TreeLight.h>

static const char SSID[] = "xxxx";
static const char PASS[] = "xxxx";
static const IPAddress BROKER(192, 168, 1, 2);
static const uint16_t PORT =  1883;

Ticker VitoWiFiTimer;
Ticker StatsTimer;
volatile bool updateVitoWiFi = false;
volatile bool updateStats = false;

// datapoint names and groups list
const char outsideTempName[] = "outsideTemp";
const char boilerTempName[] = "boilerTemp";
const char dhwTempName[] = "dhwTemp";
const char dhwSolerTempName[] = "dhwSolarTemp";
const char dhwSolarCollTempName[] = "dhwSolarCollTemp";
const char solarPumpStatName[] = "solarPumpStat";
const char solarPumpHoursName[] = "solarPumpHours";
const char boilerGroup[] = "boiler";
const char hotwaterGroup[] = "hotwater";

// VitoWiFi initialization - VitoDens 2xx with Vitotronic 200
VitoWiFi_setProtocol(P300);  // this also initializes VitoWiFi.
DPTemp outsideTemp(outsideTempName, boilerGroup, 0x5525);
DPTemp boilerTemp(boilerTempName, boilerGroup, 0x0810);
DPTemp dhwTemp(dhwTempName, hotwaterGroup, 0x0812);
DPTemp dhwSolarTemp(dhwSolerTempName, hotwaterGroup, 0x6566);
DPTemp dhwSolarCollTemp(dhwSolarCollTempName, hotwaterGroup, 0x6564);
DPStat solarPumpStat(solarPumpStatName, hotwaterGroup, 0x6552);
DPCountS solarPumpHours(solarPumpHoursName, hotwaterGroup, 0x6568);

// TreeLightNodes initialization
TreeLightNode boilerTempNode(boilerTempName, false, NUMBER);
TreeLightNode outsideTempNode(outsideTempName, false, NUMBER);
TreeLightNode dhwTempNode(dhwTempName, false, NUMBER);
TreeLightNode dhwSolarTempNode(dhwSolerTempName, false, NUMBER);
TreeLightNode dhwSolarCollTempNode(dhwSolarCollTempName, false, NUMBER);
TreeLightNode solarPumpStatNode(solarPumpStatName, false, NUMBER);
TreeLightNode solarPumpHoursNode(solarPumpHoursName, false, NUMBER);

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  TreeLightNode* n = TreeLight.findNode(dp.getName());
  if (n) {
    char str[9] = {"\0"};
    value.getString(str, sizeof(str));
    TreeLight.setNode(*n, str);
  }
}

void setup() {
  VitoWiFi.setGlobalCallback(globalCallbackHandler);
  VitoWiFi.setLogger(&TreeLight);
  VitoWiFi.enableLogger();
  VitoWiFi.setup(&Serial);

  VitoWiFiTimer.attach(60, [](){
    updateVitoWiFi = true;
  });
  StatsTimer.attach(10, [](){
    updateStats = true;
  });

  TreeLight.setHostname("HEATING");
  TreeLight.setupWiFi(SSID, PASS);
  TreeLight.setupServer(80);
  TreeLight.setupMqtt(BROKER, PORT);
  TreeLight.begin();
}

void loop() {
  VitoWiFi.loop();
  TreeLight.loop();
  if (updateVitoWiFi) {
    updateVitoWiFi = false;
    VitoWiFi.readAll();
  }
  if (updateStats) {
    updateStats = false;
    TreeLight.updateStats();
  }
}
