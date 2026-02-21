/*
** Unit tests for SzLocation(), FormatLon(), and FormatLat().
** Uses Catch2 v2 single-header framework.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"
#include "xdialog.h"

#include <cstring>

// Helper to save and restore the US globals we modify in tests.
struct UsSaver {
  int fAnsiChar, fSeconds, fSecond1K, nDegForm, fRound, fGraphics, nCharset;
  UsSaver() :
    fAnsiChar(us.fAnsiChar), fSeconds(us.fSeconds), fSecond1K(us.fSecond1K),
    nDegForm(us.nDegForm), fRound(us.fRound), fGraphics(us.fGraphics),
    nCharset(us.nCharset) {}
  ~UsSaver() {
    us.fAnsiChar = fAnsiChar; us.fSeconds = fSeconds;
    us.fSecond1K = fSecond1K; us.nDegForm = nDegForm;
    us.fRound = fRound; us.fGraphics = fGraphics;
    us.nCharset = nCharset;
  }
};

// Set a known baseline for location formatting.
static void SetDefaults() {
  us.fAnsiChar = fFalse;
  us.fSeconds  = fFalse;
  us.fSecond1K = fFalse;
  us.nDegForm  = 0;       // Normal degrees
  us.fRound    = fFalse;
  us.fGraphics = fFalse;
}

// ---- SzLocation basic output ----

TEST_CASE("SzLocation default mode", "[location]") {
  UsSaver sav;
  SetDefaults();

  // 122.5 = 122 deg 30 min W, 47.5 = 47 deg 30 min N
  const char *sz = SzLocation(122.5, 47.5);
  REQUIRE(std::string(sz) == "122:30W 47:30N");
}

TEST_CASE("SzLocation negative coords (East/South)", "[location]") {
  UsSaver sav;
  SetDefaults();

  // Negative lon → East, negative lat → South
  const char *sz = SzLocation(-73.5, -33.5);
  REQUIRE(std::string(sz) == " 73:30E 33:30S");
}

TEST_CASE("SzLocation zero coordinates", "[location]") {
  UsSaver sav;
  SetDefaults();

  const char *sz = SzLocation(0.0, 0.0);
  REQUIRE(std::string(sz) == "  0:00W  0:00N");
}

// ---- SzLocation seconds mode ----

TEST_CASE("SzLocation seconds mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  // 122 deg 30 min 0 sec, 47 deg 30 min 0 sec
  const char *sz = SzLocation(122.5, 47.5);
  REQUIRE(std::string(sz) == "122:30'00W 47:30'00N");
}

// ---- SzLocation seconds+1K mode ----

TEST_CASE("SzLocation seconds+1K mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds  = fTrue;
  us.fSecond1K = fTrue;

  const char *sz = SzLocation(122.5, 47.5);
  REQUIRE(std::string(sz) == "122:30'00.000W 47:30'00.000N");
}

// ---- SzLocation 360-degree format ----

TEST_CASE("SzLocation 360-degree format", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.nDegForm = df360;

  const char *sz = SzLocation(122.5, 47.5);
  REQUIRE(std::string(sz) == "122.50W 47.50N");
}

TEST_CASE("SzLocation 360-degree format with seconds", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.nDegForm  = df360;
  us.fSeconds  = fTrue;

  const char *sz = SzLocation(122.5, 47.5);
  REQUIRE(std::string(sz) == "122.50000W 47.50000N");
}

// ---- FormatLon extraction ----

TEST_CASE("FormatLon default mode", "[location]") {
  UsSaver sav;
  SetDefaults();

  char buf[32];
  FormatLon(buf, sizeof(buf), 122.5);
  REQUIRE(std::string(buf) == "122:30W");
}

TEST_CASE("FormatLon seconds mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  char buf[32];
  FormatLon(buf, sizeof(buf), 122.5);
  REQUIRE(std::string(buf) == "122:30'00W");
}

TEST_CASE("FormatLon seconds+1K mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds  = fTrue;
  us.fSecond1K = fTrue;

  char buf[32];
  FormatLon(buf, sizeof(buf), 122.5);
  REQUIRE(std::string(buf) == "122:30'00.000W");
}

TEST_CASE("FormatLon east (negative)", "[location]") {
  UsSaver sav;
  SetDefaults();

  char buf[32];
  FormatLon(buf, sizeof(buf), -73.5);
  REQUIRE(std::string(buf) == " 73:30E");
}

// ---- FormatLat extraction ----

TEST_CASE("FormatLat default mode", "[location]") {
  UsSaver sav;
  SetDefaults();

  char buf[32];
  FormatLat(buf, sizeof(buf), 47.5);
  REQUIRE(std::string(buf) == "47:30N");
}

TEST_CASE("FormatLat seconds mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  char buf[32];
  FormatLat(buf, sizeof(buf), 47.5);
  REQUIRE(std::string(buf) == "47:30'00N");
}

TEST_CASE("FormatLat seconds+1K mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds  = fTrue;
  us.fSecond1K = fTrue;

  char buf[32];
  FormatLat(buf, sizeof(buf), 47.5);
  REQUIRE(std::string(buf) == "47:30'00.000N");
}

TEST_CASE("FormatLat south (negative)", "[location]") {
  UsSaver sav;
  SetDefaults();

  char buf[32];
  FormatLat(buf, sizeof(buf), -33.5);
  REQUIRE(std::string(buf) == "33:30S");
}

// ---- FormatLon/FormatLat round-trip consistency ----

TEST_CASE("FormatLon + FormatLat reconstruct SzLocation", "[location]") {
  UsSaver sav;
  SetDefaults();

  char lon[32], lat[32];
  FormatLon(lon, sizeof(lon), 122.5);
  FormatLat(lat, sizeof(lat), 47.5);

  // SzLocation produces "122:30W 47:30N"; FormatLon/FormatLat split that
  // into "122:30W" and "47:30N" (without leading space on lat).
  std::string reconstructed = std::string(lon) + " " + lat;

  SetDefaults();
  const char *full = SzLocation(122.5, 47.5);
  // The full string has a leading space before lat (" 47:30N") due to %3.0f,
  // but FormatLat strips it, so we compare trimmed.
  std::string fullStr(full);
  // Trim internal extra spaces for comparison
  REQUIRE(reconstructed == "122:30W 47:30N");
  REQUIRE(fullStr == "122:30W 47:30N");
}

TEST_CASE("FormatLon + FormatLat consistency in seconds mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds = fTrue;

  char lon[32], lat[32];
  FormatLon(lon, sizeof(lon), 122.5);
  FormatLat(lat, sizeof(lat), 47.5);

  REQUIRE(std::string(lon) == "122:30'00W");
  REQUIRE(std::string(lat) == "47:30'00N");
}

TEST_CASE("FormatLon + FormatLat consistency in seconds+1K mode", "[location]") {
  UsSaver sav;
  SetDefaults();
  us.fSeconds  = fTrue;
  us.fSecond1K = fTrue;

  char lon[32], lat[32];
  FormatLon(lon, sizeof(lon), 122.5);
  FormatLat(lat, sizeof(lat), 47.5);

  REQUIRE(std::string(lon) == "122:30'00.000W");
  REQUIRE(std::string(lat) == "47:30'00.000N");
}
