class Transistor
{
private:
  int pinU;
  int pinD;
  int transPwm;
  bool isForceStoped;

  int lastState = -1;
  unsigned long lastStateTime = millis();

public:
  Transistor(int pU, int pD);
  void setHigh();
  void setLow();
  void turnOff();
  void setPwm(int p);
  void phazeCheck( void (*pFunc)(void) );
  void forceStop(bool i)
  {
    isForceStoped = i;
  }
  void stateCompare(int s)
  {
    if (s != lastState)
    {
      lastState = s;
      lastStateTime = millis();
    }
  }
};