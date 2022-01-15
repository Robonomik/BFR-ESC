#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LSM303.h>

//Wektory
struct Vector2
{
    double x;
    double y;
};

//Sieć połączeniowa
const char* SSID = "SzymonExtended";
const char* PASSWORD = "Trawa555";

//Połączenie GPS z ESP RX TX
SoftwareSerial gps_uart (0, 16);

//Zmienne dla GPS
double robotLatitude = 0;
double robotLongitude = 0;
double robotHeight = 0;
String GPS_data = "";
String GP_GGA = "";

//Pozycje robota, smartfona - xy
struct Vector2 robotPositionGlobal;
struct Vector2 robotPositionLocal;
struct Vector2 smartfonPositionGlobal;
struct Vector2 smartfonPositionLocal;
struct Vector2 virtualNorth;

//Właściwości wektorów
double vectorRSMagnitude = 0;
double vectorNorthMagnitude = 0;
double vectorRSAngle = 0;
double robotAngle = 0;

//Protokół I2C
String I2C_data = "";

//Kompas
LSM303 compass;

//Debug
unsigned long currentMillis = 0;
unsigned long prevMillis = 0;

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
void GPS_get_postion(double *setLati, double *setLongi, double *setHght)
{
    if(gps_uart.available())
    {
        GPS_data = gps_uart.readStringUntil('\n');

        if(GPS_data.indexOf("$GPGGA") > -1)
        {
            //Wyznaczanie końca lini
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
        // Serial.print("Latitude: ");
        // Serial.println(*setLati, 7);
        // Serial.print("Longitude: ");
        // Serial.println(*setLongi, 7);
        // Serial.print("Height: ");
        // Serial.println(*setHght, 7);
        // Serial.println("");
    }
}
void get_local_postion()
{
    geo_to_cartesian(50.3194887, 23.2367116, 280.0, &smartfonPositionGlobal.x, &smartfonPositionGlobal.y);
    robotPositionLocal.x = 0;
    robotPositionLocal.y = 0;
    smartfonPositionLocal.x = robotPositionGlobal.x - smartfonPositionGlobal.x;   
    smartfonPositionLocal.y = robotPositionGlobal.y - smartfonPositionGlobal.y;   
}
void get_vectorRS()
{   
    //Ustalanie pozycji wirtualnej północy
    virtualNorth.x = 0;
    virtualNorth.y = 3;

    //Wyznaczanie długości wektora
    vectorRSMagnitude = sqrt(pow((smartfonPositionLocal.x - robotPositionLocal.x), 2) + pow((smartfonPositionLocal.y - robotPositionLocal.y), 2));
    vectorNorthMagnitude = sqrt(pow((virtualNorth.x - robotPositionLocal.x), 2) + pow((virtualNorth.y - robotPositionLocal.y), 2));

    //Obliczanie kątu wektora RS z wektorem North
    double vectorRSdot = (smartfonPositionLocal.x * virtualNorth.x) + (smartfonPositionLocal.y * virtualNorth.y);
    if(smartfonPositionLocal.x >= 0)
    {
        vectorRSAngle = rad2deg(acos(vectorRSdot / (vectorRSMagnitude * vectorNorthMagnitude)));
    }
    else
    {
        vectorRSAngle = -rad2deg(acos(vectorRSdot / (vectorRSMagnitude * vectorNorthMagnitude)));
    }
}
double read_robot_angle()
{
    compass.read();
    double angle = compass.heading();
    return angle;
}
void setup() 
{
    //Ustawienia USB-UART
    Serial.begin(115200);
    Serial.println("Inicjalizacja ESP8266!");

    //Połączenie GPS z ESP
    gps_uart.begin(9600);

    //I2C Protokół
    Wire.begin();

    //Kompas
    compass.init();
    compass.enableDefault();
    compass.m_min = (LSM303::vector<int16_t>){-2453, -2735, -2865};
    compass.m_max = (LSM303::vector<int16_t>){+3635, +3145, +3089};

    WiFi_settings();
    OTA_updater();
}
void loop() 
{
    currentMillis = millis();
    ArduinoOTA.handle();
    GPS_get_postion(&robotLatitude, &robotLongitude, &robotHeight);
    geo_to_cartesian(robotLatitude, robotLongitude, robotHeight, &robotPositionGlobal.x, &robotPositionGlobal.y);
    get_local_postion();
    robotAngle = read_robot_angle();
    get_vectorRS();
    if(currentMillis - prevMillis >= 500)
    {
        Serial.print("VectorRS Angle: ");
        Serial.println(vectorRSAngle, 7);
        Serial.print("VectorRS Magnitude: ");
        Serial.println(vectorRSMagnitude, 7);
        Serial.print("Robot Angle: ");
        Serial.println(robotAngle, 7);
        Serial.print("Smartfon local X: ");
        Serial.println(smartfonPositionLocal.x, 7);
        Serial.print("Smartfon local Y: ");
        Serial.println(smartfonPositionLocal.y, 7);
        Serial.println("");
        prevMillis = currentMillis;
    }
}