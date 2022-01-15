#include <Arduino.h>
#include "Transistor.h"

Transistor::Transistor(int pU,int pD)
{
  pinU = pU;
  pinD = pD;
  transPwm = 255;
  isForceStoped = false;
}
void Transistor::setHigh()
{
  if (isForceStoped) return;  
  digitalWrite(pinD,LOW);
  digitalWrite(pinU,HIGH);

  stateCompare(1);
}
void Transistor::setLow()
{
  if (isForceStoped) return;
  digitalWrite(pinU,LOW);
  analogWrite(pinD,transPwm);

  stateCompare(0);
}
void Transistor::turnOff()
{
  digitalWrite(pinU,LOW);
  digitalWrite(pinD,LOW);
}
void Transistor::setPwm(int p){
  transPwm = p;
}
void Transistor::phazeCheck( void (*pFunc)(void) ){
  if(lastState==-1) return;

  if (lastStateTime + 1000 < millis())
  {
    pFunc();
  } 
}