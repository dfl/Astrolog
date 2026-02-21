/*
** Unit tests for field validation functions in xdialog.cpp.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"
#include "xdialog.h"

// ---- ValidateMon ----

TEST_CASE("ValidateMon accepts 1-12", "[validation]") {
  for (int m = 1; m <= 12; m++) {
    VR vr = ValidateMon(m);
    REQUIRE(vr.fValid);
    REQUIRE(vr.szError == NULL);
  }
}

TEST_CASE("ValidateMon rejects out of range", "[validation]") {
  VR vr = ValidateMon(0);
  REQUIRE(!vr.fValid);
  REQUIRE(vr.szError != NULL);

  vr = ValidateMon(13);
  REQUIRE(!vr.fValid);

  vr = ValidateMon(-1);
  REQUIRE(!vr.fValid);
}

// ---- ValidateYea ----

TEST_CASE("ValidateYea accepts typical years", "[validation]") {
  REQUIRE(ValidateYea(2025).fValid);
  REQUIRE(ValidateYea(1).fValid);
  REQUIRE(ValidateYea(-500).fValid);
  REQUIRE(ValidateYea(0).fValid);
}

// ---- ValidateDay ----

TEST_CASE("ValidateDay normal months", "[validation]") {
  // 31-day months
  REQUIRE(ValidateDay(31, 1, 2025).fValid);   // Jan
  REQUIRE(ValidateDay(31, 3, 2025).fValid);   // Mar
  REQUIRE(!ValidateDay(32, 1, 2025).fValid);

  // 30-day months
  REQUIRE(ValidateDay(30, 4, 2025).fValid);   // Apr
  REQUIRE(!ValidateDay(31, 4, 2025).fValid);

  // Day 0 is invalid
  REQUIRE(!ValidateDay(0, 6, 2025).fValid);
}

TEST_CASE("ValidateDay February leap year", "[validation]") {
  // 2024 is a leap year
  REQUIRE(ValidateDay(29, 2, 2024).fValid);
  REQUIRE(!ValidateDay(30, 2, 2024).fValid);

  // 2025 is not a leap year
  REQUIRE(ValidateDay(28, 2, 2025).fValid);
  REQUIRE(!ValidateDay(29, 2, 2025).fValid);

  // 2000 is a leap year (divisible by 400)
  REQUIRE(ValidateDay(29, 2, 2000).fValid);

  // 1900 is not a leap year (divisible by 100 but not 400)
  REQUIRE(!ValidateDay(29, 2, 1900).fValid);
}

// ---- ValidateTim ----

TEST_CASE("ValidateTim valid times", "[validation]") {
  REQUIRE(ValidateTim(0.0).fValid);
  REQUIRE(ValidateTim(12.0).fValid);
  REQUIRE(ValidateTim(23.5).fValid);
  // Slightly negative is valid (> -2.0)
  REQUIRE(ValidateTim(-1.0).fValid);
}

TEST_CASE("ValidateTim invalid times", "[validation]") {
  REQUIRE(!ValidateTim(24.0).fValid);
  REQUIRE(!ValidateTim(-2.0).fValid);
  REQUIRE(!ValidateTim(100.0).fValid);
}

// ---- ValidateZon / ValidateDst ----

TEST_CASE("ValidateZon boundaries", "[validation]") {
  REQUIRE(ValidateZon(0.0).fValid);
  REQUIRE(ValidateZon(-24.0).fValid);
  REQUIRE(ValidateZon(24.0).fValid);
  REQUIRE(!ValidateZon(-24.1).fValid);
  REQUIRE(!ValidateZon(24.1).fValid);
}

TEST_CASE("ValidateDst matches ValidateZon range", "[validation]") {
  REQUIRE(ValidateDst(0.0).fValid);
  REQUIRE(ValidateDst(1.0).fValid);
  REQUIRE(ValidateDst(-24.0).fValid);
  REQUIRE(!ValidateDst(24.1).fValid);
}

// ---- ValidateLon ----

TEST_CASE("ValidateLon boundaries", "[validation]") {
  REQUIRE(ValidateLon(0.0).fValid);
  REQUIRE(ValidateLon(180.0).fValid);
  REQUIRE(ValidateLon(-180.0).fValid);
  REQUIRE(ValidateLon(360.0).fValid);
  REQUIRE(ValidateLon(-360.0).fValid);
  REQUIRE(!ValidateLon(360.1).fValid);
  REQUIRE(!ValidateLon(-360.1).fValid);
}

// ---- ValidateLat ----

TEST_CASE("ValidateLat boundaries", "[validation]") {
  REQUIRE(ValidateLat(0.0).fValid);
  REQUIRE(ValidateLat(90.0).fValid);
  REQUIRE(ValidateLat(-90.0).fValid);
  REQUIRE(!ValidateLat(90.1).fValid);
  REQUIRE(!ValidateLat(-90.1).fValid);
}

// ---- FValidateCI composite ----

TEST_CASE("FValidateCI accepts valid chart info", "[validation]") {
  char nam[] = "Test";
  char loc[] = "Seattle";
  CI ci = { 6, 15, 2025, 12.0, 1.0, 8.0, 122.5, 47.5, nam, loc };
  const char *szError = NULL;

  REQUIRE(FValidateCI(&ci, &szError));
  REQUIRE(szError == NULL);
}

TEST_CASE("FValidateCI rejects bad month", "[validation]") {
  char nam[] = "";
  char loc[] = "";
  CI ci = { 13, 15, 2025, 12.0, 0.0, 8.0, 122.5, 47.5, nam, loc };
  const char *szError = NULL;

  REQUIRE(!FValidateCI(&ci, &szError));
  REQUIRE(szError != NULL);
}

TEST_CASE("FValidateCI rejects bad day", "[validation]") {
  char nam[] = "";
  char loc[] = "";
  CI ci = { 2, 30, 2025, 12.0, 0.0, 8.0, 122.5, 47.5, nam, loc };
  const char *szError = NULL;

  REQUIRE(!FValidateCI(&ci, &szError));
  REQUIRE(szError != NULL);
}

TEST_CASE("FValidateCI rejects bad latitude", "[validation]") {
  char nam[] = "";
  char loc[] = "";
  CI ci = { 6, 15, 2025, 12.0, 0.0, 8.0, 122.5, 91.0, nam, loc };
  const char *szError = NULL;

  REQUIRE(!FValidateCI(&ci, &szError));
  REQUIRE(szError != NULL);
}
