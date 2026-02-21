/*
** Unit tests for formatting functions: SzDate, SzTim, SzZone,
** FormatMon/Day/Yea/Tim, FormatCI, FormatRGB/KvParseRGB.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"
#include "xdialog.h"

#include <cstring>

// Helper to save and restore globals we modify in tests.
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

// ---- SzDate ----

TEST_CASE("SzDate US formats", "[formatting]") {
  UsSaver sav;
  SetDefaults();

  // nFormat 0 (default): MM/DD/YYYY
  REQUIRE(std::string(SzDate(6, 15, 2025, 0)) == " 6/15/2025");

  // nFormat 1: Month DD, YYYY
  REQUIRE(std::string(SzDate(1, 1, 2025, 1)) == "January 1, 2025");

  // nFormat 2: Mon DD YYYY
  REQUIRE(std::string(SzDate(12, 25, 2025, 2)) == "Dec 25 2025");

  // nFormat 3: Mon DD, YYYY
  REQUIRE(std::string(SzDate(3, 5, 2000, 3)) == "Mar  5, 2000");

  // nFormat -1: MM/DD/YY
  REQUIRE(std::string(SzDate(6, 15, 2025, -1)) == " 6/15/25");
}

TEST_CASE("SzDate Euro formats", "[formatting]") {
  UsSaver sav;
  SetDefaults();
  us.fEuroDate = fTrue;

  // nFormat 0 (default): DD-MM-YYYY
  REQUIRE(std::string(SzDate(6, 15, 2025, 0)) == "15- 6-2025");

  // nFormat 2: DD Mon YYYY
  REQUIRE(std::string(SzDate(12, 25, 2025, 2)) == "25 Dec 2025");
}

// ---- SzTim ----

TEST_CASE("SzTim 12-hour format", "[formatting]") {
  UsSaver sav;
  SetDefaults();  // fEuroTime = false → 12-hour with am/pm

  // Noon
  std::string s = SzTim(12.0);
  REQUIRE(s.find("12:00") != std::string::npos);
  REQUIRE(s.find('p') != std::string::npos);

  // Midnight
  s = SzTim(0.0);
  REQUIRE(s.find("12:00") != std::string::npos);
  REQUIRE(s.find('a') != std::string::npos);

  // 3:30 PM = 15.5
  s = SzTim(15.5);
  REQUIRE(s.find("3:30") != std::string::npos);
  REQUIRE(s.find('p') != std::string::npos);
}

TEST_CASE("SzTim 24-hour format", "[formatting]") {
  UsSaver sav;
  SetDefaults();
  us.fEuroTime = fTrue;

  std::string s = SzTim(15.5);
  REQUIRE(s.find("15:30") != std::string::npos);
  // No am/pm marker
  REQUIRE(s.find('a') == std::string::npos);
  REQUIRE(s.find('p') == std::string::npos);
}

// ---- SzZone ----

TEST_CASE("SzZone whole hour zones", "[formatting]") {
  UsSaver sav;
  SetDefaults();

  // US Pacific: 8W
  REQUIRE(std::string(SzZone(8.0)) == "8W");

  // East of UTC: negative → E
  REQUIRE(std::string(SzZone(-5.5)) == "5:30E");

  // UTC
  REQUIRE(std::string(SzZone(0.0)) == "0W");
}

TEST_CASE("SzZone special values", "[formatting]") {
  REQUIRE(std::string(SzZone(zonLMT)) == "LMT");
  REQUIRE(std::string(SzZone(zonLAT)) == "LAT");
}

TEST_CASE("SzZone fractional hour", "[formatting]") {
  UsSaver sav;
  SetDefaults();

  // India: UTC+5:30 → stored as -5.5 (East)
  std::string s = SzZone(-5.5);
  REQUIRE(s == "5:30E");
}

// ---- FormatMon ----

TEST_CASE("FormatMon abbreviates months", "[formatting]") {
  char buf[16];
  FormatMon(buf, sizeof(buf), 1);
  REQUIRE(std::string(buf) == "Jan");

  FormatMon(buf, sizeof(buf), 12);
  REQUIRE(std::string(buf) == "Dec");
}

TEST_CASE("FormatMon out of range falls back to number", "[formatting]") {
  char buf[16];
  FormatMon(buf, sizeof(buf), 0);
  REQUIRE(std::string(buf) == "0");

  FormatMon(buf, sizeof(buf), 13);
  REQUIRE(std::string(buf) == "13");
}

// ---- FormatDay / FormatYea ----

TEST_CASE("FormatDay and FormatYea produce numeric strings", "[formatting]") {
  char buf[16];

  FormatDay(buf, sizeof(buf), 1);
  REQUIRE(std::string(buf) == "1");

  FormatDay(buf, sizeof(buf), 31);
  REQUIRE(std::string(buf) == "31");

  FormatYea(buf, sizeof(buf), 2025);
  REQUIRE(std::string(buf) == "2025");

  FormatYea(buf, sizeof(buf), -500);
  REQUIRE(std::string(buf) == "-500");
}

// ---- FormatCI integration ----

TEST_CASE("FormatCI fills all fields", "[formatting]") {
  UsSaver sav;
  SetDefaults();

  char nam[] = "Test";
  char loc[] = "Seattle";
  CI ci = { 6, 15, 2025, 12.0, 1.0, 8.0, 122.5, 47.5, nam, loc };
  CIS cis;
  memset(&cis, 0, sizeof(cis));

  FormatCI(&ci, &cis);

  REQUIRE(std::string(cis.szMon) == "Jun");
  REQUIRE(std::string(cis.szDay) == "15");
  REQUIRE(std::string(cis.szYea) == "2025");
  REQUIRE(std::string(cis.szLon) == "122:30W");
  REQUIRE(std::string(cis.szLat) == "47:30N");
  // Time and zone fields are non-empty
  REQUIRE(strlen(cis.szTim) > 0);
  REQUIRE(strlen(cis.szZon) > 0);
  REQUIRE(strlen(cis.szDst) > 0);
}

// ---- FormatRGB / KvParseRGB ----

TEST_CASE("FormatRGB extracts components", "[formatting]") {
  char r[8], g[8], b[8];
  KV kv = Rgb(255, 128, 0);
  FormatRGB(r, g, b, sizeof(r), kv);

  REQUIRE(std::string(r) == "255");
  REQUIRE(std::string(g) == "128");
  REQUIRE(std::string(b) == "0");
}

TEST_CASE("KvParseRGB creates color value", "[formatting]") {
  KV kv = KvParseRGB("255", "128", "0");
  REQUIRE(RgbR(kv) == 255);
  REQUIRE(RgbG(kv) == 128);
  REQUIRE(RgbB(kv) == 0);
}

TEST_CASE("KvParseRGB clamps out-of-range values", "[formatting]") {
  KV kv = KvParseRGB("300", "-5", "128");
  REQUIRE(RgbR(kv) == 255);
  REQUIRE(RgbG(kv) == 0);
  REQUIRE(RgbB(kv) == 128);
}

TEST_CASE("FormatRGB and KvParseRGB round-trip", "[formatting]") {
  KV original = Rgb(42, 170, 99);
  char r[8], g[8], b[8];
  FormatRGB(r, g, b, sizeof(r), original);

  KV parsed = KvParseRGB(r, g, b);
  REQUIRE(RgbR(parsed) == 42);
  REQUIRE(RgbG(parsed) == 170);
  REQUIRE(RgbB(parsed) == 99);
}
