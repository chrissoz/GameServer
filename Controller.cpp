//network setup
#include <ESP8266WiFi.h>
#include "WiFiUdp.h"
#include "Controller.h"
unsigned int controllerUDPPort = 58266;  // local port to listen on
WiFiUDP ControlerUdp;
unsigned long int controllerPlayerColours[6] = {0, 255 << 16, 255 << 8, 255 , (255 << 16) + (255 << 8), 255 + (255 << 16)};

Controller::Controller (int n) {
  number = n;
  Serial.printf("Intialised controller to %d\n", number);
}

void  Controller::setLed(int led, int r, int g, int b)
{
  Serial.printf("setting led %d on COntroller %d to %d %d %d\n", led, number, r, g, b);
  leds[led * 3] = r;
  leds[led * 3 + 1] = g;
  leds[led * 3 + 2] = b;
  for  (int i = 0; i < ledNum; i++) {
    Serial.printf(" %d", leds[i]);
  }
  Serial.println();
}

void  Controller::setLed(int led, unsigned long int c) {
  setLed(led, ((c >> 16) & 0xff), ((c >> 8) & 0xff), (c & 0xff) );
}

void  Controller::setAllLed(int s, unsigned long int c) {
  setAllLed(s, ((c >> 16) & 0xff), ((c >> 8) & 0xff), (c & 0xff) );
}

void  Controller::setAllLed(int s, int r, int g, int b) {
  Serial.printf("set all leds %d %d %d %d %d \n", number, s, r, g, b);
  int l = 0; int h = 6;
  if (s == 1) {
    l = 0; h = 3;
  }
  if (s == 2) {
    l = 3; int h = 6;
  }
  for (int x = l; x < h; x++) {
    this->setLed(x, r, g, b);

  }
}

void  Controller::displayLed () {
  ControlerUdp.beginPacket(IPAddress(192, 168, 4, number), controllerUDPPort);
  ControlerUdp.write(leds, 18);
  ControlerUdp.endPacket();
  Serial.printf("Sending Controller %d\n",number);
  for  (int i = 0; i < 18; i++) {
    Serial.printf(" %d", leds[i]);
  }
  Serial.println();
}
void  Controller::sendKeepalive() {
  if (millis() > (keepAlive + 200)) {
    keepAlive = millis();
    ControlerUdp.beginPacket(IPAddress(192, 168, 4, number), controllerUDPPort);
    ControlerUdp.write(255);
    ControlerUdp.endPacket();
  }
}

void  Controller::setPlayer (int player, int side) {
  this->setAllLed(side, controllerPlayerColours[player]);
  this->displayLed();
  onPlayer[side] = player;
}

void  Controller::setPlayer (int player) {
  this->setAllLed(3, controllerPlayerColours[player]);
  this->displayLed();
  onPlayer[1] = player;
};




