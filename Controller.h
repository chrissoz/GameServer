#if !(defined(H_Controller))  
#define H_Controller
class Controller {
  private:
    byte leds[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 , 0, 0, 0};
    int number;
    const int ledNum = 18;
    const int switchNum = 3;
    long int keepAlive;
  public:
    Controller (int n) ;
    bool switches [3] = {0, 0, 0};
    bool isActive = false;
    unsigned int onPlayer [3] = {0, 0, 0};
    void setLed(int led, unsigned long int c);
    void setLed(int led, int r, int g, int b);
    void  setAllLed(int s, unsigned long int c);
    void  setAllLed(int s, int r, int g, int b);
    void  displayLed ();
    void sendKeepalive() ;
    void  setPlayer (int player, int side);
    void setPlayer (int player);
    //void showController ();
};
#endif
