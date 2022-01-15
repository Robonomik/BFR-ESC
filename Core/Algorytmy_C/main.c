#include <stdio.h>
#include <math.h>
#include "extendedMath.h"

struct Vector2 {
  double x;
  double y;
};

//Pozycje geodezyjne robota
double latitude = 50.562223;
double longitude = 23.384057;
double height = 367;

//Pozycje geodezyjne smartfona
double Slatitude = 50.562536;
double Slongitude = 23.385487;
double Sheight = 367;

///Pozycje Kartezjańskie
struct Vector2 robotPostionEarth;
struct Vector2 robotPostion;
struct Vector2 smartfonPostionEarth;
struct Vector2 smartfonPostion;
struct Vector2 virtualNorth;

double vectorRSMagnitude = 0;
double vectorNorthMagnitude = 0;
double vectorRSAngle = 0;

void geo_to_cartesian(double lati, double longi, double hght, double *x, double *y)
{
  //Średnie promienie ziemi a i b
  double equatorialRadius = 6378137.0;
  double polarRadius = 6356752.314245;

  //Zmiana latitude i longitude z stopni na radiany (nie wiem jak u ciebie w Kotlinie jest czy cos przymuje radiany czy stopnie), niżej daje funkcje
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

int main(int argc, char const *argv[])
{
  //Wirtualna północ - pozycja lokalna
  virtualNorth.x = 0;
  virtualNorth.y = 3;

  //Przeliczanie lati longi height do x y 
  geo_to_cartesian(latitude, longitude, height, &robotPostionEarth.x, &robotPostionEarth.y);
  geo_to_cartesian(Slatitude, Slongitude, Sheight, &smartfonPostionEarth.x, &smartfonPostionEarth.y);

  //Obliczanie pozycji lokalnych
  robotPostion.x = 0;
  robotPostion.y = 0;
  smartfonPostion.x = robotPostionEarth.x - smartfonPostionEarth.x;
  smartfonPostion.y = robotPostionEarth.y - smartfonPostionEarth.y;

  //Obliczanie wielkości wektora
  vectorRSMagnitude = sqrt(pow((smartfonPostion.x - robotPostion.x), 2) + pow((smartfonPostion.y - robotPostion.y), 2));
  vectorNorthMagnitude = sqrt(pow((virtualNorth.x - robotPostion.x), 2) + pow((virtualNorth.y - robotPostion.y), 2));

  //Obliczanie kąta pomiędzy wektorem RS a osią y
  double vectorRSDot = (smartfonPostion.x * virtualNorth.x) + (smartfonPostion.y * virtualNorth.y);
  if (smartfonPostion.x >= 0)
  {
    vectorRSAngle = rad2deg(acos(vectorRSDot / (vectorRSMagnitude * vectorNorthMagnitude)));
  }
  else
  {
    vectorRSAngle = -rad2deg(acos(vectorRSDot / (vectorRSMagnitude * vectorNorthMagnitude)));
  }

  //Debugowanie
  printf("\nRobot - Latitude: %.5f stopni, Longitude: %.5f stopni, Height: %.2f metrow\n", latitude, longitude, height);
  printf("Smartfon - Latitude: %.5f stopni, Longitude: %.5f stopni, Height: %.2f metrow\n\n", Slatitude, Slongitude, Sheight);
  printf("Pozycja globalna robota: x: %.5f, y: %.5f\n", robotPostionEarth.x, robotPostionEarth.y);
  printf("Pozycja globalna smartfona: x: %.5f, y: %.5f\n", smartfonPostionEarth.x, smartfonPostionEarth.y);
  printf("Pozycja lokalna robota: x: %.5f, y: %.5f\n", robotPostion.x, robotPostion.y);
  printf("Pozycja lokalna smartfona: x: %.5f, y: %.5f\n\n", smartfonPostion.x, smartfonPostion.y);
  printf("Kat WektoraRS: %.2f stopni\n", vectorRSAngle);
  printf("Wielkosc WektoraRS: %.2fm\n", vectorRSMagnitude);
  //printf("X: %.5f\n", robotPostionEarth.x);
  //printf("Y: %.5f\n", robotPostionEarth.y);

  getchar();
  return 0;
}