#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <Arduino.h>
#include "FanMonitor.h"


const char *AP_name = "ROG兼容风扇";   
#define pwm D2
#define FAN_MONITOR_PIN D1
#define ledr D6
#define ledg D5
#define ledb D7

unsigned long previousMillis = 0;
const long interval = 2000;

String comdata = "";
FanMonitor _fanMonitor = FanMonitor(FAN_MONITOR_PIN, FAN_TYPE_BIPOLE);
const byte DNS_PORT = 53;
int fszs = 0;


IPAddress apIP(6, 6, 6, 6);
ESP8266WebServer webServer(80);
DNSServer dnsServer;
void setup()
{
  EEPROM.begin(4096);
  EEPROM.get(100, fszs);
  Serial.begin(115200);
  Serial.println("");
  SPIFFS.begin();
  analogWriteFreq(40000);
  pinMode(pwm, OUTPUT);
  pinMode(ledr, OUTPUT);
  pinMode(ledg, OUTPUT);
  pinMode(ledb, OUTPUT);
  digitalWrite(ledr, LOW); 
  digitalWrite(ledg, LOW); 
  digitalWrite(ledb, LOW); 
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_name);
  webServer.begin();
  webServer.on("/", wwwroot);
  webServer.on("/rog", rog);
  webServer.on("/f", f); //数据获取
  // 配置DNS服务
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  _fanMonitor.begin();//启动测速

  fxzj(); //风扇自检
  if (fszs <= 255)
  {
    ts(fszs);
  }
  else
  {
    ts(0);
  }


}





void wwwroot()
{

  File file = SPIFFS.open("/index.html", "r");
  webServer.streamFile(file, "text/html");
  file.close();



}
void rog() {
  File file = SPIFFS.open("/rog.apk", "r");
  webServer.streamFile(file, "application/vnd.android.package-archive");
  file.close();

}

void f() {
  String a = webServer.arg("f");
  fszs = a.toInt();
  EEPROM.put(100, fszs);
  EEPROM.commit();
  ts(fszs);
  webServer.send(200, "text/plain", "ok");
}




void loop()
{


  webServer.handleClient();
  if (Serial.available() > 0)
  {
    comdata = "";//缓存清零
    while (Serial.available() > 0) {
      comdata += char(Serial.read());
      delay(2);
    }
    if (comdata.length() > 0) {
      int a = comdata.toInt();
      if (a >= 0 && a <= 255) ts(a);
      if (a > 255) ys(a);
    }
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (fszs != 0) {
      uint16_t rpm = _fanMonitor.getSpeed();
      Serial.print(rpm);
    }
  }

}


void fxzj() {
  analogWrite(pwm, 1023);
  delay(1000);
  for (int a = 1023; a >= 500; a--) {
    analogWrite(pwm, a);
    delay(5);
  }
  analogWrite(pwm, 0);
}


void ts(int s) {

  int a;
  if (s == 0) {
    a = 0;
  } else if (s == 255) {
    a = 1023;
  } else {
    s = s + 245;
    float b = 1023 / 500;
    a = s * b;
  }
  analogWrite(pwm, a);
  fszs = a;

}

void ys(int y) {
  if (y >= 256 && y <= 511) {
    analogWrite(ledr, y - 256);
  }
  if (y >= 512 && y <= 767) {
    analogWrite(ledg, y - 512);
  }
  if (y >= 768 && y <= 1023) {
    analogWrite(ledb, y - 768);
  }
}
