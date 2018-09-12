//#include "ScrollLib.h"


//generic slection routine?
//icoming keepalives for controllers coming along - need to maintain a list of active controllers
//WDT reset issue

//matrix setup
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#define PIN 0
#define BRIGHTNESS 30
#define mw 10
#define mh 10
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(mw, mh, PIN,
    NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
    NEO_GRB            + NEO_KHZ800);
//end matrix setup

//network setup
//esp2866 ucoment follwoing
#include <ESP8266WiFi.h>
//esp32 uncomment following
//#include <WiFi.h>
#include "WiFiUdp.h"
WiFiUDP EthernetUdp;
unsigned int UDPPort = 58266;  // local port to listen on
WiFiUDP Udp;

#include "Controller.h"
Controller cnt[8] = {Controller(0), Controller(1), Controller(2), Controller(3), Controller(4), Controller(5), Controller(6), Controller(7)};
//int numControllers = 3;



#define maxPlayers 5
#define minPlayers 2
#define maxgame 4
#define maxControllers 10

int gameMode = 0;
#define gameCounterReset 500
int gameCounter = gameCounterReset;
int winScore = 8;
int winner = 0;
bool debug = false;
char incomingPacket[255];
//int controllerCount = 0;
//int playerColours[6][3] = {{0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 0, 255}};
unsigned long int playerColours[6] = {0, 255 << 16, 255 << 8, 255 , (255 << 16) + (255 << 8), 255 + (255 << 16)};
int numberActiveControllers = 0;
int activeControllerList[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define controllerMin 2
#define controllerMax 3
int numberControllers = 5;
int i = 0;
int sw[3];
int cont;
int game = 1;
int actionChance = 100;
int test1, test2, test3;
byte leds[5][18];
int score[5] = {0, 0, 0, 0, 0};
int numberPlayers = 3;
unsigned long oldmillis;
bool isPacket = false;

//scrolltext variables
unsigned long messSpeed = 60;
unsigned long lastTime;
String Message = "";
int messageLoop;
bool inProg = false;
//end scrolltext variables

void setup() {

  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(BRIGHTNESS);
  matrix->clear();
  matrix->setCursor(0, 0);
  matrix->print(F("Howdy"));

  Serial.begin(115200);
  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("Game");
  display_scrollText("B");

  Udp.begin(UDPPort);
  randomSeed(analogRead(1));

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
}



void loop() {
  delay(20);
  //Serial.print(".");
  for (int i = 1; i < numberControllers; i++) {
    cnt[i].sendKeepalive();
    cnt[i].isAlive++;
    if (cnt[i].isAlive == 300) {
      Serial.printf("Controller %d Dead\n", i);
    }
  }
  // scrolltext runs ever loop to scroll the text
  display_scrollText("");
  isPacket = false;
  // get packet and parse it
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (debug) {
      Serial.print("Incoming packet :");
      Serial.println(incomingPacket);
    }
    if (packetSize == 4) {
      sw[1] = incomingPacket[1] - 48;
      sw[2] = incomingPacket[2] - 48;
      sw[3] = incomingPacket[3] - 48;
      cont = incomingPacket[0] - 48;
      isPacket = true;
      if (debug) {
        displayScreen();
      }
    }
    if (packetSize == 1) {
      int controllerKeepaliveRecieved = incomingPacket[0];
      if ((controllerKeepaliveRecieved > 9) || (controllerKeepaliveRecieved < 2)) {
        Serial.printf("Keepalive out of bounds %d\n", controllerKeepaliveRecieved);
      } else {
        Serial.printf("Keepalive recived %d\n", controllerKeepaliveRecieved);
        if (cnt[controllerKeepaliveRecieved].isAlive > 299) {
          Serial.printf("Controller %d is alive \n", controllerKeepaliveRecieved);
        }
        cnt[controllerKeepaliveRecieved].isAlive = 0;
      }
    }
  }

  // this section just allow simuation via kboard if controllers are not availble - and toggling of debug mode and pausing
  char ch = Serial.read();
  if (!(ch == 255)) {
    Serial.println(int(ch));
    isPacket = true;
    if (ch == 49) { //1 on kvd
      sw[1] = 0;
    } else {
      sw[1] = 1;
    }
    if (ch == 50) {
      sw[2] = 0;
    } else {
      sw[2] = 1;
    }
    if (ch == 51) {
      sw[3] = 0;
    } else {
      sw[3] = 1;
    }

    if ((ch == 52) && (!(gameMode == 14))) { // 4 is pause
      test3 = gameMode;  gameMode = 14;
      Serial.print("Pause on");
      isPacket = false;
    }
    if (ch == 53) { //5 toggles debug
      debug = !debug;
      Serial.printf("debug is %d", debug);
      isPacket = false;
    }

    cont = 2;
  }
  switch (gameMode) {
    case 14:
      //just pauses current game
      if (isPacket) {
        gameMode = test3;
        Serial.print("Pause off");
      }
      break;
    case 0:
      //gamemode 0 is startup
      display_scrollText("Players? ");
      for (int i = 0; i <= maxPlayers; i++) {
        score[i] = 0;

      }
      for (int i = controllerMin; i <= controllerMax; i++) {
        cnt[i].setPlayer(0);
      }
      gameCounter == 30;
      gameMode = 2;
      break;
    case 2:    //gamemode 2 is select number of players
      if (inProg) {} else {
        displayText(numberPlayers);
      }
      if (isPacket) {
        if (sw[1] == 0) {
          numberPlayers++;
          if (numberPlayers > maxPlayers) {
            numberPlayers = minPlayers;
          }
        }
        if (sw[2] == 0) {
          isPacket = false;
          gameMode = 3;
        }
      }
      break;
    case 3:    //gamemode 3 is display game?
      display_scrollText("Game? ");
      gameMode = 4;
      break;
    case 4: //gamemode 14 is select game

      if (!inProg) {
        displayText(game);
      };
      if (isPacket) {
        if (sw[1] == 0) {
          game++;
          if (game > maxgame) {
            game = 1;
          }
        }
        if (sw[2] == 0) {
          matrix->clear();
          matrix->show();
          if (game == 1) {
            gameMode = 100;
            actionChance = 100;
          }
          if (game == 2) {
            gameMode = 100;
            actionChance = 30;
          }
          if (game == 3) {
            gameMode = 110;
            actionChance = 100;
          }
          if (game == 4) {
            gameMode = 110;
            actionChance = 100;
          }
        }
      }
      break;
    case 15:
      // this is just a matrix test\
      test1++;
      if (test1 % 30 == 0) {
        test2++;
        int p = test2 % (maxPlayers + 1);
        for (int i = 0; i < 11; i++) {
          for (int j = 0; j < 11; j++) {
            matrix->drawPixel(i, j, matrix->Color(i * 25, j * 25, ((test2 % 25) * 10)));
          }
        }

        matrix->show();
      }

      break;
    case 20: //flashes everything on a win
      if (gameCounter % 40 == 20) {
        matrix->fillRect(0, 0, 9, 9, c24to16(playerColours[winner]));
        matrix->show();
        for (int i = 2; i < 5; i++) {
          cnt[i].setPlayer(winner);
        }
      }
      if (gameCounter % 40 == 0) {
        matrix->fillRect(0, 0, 9, 9, playerColours[0]);
        matrix->show();
        for (int i = 2; i < 5; i++) {
          cnt[i].setPlayer(0);
        }
      }
      //gamemode 20 is pause after win
      if (gameCounter == 0) {
        if (isPacket) {
          gameMode = 0;
          gameCounter = gameCounterReset;
        }
      } else {
        gameCounter--;
      }
      break;
    case 100:  //gamemode 100 is game 1
      if (random (1, actionChance) == 3) {
        compileControllerList ();
        // controllerNumber is the controller number
        int controllerNumber = activeControllerList[random(1, numberActiveControllers+1)];
        int playerNumber = random(1, 7);
        if (playerNumber <= numberPlayers) {
          if (debug) {
            Serial.printf ("\nSet player %d controller %d colour %d\n", playerNumber, controllerNumber), playerColours[playerNumber];
          }
          cnt[controllerNumber].setPlayer(playerNumber);
        } else {
          cnt[controllerNumber].setPlayer(0);
          if (debug) {
            Serial.printf ("\nReset controller %d", controllerNumber);
          }

        }
      }
      if (((sw[1] + sw[2] + sw[3]) == 3)) {} else {
        if (cnt[cont].onPlayer[1] == 0) {} else {
          int p = cnt[cont].onPlayer[1];
          if (debug) {
            Serial.printf ("\nmatch player %d colour %d controller %d\n", p, playerColors[p],cont);
          }
          score[p]++;
          displayScreen();
          if (score[p] == winScore) {
            winner = p;
            gameMode = 20;
          }
          cnt[cont].setPlayer(0);
        }
      }
      break;
    case 110: //gamemode 110 is game 2
      if (random (1, actionChance) == 3) {
        // controllerNumber is the controller number
        int controllerNumber = random(2, 4);
        int side = random(1, 3);
        int playerNumber = random(1, 7);
        if (playerNumber <= numberPlayers) {
          //if (debug) {
          Serial.printf ("\nSet player %d controller %d colour %d side %d\n", playerNumber, controllerNumber, playerColours[playerNumber], side);
          //}
          cnt[controllerNumber].setPlayer(playerNumber, side);
          if (game == 4) {
            int playerNumber2 = random(1, numberPlayers);
            if (playerNumber2 == playerNumber) {
              playerNumber2 = numberPlayers;
            }

            if (side == 1) {
              side = 2;
            } else {
              side = 1;
            }
            //if (debug) {
            Serial.printf("side is %d p1 %d p2 %d\n", side, playerNumber, playerNumber2);
            //}
            cnt[controllerNumber].setPlayer(playerNumber2, side );
          } else {
            if (game == 4) {
              cnt[controllerNumber].setPlayer(0);
            } else {
              cnt[controllerNumber].setPlayer(0, side);
            }
          }
        }
      }


      if (((sw[2] == 0) || (sw[1] == 0)) && isPacket) {
        int side = 1;
        if (sw[1] == 0) {
          side = 2;
        }

        if (cnt[cont].onPlayer[side] == 0) {} else {
          int p = cnt[cont].onPlayer[side];

          //if (debug) {
          Serial.printf ("\nmatch player %d  controller %d side %d\n", p , cont, side);
          //}
          if (game == 4) {
            cnt[cont].setPlayer(0);
          } else {
            cnt[cont].setPlayer(0, side);

          }
          score[p]++;
          displayScreen();
          if (score[p] == winScore) {
            winner = p;
            gameMode = 20;
          }
        }
      }

      break;
    default:
      break;
  }

  //send keepalive
  if (millis() > (oldmillis + 200)) {
    oldmillis = millis();
    for (int z = 2; z < 4; z++) {
      //EthernetUdp
      Udp.beginPacket(IPAddress(192, 168, 4, z), UDPPort);
      Udp.write(255);
      Udp.endPacket();
    }
  }
}



