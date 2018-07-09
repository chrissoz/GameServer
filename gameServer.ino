
//make selection static
//build display of game or players into routine and di every 5 sec
//generic slection routine?


//matrix setup
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#define PIN 0
#define BRIGHTNESS 96
#define mw 10
#define mh 10
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(mw, mh, PIN,
    NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
    NEO_GRB            + NEO_KHZ800);
//end matrix setup

//network setup
#include <ESP8266WiFi.h>
#include "WiFiUdp.h"
WiFiUDP EthernetUdp;
unsigned int UDPPort = 58266;  // local port to listen on
WiFiUDP Udp;



#define maxPlayers 5
#define minPlayers 2
#define maxgame 4

int gameMode = 0;
#define gameCounterReset 500
int gameCounter = gameCounterReset;
int winScore = 9;
int winner = 0;
bool debug = true;
char incomingPacket[255];
//int controllerCount = 0;
//int playerColours[6][3] = {{0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 0, 255}};
unsigned long int playerColours[6] = {0, 255 << 16, 255 << 8, 255 , (255 << 16) + (255 << 8), 255 + (255 << 16)};

//this hold status of which player's lights are lit on a controller
int controller[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define controllerMin 2
#define controllerMax 3
int numberControllers = 2;
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

  Serial.begin(115200);
  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("Game");
  display_scrollText("B");

  Udp.begin(UDPPort);
  randomSeed(analogRead(0));

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
}



