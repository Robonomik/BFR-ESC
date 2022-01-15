#include <SoftwareSerial.h>

SoftwareSerial uart(2, 3); //RX TX

void setup() 
{
  uart.begin(9600);
  Serial.begin(9600);
}

void loop() 
{
    if(uart.available())
    {
        Serial.write(uart.read());
    }
}
