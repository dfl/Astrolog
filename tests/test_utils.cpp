/*
** Unit tests for utility functions in xdialog.cpp:
** NClamp, RClamp, FInRange, FInRangeR, NParseIntSafe, RParseRealSafe.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"
#include "xdialog.h"

// ---- NClamp ----

TEST_CASE("NClamp within range returns value", "[utils]") {
  REQUIRE(NClamp(5, 0, 10) == 5);
  REQUIRE(NClamp(0, 0, 10) == 0);
  REQUIRE(NClamp(10, 0, 10) == 10);
}

TEST_CASE("NClamp below minimum returns minimum", "[utils]") {
  REQUIRE(NClamp(-1, 0, 10) == 0);
  REQUIRE(NClamp(-100, 0, 255) == 0);
}

TEST_CASE("NClamp above maximum returns maximum", "[utils]") {
  REQUIRE(NClamp(11, 0, 10) == 10);
  REQUIRE(NClamp(300, 0, 255) == 255);
}

// ---- RClamp ----

TEST_CASE("RClamp within range returns value", "[utils]") {
  REQUIRE(RClamp(5.5, 0.0, 10.0) == 5.5);
  REQUIRE(RClamp(0.0, 0.0, 10.0) == 0.0);
  REQUIRE(RClamp(10.0, 0.0, 10.0) == 10.0);
}

TEST_CASE("RClamp clamps to boundaries", "[utils]") {
  REQUIRE(RClamp(-0.1, 0.0, 10.0) == 0.0);
  REQUIRE(RClamp(10.1, 0.0, 10.0) == 10.0);
}

// ---- FInRange ----

TEST_CASE("FInRange boundary and interior", "[utils]") {
  REQUIRE(FInRange(5, 1, 10));
  REQUIRE(FInRange(1, 1, 10));
  REQUIRE(FInRange(10, 1, 10));
  REQUIRE(!FInRange(0, 1, 10));
  REQUIRE(!FInRange(11, 1, 10));
}

// ---- FInRangeR ----

TEST_CASE("FInRangeR boundary and interior", "[utils]") {
  REQUIRE(FInRangeR(5.0, 1.0, 10.0));
  REQUIRE(FInRangeR(1.0, 1.0, 10.0));
  REQUIRE(FInRangeR(10.0, 1.0, 10.0));
  REQUIRE(!FInRangeR(0.9, 1.0, 10.0));
  REQUIRE(!FInRangeR(10.1, 1.0, 10.0));
}

// ---- NParseIntSafe ----

TEST_CASE("NParseIntSafe normal input", "[utils]") {
  REQUIRE(NParseIntSafe("42", 0) == 42);
  REQUIRE(NParseIntSafe("-7", 0) == -7);
  REQUIRE(NParseIntSafe("0", 99) == 0);
}

TEST_CASE("NParseIntSafe NULL and empty return default", "[utils]") {
  REQUIRE(NParseIntSafe(NULL, 99) == 99);
  REQUIRE(NParseIntSafe("", 99) == 99);
}

// ---- RParseRealSafe ----

TEST_CASE("RParseRealSafe normal input", "[utils]") {
  REQUIRE(RParseRealSafe("3.14", 0.0) == Approx(3.14));
  REQUIRE(RParseRealSafe("-1.5", 0.0) == Approx(-1.5));
  REQUIRE(RParseRealSafe("0", 99.0) == Approx(0.0));
}

TEST_CASE("RParseRealSafe NULL and empty return default", "[utils]") {
  REQUIRE(RParseRealSafe(NULL, 99.0) == Approx(99.0));
  REQUIRE(RParseRealSafe("", 99.0) == Approx(99.0));
}