void loop() {
  delay(20);
  // scrolltext runs ever loop to scroll the text
  display_scrollText("");
  isPacket = false;
  // get packet and parse it
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (debug) {
      Serial.println(incomingPacket);
    }
    if (packetSize == 4) {
      sw[1] = incomingPacket[1] - 48;
      sw[2] = incomingPacket[2] - 48;
      sw[3] = incomingPacket[3] - 48;
      cont = incomingPacket[0] - 48;
      isPacket = true;
      if (debug) {
        matrix->drawPixel(6 + cont, 4 , matrix->Color(255, 255, 255));
        matrix->drawPixel(6 + cont, 1 , matrix->Color(255 * sw[1], 255 * sw[1], 255 * sw[1]));
        matrix->drawPixel(6 + cont, 2 , matrix->Color(255 * sw[2], 255 * sw[2], 255 * sw[2]));
        matrix->drawPixel(6 + cont, 3 , matrix->Color(255 * sw[3], 255 * sw[3], 255 * sw[3]));
        matrix->show();
        matrix->drawPixel(6 + cont, 4 , matrix->Color(0, 0, 0));
        matrix->show();
      }
    }
  }

  char ch = Serial.read();
  if (!(ch == 255)) {
    Serial.println(int(ch));
    isPacket = true;
    if (ch == 49) {
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

    if ((ch == 52) && (!(gameMode == 14))) {
      test3 = gameMode;  gameMode = 14;
      Serial.print("Pause on");
      isPacket = false;
    }
    if (ch == 53) {
      debug = !debug;
      Serial.printf("debug is %d", debug);
      isPacket = false;
    }

    cont = 2;
  }
  if (gameMode == 14) {
    //just pauses current game
    if (isPacket) {
      gameMode = test3;
      Serial.print("Pause off");
    }
  }
  if (gameMode == 0) {
    //gamemode 0 is startup
    display_scrollText("Players? ");
    for (int i = 0; i <= maxPlayers; i++) {
      score[i] = 0;

    }
    for (int i = controllerMin; i <= controllerMax; i++) {
      Serial.printf("Count %d\n", i);
      setcontall(i, 0, 0, 0);
      controller[i] = 0;
      controller[i + 5] = 0;

    }
    Serial.println (i);
    gameCounter == 30;
    gameMode = 2;
  }

  if (gameMode == 2) {

    //gamemode 2 is select number of players
    if (inProg) {} else {
      display_scrollText(numberPlayers);
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
  }
  if (gameMode == 3) {
    //gamemode 3 is display game?
    display_scrollText("Game? ");
    gameMode = 4;
  }
  if (gameMode == 4) {

    //gamemode 14 is select game
    if (inProg) {} else {
      display_scrollText(game);
    }
    if (isPacket) {
      if (sw[1] == 0) {
        game++;
        if (game > maxgame) {
          game = 1;
        }
      }
      if (sw[2] == 0) {
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
  }
  if (gameMode == 15) {
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
  }
  if (gameMode == 20) {
    if (gameCounter % 40 == 20) {
      matrix->fillRect(0, 0, 9, 9, c24to16(playerColours[winner]));
      matrix->show();
      for (int i = 2; i < 5; i++) {
        setcontall(i, playerColours[winner]);
      }
    }
    if (gameCounter % 40 == 0) {
      matrix->fillRect(0, 0, 9, 9, playerColours[0]);
      matrix->show();
      for (int i = 2; i < 5; i++) {
        setcontall(i, playerColours[0]);
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
  }


  if (gameMode == 100) {
    //gamemode 100 is game 1
    if (random (1, actionChance) == 3) {
      // controllerNumber is the controller number
      int controllerNumber = random(2, 4);
      int playerNumber = random(1, 7);
      if (playerNumber <= numberPlayers) {
        if (debug) {
          Serial.printf ("\nSet player %d controller %d colour %d\n", playerNumber, controllerNumber), playerColours[playerNumber];
        }
        setcontall(controllerNumber, playerColours[playerNumber]);
        controller[controllerNumber] = playerNumber;
      } else {
        controller[controllerNumber] = 0;
        setcontall(controllerNumber, 0, 0, 0);
        if (debug) {
          Serial.printf ("\nReset controller %d", controllerNumber);
        }

      }


    }



    if (((sw[1] + sw[2] + sw[3]) == 3)) {} else {
      if (controller[cont] == 0) {} else {
        if (debug) {
          Serial.printf ("\nmatch player %d colour %d controller %d\n", controller[cont], playerColours[controller[cont]], cont);
        }
        setcontall(cont, 0, 0, 0);
        score[controller[cont]]++;
        matrix->drawPixel(controller[cont], score[controller[cont]] , c24to16(playerColours[controller[cont]]));
        matrix->show();
        if (score[controller[cont]] == 6) {
          winner = controller[cont];

          for (int i = 0; i < 5; i++) {
            score[i] = 0;
            controller[i] = 0;
          }
          gameMode = 20;

        }
        controller[cont] = 0;

      }
    }
  }


  if (gameMode == 110) {
    //gamemode 110 is game 2
    if (random (1, actionChance) == 3) {
      // controllerNumber is the controller number
      int controllerNumber = random(2, 4);
      int side = random(1, 3);
      int playerNumber = random(1, 7);
      if (playerNumber <= numberPlayers) {
        if (debug) {
          Serial.printf ("\nSet player %d controller %d colour %d side %d\n", playerNumber, controllerNumber, playerColours[playerNumber], side);
        }
        setcontall(controllerNumber, side, playerColours[playerNumber]);
        controller[controllerNumber + ((side - 1) * 5)] = playerNumber;
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
          setcontall(controllerNumber, side, playerColours[playerNumber2]);
          controller[controllerNumber + ((side - 1) * 5)] = playerNumber2;
        }
      } else {
        if (game == 4) {
          controller[controllerNumber] = 0;
          controller[controllerNumber + 5] = 0;
          setcontall(controllerNumber, 0, 0, 0);

        } else
          controller[controllerNumber + ((side - 1) * 5)] = 0;
        setcontall(controllerNumber, side, 0, 0, 0);
      }
      Serial.print ("Reset controller");
      Serial.println (controllerNumber + ((side - 1) * 5));
    }


  }
  if (((sw[2] == 0) || (sw[1] == 0)) && isPacket) {
    int offset = 0;
    if (sw[1] == 0) {
      offset = 5;
    }

    if (controller[cont + offset] == 0) {} else {
      if (debug) {
        Serial.printf ("\nmatch player %d colour %d controller %d offset %d\n", controller[cont + offset], playerColours[controller[cont + offset]], cont, offset);
      }
      if (game == 4) {
        if (offset == 0) {
          controller[cont + 5] = 0;
          setcontall(cont, 2, 0, 0, 0);
        } else {
          controller[cont] = 0;
          setcontall(cont, 1, 0, 0, 0);

        }
        setcontall(cont, 1 + (offset / 5), 0, 0, 0);
        score[controller[cont + offset]]++;
        matrix->drawPixel(controller[cont + offset], score[controller[cont + offset]] , c24to16(playerColours[controller[cont + offset]]));
        matrix->show();

        if (score[controller[cont + offset]] == 6) {
          winner = controller[cont + offset];
          gameMode = 20;
        }
        controller[cont + offset] = 0;
      }
    }
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


void setcontall(int controller, int s, int r, int g, int b) {

  int l = 0; int h = 6;
  if (s == 1) {
    l = 0; h = 3;
  }

  if (s == 2) {
    l = 3; int h = 6;
  }
  Serial.printf("\ncontroller %d side %d r %d g %d b %d l %d h %d\n", controller, s, r, g, b, l, h);

  for (int x = l; x < h; x++) {
    leds[controller][x * 3] = r;
    leds[controller][x * 3 + 1] = g;
    leds[controller][x * 3 + 2] = b;

  }
  Udp.beginPacket(IPAddress(192, 168, 4, controller), UDPPort);
  Udp.write(leds[controller], 18);
  Udp.endPacket();
  for  (int i = 0; i < 19; i++) {
    Serial.printf(" %d", leds[controller][i]);
  }
  Serial.println();
  matrix->drawPixel(8, controller + 4, matrix->Color(leds[controller][0], leds[controller][1], leds[controller][2]));
  matrix->drawPixel(9, controller + 4, matrix->Color(leds[controller][9], leds[controller][10], leds[controller][11]));
  matrix->show();
}
void setcontall(int controller, int r, int g, int b) {
  setcontall(controller, 3, r, g, b);
}

void setcontall(int controller, unsigned long int c) {
  setcontall(controller,  ((c >> 16) & 0xff), ((c >> 8) & 0xff), (c & 0xff) );
}
void setcontall(int controller, int r, unsigned long int c) {
  setcontall(controller, r,  ((c >> 16) & 0xff), ((c >> 8) & 0xff), (c & 0xff) );
}

unsigned int c24to16 (unsigned long int c) {
  int b = (c & 255) >> 3;
  int g = ((c >> 8) & 255) >> 2;
  int r = ((c >> 16) & 255) >> 3;
  return (b + (g << 5) + (r << 11));
}






