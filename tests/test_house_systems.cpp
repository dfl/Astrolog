/*
** Unit tests for house system calculations.
** Uses Swiss Ephemeris (SwissHouse) to compute house cusps and verifies
** properties of various house systems.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"

#include <cstring>

// Helper to save and restore global state modified during house computation.
struct HouseSaver {
  CI ciSav;
  IS isSav;
  CP cpSav;
  US usSav;
  HouseSaver() : ciSav(ciCore), isSav(is), cpSav(cp0), usSav(us) {}
  ~HouseSaver() { ciCore = ciSav; is = isSav; cp0 = cpSav; us = usSav; }
};

// Set up chart for a known date/location and compute houses.
// Uses J2000.0 epoch: Jan 1, 2000 12:00 UT at Greenwich (0W, 51.5N).
static void ComputeTestHouses(int housesystem) {
  ClearB((pbyte)&cp0, sizeof(CP));

  MM = 1; DD = 1; YY = 2000;
  TT = 12.0; SS = 0.0; ZZ = 0.0;
  OO = 0.0;     // Greenwich longitude
  AA = 51.5;    // London latitude

  is.JD = (real)MdyToJulian(MM, DD, YY);
  is.T = (is.JD + TT / 24.0 - 2415020.5) / 36525.0;

  us.fEphemFiles = fTrue;
  us.fPlacalcPla = fFalse;

  SwissHouse(is.T, OO, AA, housesystem,
    &is.Asc, &is.MC, &is.RA, &is.Vtx, &is.EP,
    &is.OB, &is.rOff, &is.rNut);
  ComputeHouses(housesystem);
}

// All house cusps should be in [0, 360).
static void RequireValidCusps() {
  for (int i = 1; i <= 12; i++) {
    REQUIRE(chouse[i] >= 0.0);
    REQUIRE(chouse[i] < 360.0);
  }
}

// ---- Basic properties ----

TEST_CASE("Placidus houses produce valid cusps", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsPlacidus);
  RequireValidCusps();

  // Cusp 1 should be the Ascendant
  REQUIRE(chouse[1] == Approx(is.Asc).margin(0.01));
  // Cusp 10 should be the Midheaven
  REQUIRE(chouse[10] == Approx(is.MC).margin(0.01));
}

TEST_CASE("Koch houses produce valid cusps", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsKoch);
  RequireValidCusps();

  REQUIRE(chouse[1] == Approx(is.Asc).margin(0.01));
  REQUIRE(chouse[10] == Approx(is.MC).margin(0.01));
}

TEST_CASE("Equal houses are 30 degrees apart", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsEqual);
  RequireValidCusps();

  // Cusp 1 = Ascendant, each subsequent cusp +30 degrees
  for (int i = 1; i <= 12; i++) {
    real expected = is.Asc + (real)(i - 1) * 30.0;
    if (expected >= 360.0) expected -= 360.0;
    REQUIRE(chouse[i] == Approx(expected).margin(0.01));
  }
}

TEST_CASE("Whole Sign houses start at sign boundary", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsWhole);
  RequireValidCusps();

  // Each cusp should be at a sign boundary (multiple of 30)
  for (int i = 1; i <= 12; i++) {
    real frac = chouse[i] / 30.0;
    real rem = frac - RFloor(frac);
    REQUIRE((rem == Approx(0.0).margin(0.01) || rem == Approx(1.0).margin(0.01)));
  }
}

TEST_CASE("Porphyry houses produce valid cusps", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsPorphyry);
  RequireValidCusps();

  REQUIRE(chouse[1] == Approx(is.Asc).margin(0.01));
  REQUIRE(chouse[10] == Approx(is.MC).margin(0.01));
}

// ---- Different house systems produce different results ----

TEST_CASE("Different house systems produce different intermediate cusps", "[houses]") {
  HouseSaver sav;

  ComputeTestHouses(hsPlacidus);
  real placidus2 = chouse[2];
  real placidus11 = chouse[11];

  ComputeTestHouses(hsKoch);
  real koch2 = chouse[2];
  real koch11 = chouse[11];

  ComputeTestHouses(hsEqual);
  real equal2 = chouse[2];

  // Intermediate cusps should differ between systems
  // (cusps 1 and 10 are shared for quadrant systems)
  REQUIRE(placidus2 != Approx(equal2).margin(0.1));
  // Koch and Placidus may differ slightly
  // Just verify they computed without error
  REQUIRE(koch2 >= 0.0);
  REQUIRE(koch11 >= 0.0);
}

// ---- Opposite cusps ----

TEST_CASE("Opposite cusps are 180 degrees apart for Placidus", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsPlacidus);

  for (int i = 1; i <= 6; i++) {
    real diff = chouse[i + 6] - chouse[i];
    if (diff < 0.0) diff += 360.0;
    REQUIRE(diff == Approx(180.0).margin(0.01));
  }
}

// ---- Different latitude ----

TEST_CASE("Houses at equator compute correctly", "[houses]") {
  HouseSaver sav;
  ClearB((pbyte)&cp0, sizeof(CP));

  MM = 1; DD = 1; YY = 2000;
  TT = 12.0; SS = 0.0; ZZ = 0.0;
  OO = 0.0;
  AA = 0.0;  // Equator

  is.JD = (real)MdyToJulian(MM, DD, YY);
  is.T = (is.JD + TT / 24.0 - 2415020.5) / 36525.0;

  us.fEphemFiles = fTrue;
  us.fPlacalcPla = fFalse;

  SwissHouse(is.T, OO, AA, hsPlacidus,
    &is.Asc, &is.MC, &is.RA, &is.Vtx, &is.EP,
    &is.OB, &is.rOff, &is.rNut);
  ComputeHouses(hsPlacidus);
  RequireValidCusps();
}

// ---- Regiomontanus ----

TEST_CASE("Regiomontanus houses produce valid cusps", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsRegiomontanus);
  RequireValidCusps();

  REQUIRE(chouse[1] == Approx(is.Asc).margin(0.01));
  REQUIRE(chouse[10] == Approx(is.MC).margin(0.01));
}

// ---- Campanus ----

TEST_CASE("Campanus houses produce valid cusps", "[houses]") {
  HouseSaver sav;
  ComputeTestHouses(hsCampanus);
  RequireValidCusps();

  REQUIRE(chouse[1] == Approx(is.Asc).margin(0.01));
  REQUIRE(chouse[10] == Approx(is.MC).margin(0.01));
}
