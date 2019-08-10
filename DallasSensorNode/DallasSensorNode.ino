#include <EspNowFloodingMesh.h>
#include<SimpleMqtt.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/********NODE SETUP********/
#define ESP_NOW_CHANNEL 1
const char deviceName[] = "dallasOlohuone1";
int bsid = 0x112233;
unsigned char secredKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
unsigned char iv[16] = {0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};;
const int ttl = 3;
/*****************************/

#define DS18B20_PIN 2

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
SimpleMQTT simpleMqtt = SimpleMQTT(ttl, deviceName);

void deepSleepMode(){
  ESP.deepSleep(60*1000*1000); //Sleep 1min
}

void setup() {
  Serial.begin(115200);

  espNowFloodingMesh_secredkey(secredKey);
  espNowFloodingMesh_setAesInitializationVector(iv);
  espNowFloodingMesh_setToMasterRole(false, ttl);
  espNowFloodingMesh_setToBatteryNode();
  espNowFloodingMesh_begin(ESP_NOW_CHANNEL, bsid);

  espNowFloodingMesh_ErrorDebugCB([](int level, const char *str) {
    Serial.print(level); Serial.println(str); //If you want print some debug prints
  });


  if (!espNowFloodingMesh_syncWithMasterAndWait()) {
    //Sync failed??? No connection to master????
    Serial.println("No connection to master!!! Reboot");
    deepSleepMode(); //Perhaps this works in the next time. Let's go to sleep
  }
}

float getTemperature() {
  static DeviceAddress insideThermometer;
  static bool init = true;
  if(init){
    if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");
    init = false;
    sensors.setResolution(insideThermometer, 9);
  }
  
  return sensors.getTempC(insideThermometer);
}

void loop() {
  float temperature = getTemperature();
  if (!simpleMqtt._temp(PUBLISH, "tempSensor", temperature)) { //Publish topic dallasOlohuone1/temp/tempSensor/value
    Serial.println("Publish failed... Reboot");
    deepSleepMode();
  }
  espNowFloodingMesh_loop();
  deepSleepMode();
}
