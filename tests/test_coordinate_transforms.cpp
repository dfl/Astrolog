/*
** Unit tests for coordinate transformation functions in calc.cpp / matrix.cpp:
** RecToPol, PolToRec, SphToRec, RecToSph3, CoorXform, Decan, Dwad, Navamsa.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"

#include <cmath>

// ---- RecToPol / PolToRec ----

TEST_CASE("RecToPol unit vector along Y axis", "[coords]") {
  real a, r;
  // (0, 1) -> angle=pi/2, radius=1
  RecToPol(0.0, 1.0, &a, &r);
  REQUIRE(r == Approx(1.0).margin(1e-6));
  REQUIRE(a == Approx(rPiHalf).margin(1e-6));
}

TEST_CASE("RecToPol unit vector along X axis", "[coords]") {
  real a, r;
  // (1, 0) -> angle=0, radius=1
  RecToPol(1.0, 0.0, &a, &r);
  REQUIRE(r == Approx(1.0).margin(1e-6));
  REQUIRE(a == Approx(0.0).margin(1e-6));
}

TEST_CASE("RecToPol diagonal", "[coords]") {
  real a, r;
  // (1, 1) -> angle=pi/4, radius=sqrt(2)
  RecToPol(1.0, 1.0, &a, &r);
  REQUIRE(r == Approx(sqrt(2.0)).margin(1e-6));
  REQUIRE(a == Approx(rPi / 4.0).margin(1e-6));
}

TEST_CASE("PolToRec unit circle", "[coords]") {
  real x, y;
  // angle=0, r=1 -> (1, 0)
  PolToRec(0.0001, 1.0, &x, &y);  // Avoid exact 0 (PolToRec clamps to rSmall)
  REQUIRE(x == Approx(1.0).margin(1e-3));
  REQUIRE(y == Approx(0.0).margin(1e-3));
}

TEST_CASE("PolToRec 90 degrees", "[coords]") {
  real x, y;
  // angle=pi/2, r=1 -> (0, 1)
  PolToRec(rPiHalf, 1.0, &x, &y);
  REQUIRE(x == Approx(0.0).margin(1e-6));
  REQUIRE(y == Approx(1.0).margin(1e-6));
}

TEST_CASE("RecToPol/PolToRec round-trip", "[coords]") {
  real a, r, x, y;
  RecToPol(3.0, 4.0, &a, &r);
  REQUIRE(r == Approx(5.0).margin(1e-6));
  PolToRec(a, r, &x, &y);
  REQUIRE(x == Approx(3.0).margin(1e-6));
  REQUIRE(y == Approx(4.0).margin(1e-6));
}

// ---- SphToRec / RecToSph3 ----

TEST_CASE("SphToRec north pole", "[coords]") {
  real rx, ry, rz;
  // alt=90, any azi -> (0, 0, r)
  SphToRec(1.0, 0.0, 90.0, &rx, &ry, &rz);
  REQUIRE(rz == Approx(1.0).margin(1e-6));
  REQUIRE(rx == Approx(0.0).margin(1e-6));
  REQUIRE(ry == Approx(0.0).margin(1e-6));
}

TEST_CASE("SphToRec equator east", "[coords]") {
  real rx, ry, rz;
  // alt=0, azi=0 -> (r, 0, 0)
  SphToRec(1.0, 0.0, 0.0, &rx, &ry, &rz);
  REQUIRE(rx == Approx(1.0).margin(1e-6));
  REQUIRE(ry == Approx(0.0).margin(1e-6));
  REQUIRE(rz == Approx(0.0).margin(1e-6));
}

TEST_CASE("SphToRec/RecToSph3 round-trip", "[coords]") {
  real rx, ry, rz, azi, alt;
  SphToRec(1.0, 45.0, 30.0, &rx, &ry, &rz);
  RecToSph3(rx, ry, rz, &azi, &alt);
  REQUIRE(azi == Approx(45.0).margin(0.01));
  REQUIRE(alt == Approx(30.0).margin(0.01));
}

// ---- CoorXform ----

TEST_CASE("CoorXform zero tilt is identity", "[coords]") {
  real azi = 45.0, alt = 30.0;
  CoorXform(&azi, &alt, 0.0);
  REQUIRE(azi == Approx(45.0).margin(0.01));
  REQUIRE(alt == Approx(30.0).margin(0.01));
}

TEST_CASE("CoorXform ecliptic to equatorial known point", "[coords]") {
  // Ecliptic north pole (lon=0, lat=90) with obliquity 23.44
  // should become equatorial north ecliptic pole
  real azi = 0.0, alt = 90.0;
  CoorXform(&azi, &alt, 23.44);
  // At the pole, the result should still be at alt ~66.56 (90 - 23.44)
  REQUIRE(alt == Approx(90.0 - 23.44).margin(0.1));
}

TEST_CASE("CoorXform equator point", "[coords]") {
  // A point on the ecliptic equator (azi=90, alt=0) tilted by obliquity
  // should get a latitude of +obliquity
  real azi = 90.0, alt = 0.0;
  real obliquity = 23.44;
  CoorXform(&azi, &alt, obliquity);
  REQUIRE(alt == Approx(obliquity).margin(0.1));
}

// ---- Decan ----

TEST_CASE("Decan first decan stays in same sign", "[coords]") {
  // 5 Aries (5.0 degrees absolute) -> first decan of Aries -> Aries
  real deg = 5.0;
  real result = Decan(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 1);  // Aries
}

TEST_CASE("Decan second decan", "[coords]") {
  // 15 Aries (15.0 degrees) -> second decan -> Leo (sign 5)
  real deg = 15.0;
  real result = Decan(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 5);  // Leo
}

TEST_CASE("Decan third decan", "[coords]") {
  // 25 Aries (25.0 degrees) -> third decan -> Sagittarius (sign 9)
  real deg = 25.0;
  real result = Decan(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 9);  // Sagittarius
}

// ---- Dwad ----

TEST_CASE("Dwad first dwad stays in same sign", "[coords]") {
  // 1 Aries (1.0 degrees) -> first dwad (0-2.5) -> Aries
  real deg = 1.0;
  real result = Dwad(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 1);  // Aries
}

TEST_CASE("Dwad second dwad", "[coords]") {
  // 3 Aries (3.0 degrees) -> second dwad (2.5-5.0) -> Taurus
  real deg = 3.0;
  real result = Dwad(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 2);  // Taurus
}

// ---- Navamsa ----

TEST_CASE("Navamsa first navamsa of fire sign", "[coords]") {
  // 1 Aries -> first navamsa -> Aries
  real deg = 1.0;
  real result = Navamsa(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 1);  // Aries
}

TEST_CASE("Navamsa second navamsa of fire sign", "[coords]") {
  // ~4 Aries -> second navamsa (3.33-6.67) -> Taurus
  real deg = 4.0;
  real result = Navamsa(deg);
  int sign = SFromZ(result);
  REQUIRE(sign == 2);  // Taurus
}
