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
volatile bool updateVitoWiFi = false;
#if USE_STATS
Ticker StatsTimer;
#endif
volatile bool updateStats = false;

// datapoint names and groups list
const char outsideTempName[] = "outsideTemp";
const char boilerTempName[] = "boilerTemp";
const char dhwTempName[] = "dhwTemp";
const char dhwSolarTempName[] = "dhwSolarTemp";
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
DPTemp dhwSolarTemp(dhwSolarTempName, hotwaterGroup, 0x6566);
DPTemp dhwSolarCollTemp(dhwSolarCollTempName, hotwaterGroup, 0x6564);
DPStat solarPumpStat(solarPumpStatName, hotwaterGroup, 0x6552);
DPCountS solarPumpHours(solarPumpHoursName, hotwaterGroup, 0x6568);

// TreeLightNodes initialization
FloatNode outsideTempNode(outsideTempName, false);
FloatNode boilerTempNode(boilerTempName, false);
FloatNode dhwTempNode(dhwTempName, false);
FloatNode dhwSolarTempNode(dhwSolarTempName, false);
FloatNode dhwSolarCollTempNode(dhwSolarCollTempName, false);
BoolNode solarPumpStatNode(solarPumpStatName, false);
IntNode solarPumpHoursNode(solarPumpHoursName, false);

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  char str[9] = {"\0"};
  value.getString(str, sizeof(str));
  TreeLight.printf("node %s: %str\n", dp.getName(), str);
}

void setup() {
  VitoWiFi.setGlobalCallback(globalCallbackHandler);
  VitoWiFi.setLogger(&TreeLight);
  VitoWiFi.enableLogger();
  VitoWiFi.setup(&Serial);

  VitoWiFiTimer.attach(60, [](){
    updateVitoWiFi = true;
  });

  outsideTemp.setCallback([](const IDatapoint& dp, DPValue value){
    outsideTempNode.setValue(value.getFloat());
  });
  boilerTemp.setCallback([](const IDatapoint& dp, DPValue value){
    boilerTempNode.setValue(value.getFloat());
  });
  dhwTemp.setCallback([](const IDatapoint& dp, DPValue value){
    dhwTempNode.setValue(value.getFloat());
  });
  dhwSolarTemp.setCallback([](const IDatapoint& dp, DPValue value){
    dhwSolarTempNode.setValue(value.getFloat());
  });
  dhwSolarCollTemp.setCallback([](const IDatapoint& dp, DPValue value){
    dhwSolarCollTempNode.setValue(value.getFloat());
  });
  solarPumpStat.setCallback([](const IDatapoint& dp, DPValue value){
    solarPumpStatNode.setValue(value.getBool());
  });
  solarPumpHours.setCallback([](const IDatapoint& dp, DPValue value){
    solarPumpHoursNode.setValue(value.getU16());
  });

#if USE_STATS
  StatsTimer.attach(60, [](){
    updateStats = true;
  });
#endif

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
#if USE_STATS
  if (updateStats) {
    updateStats = false;
    TreeLight.updateStats();
  }
#endif
}
