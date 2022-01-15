#include "extendedMath.h"

double deg2rad (double deg) {
  double rad = deg * PI / 180;
  return rad;
}
double rad2deg (double rad) {
  double deg = rad * 180 / PI;
  return deg;
}