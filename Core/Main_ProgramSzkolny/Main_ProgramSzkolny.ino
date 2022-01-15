#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LSM303.h>

//Wektory
struct Vector2
{
    double x;
    double y;
};

//Sieć połączeniowa
const char* SSID = "Robonomik";
const char* PASSWORD = "WojewodaA1";

//Połączenie GPS z ESP RX TX
SoftwareSerial gps_uart (D3, D0);

//Zmienne dla GPS
double robotLatitude = 0.0;
double robotLongitude = 0.0;
double robotHeight = 0.0;
String GPS_data = "";
String GP_GGA = "";

//Pozycje robota kartezjańskie
struct Vector2 robotPostionGlobal;
struct Vector2 smartfonPostionGlobal;
struct Vector2 robotPostionLocal;
struct Vector2 smartfonPostionLocal;
struct Vector2 virtualNorth;

//Właściwości Wektorów
double vectorRSMagnitude = 0;
double vectorNorthMagnitude = 0;
double vectorRSAngle = 0;

//protokół I2C
String I2CData = "";

//Kompass
LSM303 compass;

double deg2rad(double deg) 
{
    //Zmiana stopni na radiany
    double rad = deg * PI / 180;
    return rad;
}
double rad2deg(double rad) 
{
    //Zmiana radiany na stopnie
    double deg = rad * 180 / PI;
    return deg;
}
void geo_to_cartesian(double lati, double longi, double hght, double *x, double *y)
{
  //Średnie promienie ziemi a i b
  double equatorialRadius = 6378137.0;
  double polarRadius = 6356752.314245;

  //Zmiana latitude i longitude z stopni na radiany
  lati = deg2rad(lati);
  longi = deg2rad(longi);

  //Przeliczanie z geo na XYZ
  double eToSquare = 1 - (pow(polarRadius, 2) / pow(equatorialRadius, 2));
  double Nlati = equatorialRadius / sqrt(1 - eToSquare * pow(sin(lati), 2));

  //Przypisywanie wartości
  *x = (Nlati + hght) * cos(lati) * cos(longi);
  *y = (Nlati + hght) * cos(lati) * sin(longi);
  //Opcjonalne Z
  //double z = ((pow(polarRadius, 2) / pow(equatorialRadius, 2)) * Nlati + hght) * sin(lati);
}
void get_local_postion()
{
    //Zamiana wartości geodezyjnych na siatkę kartezjańską
    geo_to_cartesian(robotLatitude, robotLongitude, robotHeight, &robotPostionGlobal.x, &robotPostionGlobal.y);

    //Wyznaczanie pozycji lokalnej
    robotPostionLocal.x = 0;
    robotPostionLocal.y = 0;
    smartfonPostionLocal.x = robotPostionGlobal.x - smartfonPostionGlobal.x;
    smartfonPostionLocal.y = robotPostionGlobal.y - smartfonPostionGlobal.y;
}
void WiFi_settings()
{
    //Ustawienia WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);

    //Czekanie na połączenie
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("Nieudane połączenie do sieci! Restartowanie...");
        ESP.restart();
    }
    else if (WiFi.waitForConnectResult() == WL_CONNECTED)
    {
        Serial.print("Połączono do sieci: ");
        Serial.println(WiFi.SSID());
        Serial.print("Adres IP: ");
        Serial.println(WiFi.localIP());
    }
}
void OTA_updater()
{
    //Ustawianie nazwy ESP8266 w sieci
    ArduinoOTA.setHostname("ESP8266-Przewodnik");

    //Rebootowanie ESP po aktualizacji
    ArduinoOTA.setRebootOnSuccess(true);

    //Ustawienia ESP na start aktualizacji
    ArduinoOTA.onStart([](){
        Serial.println("Wgrywanie...");
        // Tutaj ustawić wszystko nieruchomo żeby łazik podczas aktualizacji nie zaczął jechać bez kontroli
    });

    //Ustawienia ESP po zakończeniu aktualizacji
    ArduinoOTA.onEnd([](){
        Serial.println("Wgrano program!");
    });

    //Ustawienia ESP gdy nastapi błąd podczas aktualizacji
    ArduinoOTA.onError([](ota_error_t error){
        Serial.print("Podczas aktualizacji wystąpił błąd: ");
        if(error == OTA_AUTH_ERROR)
        {
            Serial.println("AUTH_ERROR");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("BEGIN_FAILED");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("CONNECT_ERROR");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("RECEIVE_ERROR");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("END_ERROR");
        }
    });

    //Inicjalizacja OTA
    ArduinoOTA.begin();
    Serial.println("Gotowy!");
}
void GPS_get_postion(double *setLati, double *setLongi, double *setHght)
{
    if(gps_uart.available())
    {
        GPS_data = gps_uart.readStringUntil('\n');
        Serial.println(GPS_data);

        //Wyszukiwanie frazy $GPGGA
        if(GPS_data.indexOf("$GPGGA") > -1)
        {
            //Zakończenie lini
            GP_GGA = GPS_data.substring(GPS_data.indexOf("$GPGGA"), GPS_data.indexOf("*") + 3);

            int colon_array[15];
            for (int i = 1; i < 15; i++)
            {
                colon_array[i] = GPS_data.indexOf(',', colon_array[i - 1] + 1);
            }

            //Ustawienia latitude
            if (GP_GGA.substring(colon_array[2], colon_array[3]) != ",")
            {
                *setLati = (GP_GGA.substring(colon_array[2] + 1, colon_array[3])).toDouble() / 100;
            }
            else
            {
                *setLati = 0;
            }
            
            //Ustawianie longitude
            if (GP_GGA.substring(colon_array[4], colon_array[5]) != ",")
            {
                *setLongi = (GP_GGA.substring(colon_array[4] + 1, colon_array[5])).toDouble() / 100;
            }
            else
            {
                *setLongi = 0;
            }
            
            //Ustawianie Height
            if (GP_GGA.substring(colon_array[9], colon_array[10]) != ",")
            {
                *setHght = (GP_GGA.substring(colon_array[9] + 1, colon_array[10])).toDouble();
            }
            else
            {
                *setHght = 0;
            }
        }
        Serial.print("\nLatitude: ");
        Serial.println(robotLatitude);
        Serial.print("Longitude: ");
        Serial.println(robotLongitude);
    }
}
void I2C_read(String *data, byte address)
{
    Wire.beginTransmission(address);
}
double read_robot_angle()
{
    compass.read();
    double angle = compass.heading();
    return angle;
}
void setup() 
{
    //I2C
    Wire.begin();

    //Ustawienia USB-UART
    Serial.begin(115200);
    Serial.println("Inicjalizacja ESP8266!");

    //Połączenie GPS z ESP
    gps_uart.begin(9600);

    //Inicjalizowanie kompasu
    compass.init();
    compass.enableDefault();
    compass.m_min = (LSM303::vector<int16_t>){-2453, -2735, -2865};
    compass.m_max = (LSM303::vector<int16_t>){+3635, +3145, +3089};

    WiFi_settings();
    OTA_updater();
}
void loop() 
{
    ArduinoOTA.handle();
    GPS_get_postion(&robotLatitude, &robotLongitude, &robotHeight);
    get_local_postion();
    delay(1000);
}