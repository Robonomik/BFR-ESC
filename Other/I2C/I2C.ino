#include <Wire.h>

String data_serial = "";
unsigned long millis_current = 0;
unsigned long millis_prev = 0;

void setup() 
{
  Wire.begin();
  Serial.begin(9600);

  Serial.println("Inicjalizacja I2C!");
}

void loop() 
{
  millis_current = millis();

  if(Serial.available())
  {
    data_serial = Serial.readStringUntil('\n');
    Serial.print("Otrzymano: ");
    Serial.print(data_serial);
    Serial.println(", wyślij następną liczbę z przedziału 0-255");
  }

  if(millis_current - millis_prev >= 50)
  {
    Wire.beginTransmission(8);
    Wire.write(data_serial.toInt());
    Wire.endTransmission();
    millis_prev = millis_current;
  }
}