unsigned int c24to16 (unsigned long int c) {
  int b = (c & 255) >> 3;
  int g = ((c >> 8) & 255) >> 2;
  int r = ((c >> 16) & 255) >> 3;
  return (b + (g << 5) + (r << 11));
}
void compileControllerList () {

  // compile a list of active controllers
  for (int i = 0; i < maxControllers; i++) {
    activeControllerList[i] = 0;
  }
  int c = 0;
  for (int i = 2; i < maxControllers; i++) {
    if (cnt[i].isAlive < 300) {
      activeControllerList[c] = i;
      c++;
    }
  }
  numberActiveControllers = c;
}
void displayScreen() {
  matrix->clear();
  if (debug) {
    //  debug for sent packets to controller

    for (int i = 1; i < numberControllers; i++) {

      matrix->drawPixel(8, i + 4, matrix->Color(leds[i][0], leds[i][1], leds[i][2]));
      matrix->drawPixel(9, i + 4, matrix->Color(leds[i][9], leds[i][10], leds[i][11]));
    }
    //debug for recieved packet from controller
    matrix->drawPixel(6 + cont, 1 , matrix->Color(255 * sw[1], 255 * sw[1], 255 * sw[1]));
    matrix->drawPixel(6 + cont, 2 , matrix->Color(255 * sw[2], 255 * sw[2], 255 * sw[2]));
    matrix->drawPixel(6 + cont, 3 , matrix->Color(255 * sw[3], 255 * sw[3], 255 * sw[3]));


  }
  for (int i = 1; i <= numberPlayers; i++) {
    if (debug) {
      Serial.printf("i %d score %d colour %d \n", i, score[1], playerColours[i]);
    }
    if (score[i]) {
      matrix->drawLine(i, 0, i, (score[i] - 1), c24to16(playerColours[i]));
    }
  }
  matrix->show();
}




