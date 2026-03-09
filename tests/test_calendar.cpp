/*
** Unit tests for date calculation functions in general.cpp and calc.cpp:
** DayInMonth, DayOfWeek, AddDay, MdyToJulian/JulianToMdy.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"

// ---- DayInMonth ----

TEST_CASE("DayInMonth 31-day months", "[calendar]") {
  REQUIRE(DayInMonth(1, 2025) == 31);   // January
  REQUIRE(DayInMonth(3, 2025) == 31);   // March
  REQUIRE(DayInMonth(5, 2025) == 31);   // May
  REQUIRE(DayInMonth(7, 2025) == 31);   // July
  REQUIRE(DayInMonth(8, 2025) == 31);   // August
  REQUIRE(DayInMonth(10, 2025) == 31);  // October
  REQUIRE(DayInMonth(12, 2025) == 31);  // December
}

TEST_CASE("DayInMonth 30-day months", "[calendar]") {
  REQUIRE(DayInMonth(4, 2025) == 30);   // April
  REQUIRE(DayInMonth(6, 2025) == 30);   // June
  REQUIRE(DayInMonth(9, 2025) == 30);   // September
  REQUIRE(DayInMonth(11, 2025) == 30);  // November
}

TEST_CASE("DayInMonth February non-leap", "[calendar]") {
  REQUIRE(DayInMonth(2, 2023) == 28);
  REQUIRE(DayInMonth(2, 2025) == 28);
  REQUIRE(DayInMonth(2, 1900) == 28);  // Divisible by 100 but not 400
}

TEST_CASE("DayInMonth February leap years", "[calendar]") {
  REQUIRE(DayInMonth(2, 2024) == 29);  // Divisible by 4
  REQUIRE(DayInMonth(2, 2000) == 29);  // Divisible by 400
  REQUIRE(DayInMonth(2, 1996) == 29);
}

// ---- DayOfWeek ----

TEST_CASE("DayOfWeek known dates", "[calendar]") {
  // 0=Sunday, 1=Monday, ..., 6=Saturday
  // Jan 1, 2025 = Wednesday (3)
  REQUIRE(DayOfWeek(1, 1, 2025) == 3);

  // Jan 1, 2000 = Saturday (6)
  REQUIRE(DayOfWeek(1, 1, 2000) == 6);

  // July 4, 1776 = Thursday (4)
  REQUIRE(DayOfWeek(7, 4, 1776) == 4);

  // March 9, 2026 = Monday (1)
  REQUIRE(DayOfWeek(3, 9, 2026) == 1);
}

TEST_CASE("DayOfWeek sequential days", "[calendar]") {
  // Verify consecutive days increment properly
  int dow = DayOfWeek(1, 1, 2025);
  for (int d = 2; d <= 7; d++) {
    int next = DayOfWeek(1, d, 2025);
    REQUIRE(next == (dow + d - 1) % 7);
  }
}

// ---- MdyToJulian / JulianToMdy round-trip ----

TEST_CASE("MdyToJulian known values", "[calendar]") {
  // J2000.0 epoch: Jan 1.5, 2000 = JD 2451545.0
  // MdyToJulian computes at noon, so Jan 1, 2000 should give 2451545
  long jd = MdyToJulian(1, 1, 2000);
  REQUIRE(jd == 2451545L);
}

TEST_CASE("MdyToJulian/JulianToMdy round-trip", "[calendar]") {
  // Test several known dates
  struct { int mon, day, yea; } dates[] = {
    {1, 1, 2000},
    {6, 15, 2025},
    {12, 31, 1999},
    {3, 1, 1900},
    {7, 4, 1776},
  };

  for (auto &d : dates) {
    long jd = MdyToJulian(d.mon, d.day, d.yea);
    int mon, day, yea;
    JulianToMdy((real)jd, &mon, &day, &yea);
    REQUIRE(mon == d.mon);
    REQUIRE(day == d.day);
    REQUIRE(yea == d.yea);
  }
}

TEST_CASE("MdyToJulian monotonically increasing", "[calendar]") {
  long jd1 = MdyToJulian(1, 1, 2025);
  long jd2 = MdyToJulian(1, 2, 2025);
  long jd3 = MdyToJulian(2, 1, 2025);

  REQUIRE(jd2 == jd1 + 1);
  REQUIRE(jd3 > jd1);
}

// ---- AddDay ----

TEST_CASE("AddDay basic addition", "[calendar]") {
  // AddDay just adds delta to day, so day 15 + 1 = 16
  REQUIRE(AddDay(6, 15, 2025, 1) == 16);
  REQUIRE(AddDay(6, 15, 2025, -1) == 14);
  REQUIRE(AddDay(1, 1, 2025, 0) == 1);
}

TEST_CASE("AddDay across month boundary value", "[calendar]") {
  // Day 31 + 1 = 32 (caller handles month rollover)
  REQUIRE(AddDay(1, 31, 2025, 1) == 32);
  // Day 1 - 1 = 0 (caller handles month rollover)
  REQUIRE(AddDay(2, 1, 2025, -1) == 0);
}
