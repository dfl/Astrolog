/*
** Unit tests for NParseSz / RParseSz parsing functions in io.cpp.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"

// ---- pmMon: Month name parsing ----

TEST_CASE("NParseSz pmMon full month names", "[parsing]") {
  REQUIRE(NParseSz("January", pmMon) == 1);
  REQUIRE(NParseSz("February", pmMon) == 2);
  REQUIRE(NParseSz("March", pmMon) == 3);
  REQUIRE(NParseSz("April", pmMon) == 4);
  REQUIRE(NParseSz("May", pmMon) == 5);
  REQUIRE(NParseSz("June", pmMon) == 6);
  REQUIRE(NParseSz("July", pmMon) == 7);
  REQUIRE(NParseSz("August", pmMon) == 8);
  REQUIRE(NParseSz("September", pmMon) == 9);
  REQUIRE(NParseSz("October", pmMon) == 10);
  REQUIRE(NParseSz("November", pmMon) == 11);
  REQUIRE(NParseSz("December", pmMon) == 12);
}

TEST_CASE("NParseSz pmMon abbreviations", "[parsing]") {
  REQUIRE(NParseSz("Jan", pmMon) == 1);
  REQUIRE(NParseSz("Feb", pmMon) == 2);
  REQUIRE(NParseSz("Dec", pmMon) == 12);
}

TEST_CASE("NParseSz pmMon case insensitive", "[parsing]") {
  REQUIRE(NParseSz("january", pmMon) == 1);
  REQUIRE(NParseSz("MARCH", pmMon) == 3);
  REQUIRE(NParseSz("jun", pmMon) == 6);
}

TEST_CASE("NParseSz pmMon numeric", "[parsing]") {
  REQUIRE(NParseSz("1", pmMon) == 1);
  REQUIRE(NParseSz("12", pmMon) == 12);
}

// ---- pmSign: Zodiac sign parsing ----

TEST_CASE("NParseSz pmSign full sign names", "[parsing]") {
  REQUIRE(NParseSz("Aries", pmSign) == 1);
  REQUIRE(NParseSz("Taurus", pmSign) == 2);
  REQUIRE(NParseSz("Gemini", pmSign) == 3);
  REQUIRE(NParseSz("Cancer", pmSign) == 4);
  REQUIRE(NParseSz("Leo", pmSign) == 5);
  REQUIRE(NParseSz("Virgo", pmSign) == 6);
  REQUIRE(NParseSz("Libra", pmSign) == 7);
  REQUIRE(NParseSz("Scorpio", pmSign) == 8);
  REQUIRE(NParseSz("Sagittarius", pmSign) == 9);
  REQUIRE(NParseSz("Capricorn", pmSign) == 10);
  REQUIRE(NParseSz("Aquarius", pmSign) == 11);
  REQUIRE(NParseSz("Pisces", pmSign) == 12);
}

TEST_CASE("NParseSz pmSign abbreviations", "[parsing]") {
  REQUIRE(NParseSz("Ari", pmSign) == 1);
  REQUIRE(NParseSz("Tau", pmSign) == 2);
  REQUIRE(NParseSz("Sco", pmSign) == 8);
  REQUIRE(NParseSz("Pis", pmSign) == 12);
}

// ---- pmObject: Planet/object parsing ----

TEST_CASE("NParseSz pmObject planets", "[parsing]") {
  // Earth=0, Sun=1, Moon=2, Mercury=3, etc. per szObjName[] in data.cpp
  REQUIRE(NParseSz("Earth", pmObject) == 0);
  REQUIRE(NParseSz("Sun", pmObject) == oSun);    // 1
  REQUIRE(NParseSz("Moon", pmObject) == oMoo);    // 2
  REQUIRE(NParseSz("Mercury", pmObject) == oMer); // 3
  REQUIRE(NParseSz("Venus", pmObject) == oVen);   // 4
  REQUIRE(NParseSz("Mars", pmObject) == oMar);    // 5
  REQUIRE(NParseSz("Jupiter", pmObject) == oJup); // 6
  REQUIRE(NParseSz("Saturn", pmObject) == oSat);  // 7
  REQUIRE(NParseSz("Uranus", pmObject) == oUra);  // 8
  REQUIRE(NParseSz("Neptune", pmObject) == oNep); // 9
  REQUIRE(NParseSz("Pluto", pmObject) == oPlu);   // 10
}

TEST_CASE("NParseSz pmObject abbreviations", "[parsing]") {
  REQUIRE(NParseSz("Jup", pmObject) == oJup);
  REQUIRE(NParseSz("Sat", pmObject) == oSat);
  REQUIRE(NParseSz("Nep", pmObject) == oNep);
}

// ---- pmColor: Color name parsing ----

TEST_CASE("NParseSz pmColor names", "[parsing]") {
  REQUIRE(NParseSz("Black", pmColor) == 0);
  REQUIRE(NParseSz("Red", pmColor) == 9);
  REQUIRE(NParseSz("White", pmColor) == 15);
  REQUIRE(NParseSz("Green", pmColor) == 10);
  REQUIRE(NParseSz("Blue", pmColor) == 12);
}

TEST_CASE("NParseSz pmColor numeric", "[parsing]") {
  REQUIRE(NParseSz("0", pmColor) == 0);
  REQUIRE(NParseSz("7", pmColor) == 7);
  REQUIRE(NParseSz("15", pmColor) == 15);
}

// ---- pmRGB: RGB color parsing ----

TEST_CASE("NParseSz pmRGB hex format", "[parsing]") {
  int kv = NParseSz("#ff0000", pmRGB);
  REQUIRE(RgbR(kv) == 255);
  REQUIRE(RgbG(kv) == 0);
  REQUIRE(RgbB(kv) == 0);
}

TEST_CASE("NParseSz pmRGB hex green", "[parsing]") {
  int kv = NParseSz("#00ff00", pmRGB);
  REQUIRE(RgbR(kv) == 0);
  REQUIRE(RgbG(kv) == 255);
  REQUIRE(RgbB(kv) == 0);
}

TEST_CASE("NParseSz pmRGB csv format", "[parsing]") {
  int kv = NParseSz("255,128,0", pmRGB);
  REQUIRE(RgbR(kv) == 255);
  REQUIRE(RgbG(kv) == 128);
  REQUIRE(RgbB(kv) == 0);
}

// ---- pmDst: Daylight Saving Time parsing ----

TEST_CASE("RParseSz pmDst daylight values", "[parsing]") {
  REQUIRE(RParseSz("Yes", pmDst) == Approx(1.0));
  REQUIRE(RParseSz("Y", pmDst) == Approx(1.0));
  REQUIRE(RParseSz("Daylight", pmDst) == Approx(1.0));
  REQUIRE(RParseSz("True", pmDst) == Approx(1.0));
}

TEST_CASE("RParseSz pmDst standard values", "[parsing]") {
  REQUIRE(RParseSz("No", pmDst) == Approx(0.0));
  REQUIRE(RParseSz("N", pmDst) == Approx(0.0));
  REQUIRE(RParseSz("Standard", pmDst) == Approx(0.0));
  REQUIRE(RParseSz("False", pmDst) == Approx(0.0));
}

TEST_CASE("RParseSz pmDst autodetect", "[parsing]") {
  REQUIRE(RParseSz("Autodetect", pmDst) == Approx(dstAuto));
  REQUIRE(RParseSz("A", pmDst) == Approx(dstAuto));
  REQUIRE(dstAuto == 24.0);
}

// ---- pmZon: Time zone parsing ----

TEST_CASE("RParseSz pmZon named zones", "[parsing]") {
  REQUIRE(RParseSz("EST", pmZon) == Approx(5.0));
  REQUIRE(RParseSz("PST", pmZon) == Approx(8.0));
  REQUIRE(RParseSz("GMT", pmZon) == Approx(0.0));
}

TEST_CASE("RParseSz pmZon numeric", "[parsing]") {
  REQUIRE(RParseSz("8", pmZon) == Approx(8.0));
  REQUIRE(RParseSz("0", pmZon) == Approx(0.0));
}

// ---- pmTim: Time parsing ----

TEST_CASE("RParseSz pmTim keywords", "[parsing]") {
  REQUIRE(RParseSz("Noon", pmTim) == Approx(12.0));
  REQUIRE(RParseSz("N", pmTim) == Approx(12.0));
  REQUIRE(RParseSz("Midnight", pmTim) == Approx(0.0));
  REQUIRE(RParseSz("M", pmTim) == Approx(0.0));
}

TEST_CASE("RParseSz pmTim colon format", "[parsing]") {
  REQUIRE(RParseSz("14:30", pmTim) == Approx(14.5));
  REQUIRE(RParseSz("0:00", pmTim) == Approx(0.0));
  REQUIRE(RParseSz("23:00", pmTim) == Approx(23.0));
}

TEST_CASE("RParseSz pmTim with seconds", "[parsing]") {
  // 12:30:45 = 12 + 30/60 + 45/3600 = 12.5125
  REQUIRE(RParseSz("12:30:45", pmTim) == Approx(12.5125).margin(0.001));
}

// ---- pmLon: Longitude parsing ----

TEST_CASE("RParseSz pmLon with direction", "[parsing]") {
  // "122W30" = 122 deg 30 min West = 122.5
  REQUIRE(RParseSz("122W30", pmLon) == Approx(122.5).margin(0.01));
}

TEST_CASE("RParseSz pmLon east negative", "[parsing]") {
  // "105E30" = 105 deg 30 min East = -105.5
  REQUIRE(RParseSz("105E30", pmLon) == Approx(-105.5).margin(0.01));
}

TEST_CASE("RParseSz pmLon colon format", "[parsing]") {
  REQUIRE(RParseSz("122:30", pmLon) == Approx(122.5).margin(0.01));
}

// ---- pmLat: Latitude parsing ----

TEST_CASE("RParseSz pmLat with direction", "[parsing]") {
  // "47N36" = 47 deg 36 min North = 47.6
  REQUIRE(RParseSz("47N36", pmLat) == Approx(47.6).margin(0.01));
}

TEST_CASE("RParseSz pmLat south negative", "[parsing]") {
  // "33S51" = 33 deg 51 min South = -33.85
  REQUIRE(RParseSz("33S51", pmLat) == Approx(-33.85).margin(0.01));
}

// ---- pmYea: Year with BCE ----

TEST_CASE("NParseSz pmYea BCE", "[parsing]") {
  // "5BC" -> year -4 (no year 0: 1BC=0, 2BC=-1, ..., 5BC=-4)
  REQUIRE(NParseSz("5BC", pmYea) == -4);
  REQUIRE(NParseSz("1BC", pmYea) == 0);
}

TEST_CASE("NParseSz pmYea normal", "[parsing]") {
  REQUIRE(NParseSz("2025", pmYea) == 2025);
  REQUIRE(NParseSz("1900", pmYea) == 1900);
}
