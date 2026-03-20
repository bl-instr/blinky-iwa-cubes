#define BLINKY_DIAG         0
#define CUBE_DIAG           0
#define COMM_LED_PIN       16
#define RST_BUTTON_PIN     15
#include <BlinkyPicoW.h>

struct CubeSetting
{
  int16_t authorizeBadge;
  int16_t sendWarning;
};
CubeSetting setting;
CubeSetting oldSetting;

struct CubeReading
{
  int16_t iaddr;
  int16_t ivbat;
  int16_t ivusb;
  int16_t iwatchdog;
  int16_t ibutt;
  int16_t imove;
  int16_t ichrg;
  int16_t iauth;
  int16_t irssi;
};
CubeReading reading;

struct StationCommand
{
  int16_t iaddr;
  int16_t iwarn;
  int16_t iauth;
  int16_t imsid;
};
StationCommand stationCommand;

unsigned long lastPublishTime;
unsigned long publishInterval = 30000;

void setupBlinky()
{
  if (BLINKY_DIAG > 0)
  {
    Serial.begin(9600);
    delay(5000);
  }
  
  boolean  useFlashStorage = true;
  
/*  
  BlinkyPicoW.setSsid("");
  BlinkyPicoW.setWifiPassword("");
  BlinkyPicoW.setMqttServer("");
  BlinkyPicoW.setMqttUsername("");
  BlinkyPicoW.setMqttPassword("");
  BlinkyPicoW.setBox("");
  BlinkyPicoW.setTrayType("");
  BlinkyPicoW.setTrayName("");
  BlinkyPicoW.setCubeType("");
*/ 
 
  BlinkyPicoW.setMqttKeepAlive(15);
  BlinkyPicoW.setMqttSocketTimeout(4);
  BlinkyPicoW.setMqttPort(1883);
  BlinkyPicoW.setMqttLedFlashMs(100);
  BlinkyPicoW.setHdwrWatchdogMs(8000);
  BlinkyPicoW.setRouterDelay(10000);

  BlinkyPicoW.begin(BLINKY_DIAG, COMM_LED_PIN, RST_BUTTON_PIN, useFlashStorage, sizeof(setting), sizeof(reading));
}

void setupCube()
{
  if ((CUBE_DIAG > 0) &&(BLINKY_DIAG == 0 ))
  {
    Serial.begin(9600);
    delay(5000);
  }
  setting.authorizeBadge = 0;
  setting.sendWarning = 0;
  oldSetting.authorizeBadge = 0; 
  oldSetting.sendWarning = 0; 
  Serial1.begin(9600);
  lastPublishTime = millis(); 
}
void loopCube()
{
  unsigned long now = millis();
  if ((now - lastPublishTime) > publishInterval)
  {
    lastPublishTime = now;
    reading.iaddr = 0;
    reading.ivbat = 0;
    reading.ivusb = 0;
    reading.ibutt = 0;
    reading.imove = 0;
    reading.ichrg = 0;
    reading.iauth = 0;
    reading.iaddr = 0;
    reading.iwatchdog = 0;
    reading.irssi = 0;

    boolean successful = BlinkyPicoW.publishCubeData((uint8_t*) &setting, (uint8_t*) &reading, false);
  }

  if(BlinkyPicoW.retrieveCubeSetting((uint8_t*) &setting))
  {
    if (oldSetting.authorizeBadge != setting.authorizeBadge)
    {
      stationCommand.iaddr = setting.authorizeBadge & 255;
      stationCommand.imsid = (setting.authorizeBadge >> 12) & 15;
      stationCommand.iauth = 1;
      stationCommand.iwarn = 0;
      if (CUBE_DIAG > 0)
      {
          Serial.print("setting.authorizeBadge: ");
          Serial.println(setting.authorizeBadge);
          Serial.print("stationCommand.iaddr: ");
          Serial.println(stationCommand.iaddr);
          Serial.print("stationCommand.imsid: ");
          Serial.println(stationCommand.imsid);
      }
      Serial1.write((uint8_t*) &stationCommand, sizeof(StationCommand));
    }
    if (oldSetting.sendWarning != setting.sendWarning)
    {
      stationCommand.iaddr = setting.sendWarning & 255;
      stationCommand.imsid = (setting.sendWarning >> 12) & 15;
      stationCommand.iwarn = (setting.sendWarning >> 8) & 15;
      stationCommand.iauth = 0;
      if (CUBE_DIAG > 0)
      {
          Serial.print("setting.sendWarning: ");
          Serial.println(setting.sendWarning);
          Serial.print("stationCommand.iaddr: ");
          Serial.println(stationCommand.iaddr);
          Serial.print("stationCommand.imsid: ");
          Serial.println(stationCommand.imsid);
          Serial.print("stationCommand.iwarn: ");
          Serial.println(stationCommand.iwarn);
      }
      Serial1.write((uint8_t*) &stationCommand, sizeof(StationCommand));
    }
    oldSetting.authorizeBadge = setting.authorizeBadge; 
    oldSetting.sendWarning = setting.sendWarning; 
  }

  while (Serial1.available() > 0) 
  {
    Serial1.readBytes((uint8_t*) &reading, sizeof(reading));
    if (CUBE_DIAG > 0)
    {
      Serial.print("addr: ");
      Serial.print(reading.iaddr);
      Serial.print(", vbat: ");
      Serial.print(reading.ivbat);
      Serial.print(", vusb: ");
      Serial.print(reading.ivusb);
      Serial.print(", butt: ");
      Serial.print(reading.ibutt);
      Serial.print(", move: ");
      Serial.print(reading.imove);
      Serial.print(", chrg: ");
      Serial.print(reading.ichrg);
      Serial.print(", auth: ");
      Serial.print(reading.iauth);
      Serial.print(", wdog: ");
      Serial.print(reading.iwatchdog);
      Serial.print(", rssi: ");
      Serial.println(reading.irssi);
    }
    lastPublishTime = now;
    boolean successful = BlinkyPicoW.publishCubeData((uint8_t*) &setting, (uint8_t*) &reading, true);
  }


}
