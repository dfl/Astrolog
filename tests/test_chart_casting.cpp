/*
** End-to-end chart calculation tests.
** Verifies CastChart produces correct planet positions for known dates
** using the Swiss Ephemeris. This catches regressions in the entire
** calculation pipeline (time normalization, house computation, planet
** computation, modifications, finalization).
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"

#include <cstring>

// Save/restore all global state modified by CastChart.
struct ChartSaver {
  CI ciSav;
  IS isSav;
  CP cpSav;
  US usSav;
  ChartSaver() : ciSav(ciCore), isSav(is), cpSav(cp0), usSav(us) {}
  ~ChartSaver() { ciCore = ciSav; is = isSav; cp0 = cpSav; us = usSav; }
};

// Helper: set up a basic chart and cast it.
static void CastTestChart(int mon, int day, int yea, real tim,
                          real dst, real zon, real lon, real lat) {
  static char nam[] = "Test";
  static char loc[] = "Test";
  ciCore.mon = mon;
  ciCore.day = day;
  ciCore.yea = yea;
  ciCore.tim = tim;
  ciCore.dst = dst;
  ciCore.zon = zon;
  ciCore.lon = lon;
  ciCore.lat = lat;
  ciCore.nam = nam;
  ciCore.loc = loc;

  us.fEphemFiles = fTrue;
  us.fPlacalcPla = fFalse;
  us.nHouseSystem = hsPlacidus;
  us.fProgress = fFalse;

  CastChart(0);
}

// ---- Basic sanity: all planets have valid positions ----

TEST_CASE("CastChart produces valid planet positions", "[chart]") {
  ChartSaver sav;
  // Jan 1, 2000 12:00 UT at Greenwich
  CastTestChart(1, 1, 2000, 12.0, 0.0, 0.0, 0.0, 51.5);

  for (int i = oSun; i <= oPlu; i++) {
    REQUIRE(planet[i] >= 0.0);
    REQUIRE(planet[i] < 360.0);
  }
}

TEST_CASE("CastChart produces valid house cusps", "[chart]") {
  ChartSaver sav;
  CastTestChart(1, 1, 2000, 12.0, 0.0, 0.0, 0.0, 51.5);

  for (int i = 1; i <= 12; i++) {
    REQUIRE(chouse[i] >= 0.0);
    REQUIRE(chouse[i] < 360.0);
  }
  REQUIRE(is.Asc >= 0.0);
  REQUIRE(is.MC >= 0.0);
}

// ---- Known planet positions for J2000.0 epoch ----
// Jan 1, 2000 12:00 UT — Sun should be near 280° (Capricorn ~10°)

TEST_CASE("CastChart Sun position J2000", "[chart]") {
  ChartSaver sav;
  CastTestChart(1, 1, 2000, 12.0, 0.0, 0.0, 0.0, 51.5);

  // Sun should be around 280.5° (10 Capricorn)
  REQUIRE(planet[oSun] == Approx(280.5).margin(1.0));
}

TEST_CASE("CastChart Moon position J2000", "[chart]") {
  ChartSaver sav;
  CastTestChart(1, 1, 2000, 12.0, 0.0, 0.0, 0.0, 51.5);

  // Moon moves fast (~13°/day) so wider margin, but should be valid
  REQUIRE(planet[oMoo] >= 0.0);
  REQUIRE(planet[oMoo] < 360.0);
}

// ---- Known chart: Summer solstice 2025 ----
// Jun 20, 2025 — Sun should be near 89° (beginning of Cancer, ~0° Cancer = 90°)

TEST_CASE("CastChart Sun near solstice", "[chart]") {
  ChartSaver sav;
  // June 20, 2025 ~22:00 UT is near summer solstice
  CastTestChart(6, 20, 2025, 22.0, 0.0, 0.0, 0.0, 51.5);

  // Sun should be near 90° (0 Cancer)
  REQUIRE(planet[oSun] == Approx(89.5).margin(1.5));
}

// ---- Vernal equinox: Sun near 0° Aries ----

TEST_CASE("CastChart Sun near vernal equinox", "[chart]") {
  ChartSaver sav;
  // March 20, 2025 ~09:00 UT is near vernal equinox
  CastTestChart(3, 20, 2025, 9.0, 0.0, 0.0, 0.0, 51.5);

  // Sun should be near 0° Aries (359-1° range)
  real sun = planet[oSun];
  real dist = sun < 180.0 ? sun : 360.0 - sun;
  REQUIRE(dist < 1.5);
}

// ---- Outer planets are slow-moving: verify approximate positions ----

TEST_CASE("CastChart outer planet positions 2025", "[chart]") {
  ChartSaver sav;
  CastTestChart(1, 1, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);

  // Jupiter in Gemini/Taurus area ~70-80° in early 2025
  REQUIRE(planet[oJup] == Approx(75.0).margin(10.0));

  // Saturn in Pisces ~340-350° in early 2025
  REQUIRE(planet[oSat] == Approx(343.0).margin(10.0));

  // Uranus in Taurus ~50-55°
  REQUIRE(planet[oUra] == Approx(53.0).margin(10.0));

  // Neptune in Pisces ~355-360°
  real nep = planet[oNep];
  real nepDist = nep > 180.0 ? 360.0 - nep : nep;
  REQUIRE(nepDist < 15.0);
}

// ---- Chart at different location doesn't change planet positions ----

TEST_CASE("CastChart planet positions independent of location", "[chart]") {
  ChartSaver sav;

  // Cast at Greenwich
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);
  real sunGreenwich = planet[oSun];
  real moonGreenwich = planet[oMoo];

  // Cast at Seattle (same UT time, different location)
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 122.5, 47.5);
  real sunSeattle = planet[oSun];
  real moonSeattle = planet[oMoo];

  // Planet positions should be (nearly) identical for same UT
  // (tiny difference possible due to topocentric effects)
  REQUIRE(sunGreenwich == Approx(sunSeattle).margin(0.01));
  REQUIRE(moonGreenwich == Approx(moonSeattle).margin(0.1));
}

// ---- House cusps DO change with location ----

TEST_CASE("CastChart house cusps differ by location", "[chart]") {
  ChartSaver sav;

  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);
  real ascGreenwich = chouse[1];

  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 122.5, 47.5);
  real ascSeattle = chouse[1];

  // Ascendant should differ significantly for 122.5° longitude difference
  real diff = RAbs(ascGreenwich - ascSeattle);
  if (diff > 180.0) diff = 360.0 - diff;
  REQUIRE(diff > 10.0);
}

// ---- Time zone handling ----

TEST_CASE("CastChart timezone offset produces equivalent result", "[chart]") {
  ChartSaver sav;

  // 12:00 UT at Greenwich
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);
  real sunUT = planet[oSun];

  // 4:00 AM local PST (zon=8): TT = 4 + (8 - 0) = 12:00 UT
  CastTestChart(6, 15, 2025, 4.0, 0.0, 8.0, 0.0, 51.5);
  real sunPST = planet[oSun];

  REQUIRE(sunUT == Approx(sunPST).margin(0.01));
}

// ---- DST handling ----

TEST_CASE("CastChart DST offset produces equivalent result", "[chart]") {
  ChartSaver sav;

  // 12:00 UT at Greenwich, no DST
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);
  real sunNoDST = planet[oSun];

  // 13:00 local, DST=1, zone=0 -> 13:00 - 0 + 1 = 12:00 UT
  // Wait: TT = TT + (ZZ - SS), so 13:00 + (0 - 1) = 12:00 UT
  CastTestChart(6, 15, 2025, 13.0, 1.0, 0.0, 0.0, 51.5);
  real sunDST = planet[oSun];

  REQUIRE(sunNoDST == Approx(sunDST).margin(0.01));
}

// ---- Ascendant and MC are set correctly ----

TEST_CASE("CastChart Ascendant equals house 1", "[chart]") {
  ChartSaver sav;
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);

  REQUIRE(chouse[1] == Approx(is.Asc).margin(0.01));
}

TEST_CASE("CastChart MC equals house 10", "[chart]") {
  ChartSaver sav;
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);

  REQUIRE(chouse[10] == Approx(is.MC).margin(0.01));
}

// ---- Planet array objects match cusp objects ----

TEST_CASE("CastChart planet Asc/MC objects match is values", "[chart]") {
  ChartSaver sav;
  CastTestChart(6, 15, 2025, 12.0, 0.0, 0.0, 0.0, 51.5);

  // planet[oAsc] should equal is.Asc
  REQUIRE(planet[oAsc] == Approx(is.Asc).margin(0.01));
}
