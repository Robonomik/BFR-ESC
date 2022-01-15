#include <Wire.h>

int address = 0;
int devices = 0;

void setup() 
{
    Wire.begin();
    Serial.begin(9600);
    Serial.println("Inicjalizacja Skanera I2C!");
}

void loop() 
{
    Serial.println("Skanowanie...");

    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        int WireReturn = Wire.endTransmission();

        if(!WireReturn)
        {
            Serial.print("Znaleziono urządzenie: 0x");
            if (address < 16)
            {
                Serial.print("0");
            }            
            Serial.println(address, HEX);
            devices++;
        }
        else if (WireReturn == 4)
        {
            Serial.print("Nieznany błąd na adresie: 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            devices++;
        }
    }

    Serial.println("Skończono skanowanie!");
    Serial.print("Znaleziono: ");
    Serial.print(devices);
    Serial.println(" urządzeń!");
    devices = 0;
    
    delay(30000);
}
