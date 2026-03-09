/*
** Unit tests for dialog helper functions in xdialog.cpp:
** ParseCoordFields, ParseDateFields, ParseTimeField, ParseZoneFields,
** FormatDst, FormatZon, FormatTim, and FormatCI edge cases.
**
** These functions are the bridge between FLTK dialog widgets and Astrolog's
** internal data structures. Bugs here cause silent data corruption.
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

// ===========================================================================
// ParseDateFields
// ===========================================================================

TEST_CASE("ParseDateFields month names", "[dialog]") {
  int mon = 0, day = 0, yea = 0;
  ParseDateFields("Jun", "15", "2025", &mon, &day, &yea);
  REQUIRE(mon == 6);
  REQUIRE(day == 15);
  REQUIRE(yea == 2025);
}

TEST_CASE("ParseDateFields full month name", "[dialog]") {
  int mon = 0;
  ParseDateFields("December", NULL, NULL, &mon, NULL, NULL);
  REQUIRE(mon == 12);
}

TEST_CASE("ParseDateFields numeric month", "[dialog]") {
  int mon = 0;
  ParseDateFields("3", NULL, NULL, &mon, NULL, NULL);
  REQUIRE(mon == 3);
}

TEST_CASE("ParseDateFields negative year", "[dialog]") {
  int yea = 0;
  ParseDateFields(NULL, NULL, "-500", NULL, NULL, &yea);
  REQUIRE(yea == -500);
}

TEST_CASE("ParseDateFields BCE year", "[dialog]") {
  int yea = 0;
  ParseDateFields(NULL, NULL, "5BC", NULL, NULL, &yea);
  REQUIRE(yea == -4);
}

TEST_CASE("ParseDateFields partial NULL args", "[dialog]") {
  // Should handle NULL pointers for unused fields without crashing
  int mon = 99;
  ParseDateFields("Jan", NULL, NULL, &mon, NULL, NULL);
  REQUIRE(mon == 1);
}

// ===========================================================================
// ParseTimeField
// ===========================================================================

TEST_CASE("ParseTimeField noon keyword", "[dialog]") {
  real tim = -1.0;
  ParseTimeField("Noon", &tim);
  REQUIRE(tim == Approx(12.0));
}

TEST_CASE("ParseTimeField midnight keyword", "[dialog]") {
  real tim = -1.0;
  ParseTimeField("Midnight", &tim);
  REQUIRE(tim == Approx(0.0));
}

TEST_CASE("ParseTimeField colon format", "[dialog]") {
  real tim = -1.0;
  ParseTimeField("14:30", &tim);
  REQUIRE(tim == Approx(14.5));
}

TEST_CASE("ParseTimeField NULL does not modify", "[dialog]") {
  real tim = 99.0;
  ParseTimeField(NULL, &tim);
  REQUIRE(tim == Approx(99.0));
}

// ===========================================================================
// ParseZoneFields (DST + Zone together)
// ===========================================================================

TEST_CASE("ParseZoneFields named zone", "[dialog]") {
  real dst = -1.0, zon = -1.0;
  ParseZoneFields("No", "PST", &dst, &zon);
  REQUIRE(dst == Approx(0.0));
  REQUIRE(zon == Approx(8.0));
}

TEST_CASE("ParseZoneFields daylight saving", "[dialog]") {
  real dst = -1.0;
  ParseZoneFields("Yes", NULL, &dst, NULL);
  REQUIRE(dst == Approx(1.0));
}

TEST_CASE("ParseZoneFields autodetect DST", "[dialog]") {
  real dst = -1.0;
  ParseZoneFields("Autodetect", NULL, &dst, NULL);
  REQUIRE(dst == Approx(dstAuto));
}

TEST_CASE("ParseZoneFields numeric zone", "[dialog]") {
  real zon = -1.0;
  ParseZoneFields(NULL, "5", NULL, &zon);
  REQUIRE(zon == Approx(5.0));
}

TEST_CASE("ParseZoneFields NULL does not modify", "[dialog]") {
  real dst = 99.0, zon = 88.0;
  ParseZoneFields(NULL, NULL, &dst, &zon);
  REQUIRE(dst == Approx(99.0));
  REQUIRE(zon == Approx(88.0));
}

// ===========================================================================
// ParseCoordFields
// ===========================================================================

TEST_CASE("ParseCoordFields west/north", "[dialog]") {
  real lon = 0.0, lat = 0.0;
  ParseCoordFields("122:30W", "47:30N", &lon, &lat);
  REQUIRE(lon == Approx(122.5).margin(0.01));
  REQUIRE(lat == Approx(47.5).margin(0.01));
}

TEST_CASE("ParseCoordFields east/south", "[dialog]") {
  real lon = 0.0, lat = 0.0;
  ParseCoordFields("73:30E", "33:30S", &lon, &lat);
  REQUIRE(lon == Approx(-73.5).margin(0.01));
  REQUIRE(lat == Approx(-33.5).margin(0.01));
}

TEST_CASE("ParseCoordFields NULL fields ignored", "[dialog]") {
  real lon = 99.0, lat = 88.0;
  ParseCoordFields(NULL, NULL, &lon, &lat);
  REQUIRE(lon == Approx(99.0));
  REQUIRE(lat == Approx(88.0));
}

// ===========================================================================
// FormatDst — uses SzZone (same as zone but without negation)
// ===========================================================================

TEST_CASE("FormatDst standard", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char buf[16];
  FormatDst(buf, sizeof(buf), 0.0);
  REQUIRE(std::string(buf) == "0W");
}

TEST_CASE("FormatDst daylight", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char buf[16];
  FormatDst(buf, sizeof(buf), 1.0);
  REQUIRE(std::string(buf) == "1W");
}

TEST_CASE("FormatDst LMT special value", "[dialog]") {
  char buf[16];
  FormatDst(buf, sizeof(buf), zonLMT);
  REQUIRE(std::string(buf) == "LMT");
}

TEST_CASE("FormatDst LAT special value", "[dialog]") {
  char buf[16];
  FormatDst(buf, sizeof(buf), zonLAT);
  REQUIRE(std::string(buf) == "LAT");
}

// ===========================================================================
// FormatZon — negates the value before calling SzZone
// ===========================================================================

TEST_CASE("FormatZon PST", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char buf[16];
  // PST = +8 hours west of UTC; FormatZon(-zon) -> SzZone(-8) -> "8E"
  // Wait, FormatZon does snprintf(sz, ..., SzZone(-zon))
  // For zon=8.0 (PST): SzZone(-8.0) -> "8E"
  FormatZon(buf, sizeof(buf), 8.0);
  REQUIRE(std::string(buf) == "8E");
}

TEST_CASE("FormatZon UTC", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char buf[16];
  FormatZon(buf, sizeof(buf), 0.0);
  REQUIRE(std::string(buf) == "0W");
}

TEST_CASE("FormatZon east of UTC", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char buf[16];
  // IST = -5.5 (east): FormatZon calls SzZone(5.5) -> "5:30W"
  FormatZon(buf, sizeof(buf), -5.5);
  REQUIRE(std::string(buf) == "5:30W");
}

// ===========================================================================
// FormatCI -> Parse round-trips (the critical fdialog.cpp data path)
// ===========================================================================

TEST_CASE("FormatCI round-trip all fields", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char nam[] = "Test";
  char loc[] = "Seattle";
  CI ci = { 6, 15, 2025, 14.5, 1.0, 8.0, 122.5, 47.5, nam, loc };
  CIS cis;
  memset(&cis, 0, sizeof(cis));
  FormatCI(&ci, &cis);

  // Parse back
  int mon = 0, day = 0, yea = 0;
  real tim = 0.0, dst = 0.0, zon = 0.0, lon = 0.0, lat = 0.0;

  ParseDateFields(cis.szMon, cis.szDay, cis.szYea, &mon, &day, &yea);
  ParseTimeField(cis.szTim, &tim);
  ParseZoneFields(cis.szDst, cis.szZon, &dst, &zon);
  ParseCoordFields(cis.szLon, cis.szLat, &lon, &lat);

  REQUIRE(mon == 6);
  REQUIRE(day == 15);
  REQUIRE(yea == 2025);
  REQUIRE(tim == Approx(14.5).margin(0.02));
  REQUIRE(dst == Approx(1.0).margin(0.01));
  REQUIRE(lon == Approx(122.5).margin(0.02));
  REQUIRE(lat == Approx(47.5).margin(0.02));
}

TEST_CASE("FormatCI round-trip negative coords", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char nam[] = "Test";
  char loc[] = "Tokyo";
  // Tokyo: 139.7E, 35.7N -> lon=-139.7 in Astrolog convention
  CI ci = { 1, 1, 2000, 0.0, 0.0, -9.0, -139.7, 35.7, nam, loc };
  CIS cis;
  memset(&cis, 0, sizeof(cis));
  FormatCI(&ci, &cis);

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(cis.szLon, cis.szLat, &lon, &lat);

  REQUIRE(lon == Approx(-139.7).margin(0.02));
  REQUIRE(lat == Approx(35.7).margin(0.02));
}

TEST_CASE("FormatCI round-trip zero DST", "[dialog]") {
  UsSaver sav;
  SetDefaults();

  char nam[] = "";
  char loc[] = "";
  CI ci = { 3, 1, 2025, 12.0, 0.0, 0.0, 0.0, 0.0, nam, loc };
  CIS cis;
  memset(&cis, 0, sizeof(cis));
  FormatCI(&ci, &cis);

  real dst = -1.0;
  ParseZoneFields(cis.szDst, NULL, &dst, NULL);
  REQUIRE(dst == Approx(0.0).margin(0.01));
}

TEST_CASE("FormatCI round-trip seconds mode", "[dialog]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  char nam[] = "Test";
  char loc[] = "Seattle";
  CI ci = { 6, 15, 2025, 14.5, 0.0, 8.0, 122.5, 47.5, nam, loc };
  CIS cis;
  memset(&cis, 0, sizeof(cis));
  FormatCI(&ci, &cis);

  real lon = 0.0, lat = 0.0;
  ParseCoordFields(cis.szLon, cis.szLat, &lon, &lat);
  REQUIRE(lon == Approx(122.5).margin(0.002));
  REQUIRE(lat == Approx(47.5).margin(0.002));
}
