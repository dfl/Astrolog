/*
** Unit tests for format→parse round-trips.
** These are the most important tests for the dialog bug fix: they verify
** that coordinates formatted for display can be parsed back to recover
** the original values.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"
#include "xdialog.h"

#include <cstring>

struct UsSaver {
  int fAnsiChar, fSeconds, fSecond1K, nDegForm, fRound, fGraphics, nCharset;
  int fEuroDate, fEuroTime;
  UsSaver() :
    fAnsiChar(us.fAnsiChar), fSeconds(us.fSeconds), fSecond1K(us.fSecond1K),
    nDegForm(us.nDegForm), fRound(us.fRound), fGraphics(us.fGraphics),
    nCharset(us.nCharset), fEuroDate(us.fEuroDate), fEuroTime(us.fEuroTime) {}
  ~UsSaver() {
    us.fAnsiChar = fAnsiChar; us.fSeconds = fSeconds;
    us.fSecond1K = fSecond1K; us.nDegForm = nDegForm;
    us.fRound = fRound; us.fGraphics = fGraphics;
    us.nCharset = nCharset; us.fEuroDate = fEuroDate;
    us.fEuroTime = fEuroTime;
  }
};

static void SetDefaults() {
  us.fAnsiChar = fFalse;
  us.fSeconds  = fFalse;
  us.fSecond1K = fFalse;
  us.nDegForm  = 0;
  us.fRound    = fFalse;
  us.fGraphics = fFalse;
  us.fEuroDate = fFalse;
  us.fEuroTime = fFalse;
}

// ---- Coordinate round-trips (the core bug-fix regression test) ----

TEST_CASE("Lon round-trip: FormatLon -> ParseCoordFields default mode", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();

  // Seattle: 122.5 W
  char szLon[32];
  FormatLon(szLon, sizeof(szLon), 122.5);

  real lon = 0.0;
  ParseCoordFields(szLon, NULL, &lon, NULL);
  REQUIRE(lon == Approx(122.5).margin(0.02));
}

TEST_CASE("Lat round-trip: FormatLat -> ParseCoordFields default mode", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();

  char szLat[32];
  FormatLat(szLat, sizeof(szLat), 47.5);

  real lat = 0.0;
  ParseCoordFields(NULL, szLat, NULL, &lat);
  REQUIRE(lat == Approx(47.5).margin(0.02));
}

TEST_CASE("Lon/Lat round-trip with negative coords (East/South)", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();

  // Buenos Aires approx: 58.5W → but testing East (negative lon)
  char szLon[32], szLat[32];
  FormatLon(szLon, sizeof(szLon), -73.5);  // East
  FormatLat(szLat, sizeof(szLat), -33.5);  // South

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(szLon, szLat, &lon, &lat);
  REQUIRE(lon == Approx(-73.5).margin(0.02));
  REQUIRE(lat == Approx(-33.5).margin(0.02));
}

TEST_CASE("Lon/Lat round-trip in seconds mode", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  char szLon[32], szLat[32];
  FormatLon(szLon, sizeof(szLon), 122.5);
  FormatLat(szLat, sizeof(szLat), 47.5);

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(szLon, szLat, &lon, &lat);
  REQUIRE(lon == Approx(122.5).margin(0.002));
  REQUIRE(lat == Approx(47.5).margin(0.002));
}

TEST_CASE("Lon/Lat round-trip in seconds+1K mode", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds  = fTrue;
  us.fSecond1K = fTrue;

  char szLon[32], szLat[32];
  FormatLon(szLon, sizeof(szLon), 122.5);
  FormatLat(szLat, sizeof(szLat), 47.5);

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(szLon, szLat, &lon, &lat);
  REQUIRE(lon == Approx(122.5).margin(0.0001));
  REQUIRE(lat == Approx(47.5).margin(0.0001));
}

// The original bug: lon and lat values were swapped or duplicated when
// the dialog read back what it displayed. This test uses distinct values
// to ensure each field contains the correct coordinate.

TEST_CASE("Lon and Lat are not swapped after format/parse", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();

  // Use very different values so a swap is obvious
  char szLon[32], szLat[32];
  FormatLon(szLon, sizeof(szLon), 150.0);  // Lon = 150 W
  FormatLat(szLat, sizeof(szLat), 10.0);   // Lat = 10 N

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(szLon, szLat, &lon, &lat);

  // lon should be ~150, NOT ~10
  REQUIRE(lon == Approx(150.0).margin(0.02));
  // lat should be ~10, NOT ~150
  REQUIRE(lat == Approx(10.0).margin(0.02));
}

TEST_CASE("Lon and Lat not swapped in seconds mode", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  char szLon[32], szLat[32];
  FormatLon(szLon, sizeof(szLon), 150.0);
  FormatLat(szLat, sizeof(szLat), 10.0);

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(szLon, szLat, &lon, &lat);

  REQUIRE(lon == Approx(150.0).margin(0.002));
  REQUIRE(lat == Approx(10.0).margin(0.002));
}

// ---- Zero coordinates ----

TEST_CASE("Zero lon/lat round-trip", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();

  char szLon[32], szLat[32];
  FormatLon(szLon, sizeof(szLon), 0.0);
  FormatLat(szLat, sizeof(szLat), 0.0);

  real lon = 99.0, lat = 99.0;
  ParseCoordFields(szLon, szLat, &lon, &lat);
  REQUIRE(lon == Approx(0.0).margin(0.02));
  REQUIRE(lat == Approx(0.0).margin(0.02));
}

// ---- FormatCI -> ParseCoordFields integration ----

TEST_CASE("FormatCI lon/lat fields parse back correctly", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();

  char nam[] = "Test";
  char loc[] = "NYC";
  CI ci = { 6, 15, 2025, 12.0, 1.0, 5.0, -74.0, 40.75, nam, loc };
  CIS cis;
  memset(&cis, 0, sizeof(cis));
  FormatCI(&ci, &cis);

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(cis.szLon, cis.szLat, &lon, &lat);
  REQUIRE(lon == Approx(-74.0).margin(0.02));
  REQUIRE(lat == Approx(40.75).margin(0.02));
}

// ---- Date format/parse round-trip ----

TEST_CASE("FormatMon -> ParseDateFields month round-trip", "[roundtrip]") {
  for (int m = 1; m <= 12; m++) {
    char szMon[8];
    FormatMon(szMon, sizeof(szMon), m);

    int mon = 0;
    ParseDateFields(szMon, NULL, NULL, &mon, NULL, NULL);
    REQUIRE(mon == m);
  }
}

// ---- Time format/parse round-trip ----

TEST_CASE("FormatTim -> ParseTimeField round-trip", "[roundtrip]") {
  UsSaver sav;
  SetDefaults();
  us.fEuroTime = fTrue;  // 24-hour format for cleaner parsing

  // Test a few representative times
  real times[] = { 0.0, 6.5, 12.0, 18.75, 23.0 };
  for (real t : times) {
    char szTim[16];
    FormatTim(szTim, sizeof(szTim), t);

    real parsed = 0.0;
    ParseTimeField(szTim, &parsed);
    REQUIRE(parsed == Approx(t).margin(0.02));
  }
}
