#include "Transistor.h"

int fuseLedPin = 13;
//Faza U
int uTransU = 2;
int uTransD = 9;

Transistor aTrans = Transistor(uTransU, uTransD);

int uHall = A1;
//Faza V
int vTransU = 3;
int vTransD = 10;

Transistor bTrans = Transistor(vTransU, vTransD);

int vHall = A2;
//Faza W
int wTransU = 4;
int wTransD = 11;

Transistor cTrans = Transistor(wTransU, wTransD);

int wHall = A3;

void setup()
{
  pinMode(fuseLedPin, OUTPUT);

  pinMode(uTransU, OUTPUT);
  pinMode(uTransD, OUTPUT);

  pinMode(uHall, INPUT_PULLUP);

  pinMode(vTransU, OUTPUT);
  pinMode(vTransD, OUTPUT);

  pinMode(vHall, INPUT_PULLUP);

  pinMode(wTransU, OUTPUT);
  pinMode(wTransD, OUTPUT);

  pinMode(wHall, INPUT_PULLUP);

  TCCR1B = TCCR1B & 0b11111000 | 0x02;
  TCCR2B = TCCR2B & 0b11111000 | 0x02;

  transTurnOffAll();

  Serial.begin(9600);
}

int readHall()
{
  int u = digitalRead(uHall);
  int v = digitalRead(vHall);
  int w = digitalRead(wHall);

  char os[] = "000";
  if (u)
    os[2] = '1';
  if (v)
    os[1] = '1';
  if (w)
    os[0] = '1';
  return strtol(os, (char **)NULL, 2);
}

int lastStep = 0;
unsigned long lastStepTime = millis();

void stepCompare(int s)
{
  if (s != lastStep)
  {
    lastStep = s;
    lastStepTime = millis();
  }
}

void stepTimeCheck(){
  if (lastStepTime + 500 < millis())
  {
    aTrans.setHigh();
    bTrans.turnOff();
    cTrans.turnOff();
    lastStepTime = millis();
  }
}

int outStep = 0;
void loop()
{
  outStep = readHall();

  switch (outStep)
  {
  case 5: // 1
    aTrans.turnOff();
    bTrans.setLow();
    cTrans.setHigh();
    stepCompare(outStep);
    break;
  case 1: // 2
    aTrans.setHigh();
    bTrans.setLow();
    cTrans.turnOff();
    stepCompare(outStep);
    break;
  case 3: // 3
    aTrans.setHigh();
    bTrans.turnOff();
    cTrans.setLow();
    stepCompare(outStep);
    break;
  case 2: // 4
    aTrans.turnOff();
    bTrans.setHigh();
    cTrans.setLow();
    stepCompare(outStep);
    break;
  case 6: // 5
    aTrans.setLow();
    bTrans.setHigh();
    cTrans.turnOff();
    stepCompare(outStep);
    break;
  case 4: // 6
    aTrans.setLow();
    bTrans.turnOff();
    cTrans.setHigh();
    stepCompare(outStep);
    break;
  default:
    break;
  }

  aTrans.phazeCheck(transTurnOffAll);
  bTrans.phazeCheck(transTurnOffAll);
  cTrans.phazeCheck(transTurnOffAll);

  stepTimeCheck();
  fuseCheck();
}

int transCheckState(int trans)
{
  return digitalRead(trans);
}

void forceStopAll(){
  aTrans.forceStop(true);
  bTrans.forceStop(true);
  cTrans.forceStop(true);
}

void transTurnOffAll()
{
  digitalWrite(uTransU, LOW);
  digitalWrite(uTransD, LOW);

  digitalWrite(vTransU, LOW);
  digitalWrite(vTransD, LOW);

  digitalWrite(wTransU, LOW);
  digitalWrite(wTransD, LOW);
}
