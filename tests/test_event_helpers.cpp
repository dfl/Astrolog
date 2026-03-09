/*
** Unit tests for shared event handling helpers in xevent.cpp:
** ClampRotation, ClampTilt, FAdjustZoom, FAdjustScale,
** NHandleModeKey, FIsMapMode, FSupportsRotation, GetMapDimensions,
** CalculateMapCoords, UpdateRotationTilt.
**
** These functions are used by fdriver.cpp (FLTK), wdriver.cpp (Win32),
** and xscreen.cpp (X11) for mouse/keyboard interaction with charts.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "astrolog.h"
#include "xevent.h"

// Save/restore graphics state modified by event helpers.
struct GsSaver {
  GS gsSav;
  GI giSav;
  GsSaver() : gsSav(gs), giSav(gi) {}
  ~GsSaver() { gs = gsSav; gi = giSav; }
};

// ===========================================================================
// ClampRotation
// ===========================================================================

TEST_CASE("ClampRotation wraps positive overflow", "[event]") {
  GsSaver sav;
  gs.rRot = 370.0;
  ClampRotation();
  REQUIRE(gs.rRot == Approx(10.0).margin(0.001));
}

TEST_CASE("ClampRotation wraps negative", "[event]") {
  GsSaver sav;
  gs.rRot = -30.0;
  ClampRotation();
  REQUIRE(gs.rRot == Approx(330.0).margin(0.001));
}

TEST_CASE("ClampRotation wraps large positive", "[event]") {
  GsSaver sav;
  gs.rRot = 720.0 + 45.0;
  ClampRotation();
  REQUIRE(gs.rRot == Approx(45.0).margin(0.001));
}

TEST_CASE("ClampRotation leaves valid value", "[event]") {
  GsSaver sav;
  gs.rRot = 180.0;
  ClampRotation();
  REQUIRE(gs.rRot == Approx(180.0).margin(0.001));
}

TEST_CASE("ClampRotation zero stays zero", "[event]") {
  GsSaver sav;
  gs.rRot = 0.0;
  ClampRotation();
  REQUIRE(gs.rRot == Approx(0.0).margin(0.001));
}

// ===========================================================================
// ClampTilt
// ===========================================================================

TEST_CASE("ClampTilt clamps above 90", "[event]") {
  GsSaver sav;
  gs.rTilt = 100.0;
  ClampTilt();
  REQUIRE(gs.rTilt == Approx(90.0));
}

TEST_CASE("ClampTilt clamps below -90", "[event]") {
  GsSaver sav;
  gs.rTilt = -100.0;
  ClampTilt();
  REQUIRE(gs.rTilt == Approx(-90.0));
}

TEST_CASE("ClampTilt leaves valid value", "[event]") {
  GsSaver sav;
  gs.rTilt = 45.0;
  ClampTilt();
  REQUIRE(gs.rTilt == Approx(45.0));
}

TEST_CASE("ClampTilt boundary values", "[event]") {
  GsSaver sav;

  gs.rTilt = 90.0;
  ClampTilt();
  REQUIRE(gs.rTilt == Approx(90.0));

  gs.rTilt = -90.0;
  ClampTilt();
  REQUIRE(gs.rTilt == Approx(-90.0));
}

// ===========================================================================
// FAdjustZoom
// ===========================================================================

TEST_CASE("FAdjustZoom in increases scale by ~10%", "[event]") {
  GsSaver sav;
  gs.nScale = 200;
  flag f = FAdjustZoom(1);
  REQUIRE(f == fTrue);
  REQUIRE(gs.nScale == 220);  // 200 * 11 / 10
}

TEST_CASE("FAdjustZoom out decreases scale", "[event]") {
  GsSaver sav;
  gs.nScale = 200;
  flag f = FAdjustZoom(-1);
  REQUIRE(f == fTrue);
  REQUIRE(gs.nScale == 181);  // 200 * 10 / 11 = 181
}

TEST_CASE("FAdjustZoom zero does nothing", "[event]") {
  GsSaver sav;
  gs.nScale = 200;
  flag f = FAdjustZoom(0);
  REQUIRE(f == fFalse);
  REQUIRE(gs.nScale == 200);
}

TEST_CASE("FAdjustZoom clamps to minimum", "[event]") {
  GsSaver sav;
  gs.nScale = 100;
  flag f = FAdjustZoom(-1);
  // 100 * 10 / 11 = 90, clamped to 100 -> no change
  REQUIRE(f == fFalse);
  REQUIRE(gs.nScale == 100);
}

TEST_CASE("FAdjustZoom clamps to maximum", "[event]") {
  GsSaver sav;
  gs.nScale = MAXSCALE;
  flag f = FAdjustZoom(1);
  // Already at max, scale would go above but clamp back
  REQUIRE(gs.nScale == MAXSCALE);
}

// ===========================================================================
// FAdjustScale
// ===========================================================================

TEST_CASE("FAdjustScale positive delta", "[event]") {
  GsSaver sav;
  gs.nScale = 200;
  flag f = FAdjustScale(50);
  REQUIRE(f == fTrue);
  REQUIRE(gs.nScale == 250);
}

TEST_CASE("FAdjustScale negative delta", "[event]") {
  GsSaver sav;
  gs.nScale = 200;
  flag f = FAdjustScale(-50);
  REQUIRE(f == fTrue);
  REQUIRE(gs.nScale == 150);
}

TEST_CASE("FAdjustScale clamps to minimum 100", "[event]") {
  GsSaver sav;
  gs.nScale = 120;
  flag f = FAdjustScale(-50);
  REQUIRE(f == fTrue);
  REQUIRE(gs.nScale == 100);
}

TEST_CASE("FAdjustScale clamps to MAXSCALE", "[event]") {
  GsSaver sav;
  gs.nScale = MAXSCALE - 10;
  flag f = FAdjustScale(50);
  REQUIRE(f == fTrue);
  REQUIRE(gs.nScale == MAXSCALE);
}

TEST_CASE("FAdjustScale no change returns false", "[event]") {
  GsSaver sav;
  gs.nScale = 100;
  flag f = FAdjustScale(-50);
  // 100 - 50 = 50, clamped to 100 -> no change
  REQUIRE(f == fFalse);
}

// ===========================================================================
// NHandleModeKey
// ===========================================================================

TEST_CASE("NHandleModeKey V toggles wheel/house", "[event]") {
  GsSaver sav;

  gi.nMode = gWheel;
  int result = NHandleModeKey('V');
  REQUIRE(result == gHouse);
  REQUIRE(gi.nMode == gHouse);

  // Toggle back
  result = NHandleModeKey('V');
  REQUIRE(result == gWheel);
  REQUIRE(gi.nMode == gWheel);
}

TEST_CASE("NHandleModeKey v toggles grid", "[event]") {
  GsSaver sav;

  gi.nMode = gWheel;
  int result = NHandleModeKey('v');
  REQUIRE(result == gGrid);
  REQUIRE(gi.nMode == gGrid);

  // Toggle back
  result = NHandleModeKey('v');
  REQUIRE(result == gWheel);
  REQUIRE(gi.nMode == gWheel);
}

TEST_CASE("NHandleModeKey g toggles globe", "[event]") {
  GsSaver sav;

  gi.nMode = gWheel;
  int result = NHandleModeKey('g');
  REQUIRE(result == gGlobe);

  result = NHandleModeKey('g');
  REQUIRE(result == gWheel);
}

TEST_CASE("NHandleModeKey unknown key returns -1", "[event]") {
  GsSaver sav;
  gi.nMode = gWheel;
  int result = NHandleModeKey('x');
  REQUIRE(result == -1);
  REQUIRE(gi.nMode == gWheel);  // Unchanged
}

TEST_CASE("NHandleModeKey from non-wheel activates mode", "[event]") {
  GsSaver sav;
  // If already in globe mode, pressing 'Z' should switch to horizon
  gi.nMode = gGlobe;
  int result = NHandleModeKey('Z');
  REQUIRE(result == gHorizon);
  REQUIRE(gi.nMode == gHorizon);
}

// ===========================================================================
// FIsMapMode
// ===========================================================================

TEST_CASE("FIsMapMode recognizes map modes", "[event]") {
  REQUIRE(FIsMapMode(gAstroGraph) == fTrue);
  REQUIRE(FIsMapMode(gWorldMap) == fTrue);
}

TEST_CASE("FIsMapMode rejects non-map modes", "[event]") {
  REQUIRE(FIsMapMode(gWheel) == fFalse);
  REQUIRE(FIsMapMode(gGrid) == fFalse);
  REQUIRE(FIsMapMode(gHorizon) == fFalse);
}

TEST_CASE("FIsMapMode globe with Mollewide", "[event]") {
  GsSaver sav;

  gs.fMollewide = fTrue;
  REQUIRE(FIsMapMode(gGlobe) == fTrue);

  gs.fMollewide = fFalse;
  REQUIRE(FIsMapMode(gGlobe) == fFalse);
}

// ===========================================================================
// FSupportsRotation
// ===========================================================================

TEST_CASE("FSupportsRotation for globe modes", "[event]") {
  REQUIRE(FSupportsRotation(gGlobe) == fTrue);
  REQUIRE(FSupportsRotation(gSphere) == fTrue);
  REQUIRE(FSupportsRotation(gPolar) == fTrue);
  REQUIRE(FSupportsRotation(gLocal) == fTrue);
  REQUIRE(FSupportsRotation(gTelescope) == fTrue);
}

TEST_CASE("FSupportsRotation for map modes", "[event]") {
  REQUIRE(FSupportsRotation(gAstroGraph) == fTrue);
  REQUIRE(FSupportsRotation(gWorldMap) == fTrue);
  REQUIRE(FSupportsRotation(gMidpoint) == fTrue);
}

TEST_CASE("FSupportsRotation false for static modes", "[event]") {
  REQUIRE(FSupportsRotation(gWheel) == fFalse);
  REQUIRE(FSupportsRotation(gGrid) == fFalse);
  REQUIRE(FSupportsRotation(gHouse) == fFalse);
}

// ===========================================================================
// GetMapDimensions
// ===========================================================================

TEST_CASE("GetMapDimensions at scale 1", "[event]") {
  GsSaver sav;
  gi.nScale = 1;
  int xWin, yWin;
  GetMapDimensions(&xWin, &yWin);
  REQUIRE(xWin == 360);  // nDegMax * 1
  REQUIRE(yWin == 180);  // 360 / 2
}

TEST_CASE("GetMapDimensions at scale 2", "[event]") {
  GsSaver sav;
  gi.nScale = 2;
  int xWin, yWin;
  GetMapDimensions(&xWin, &yWin);
  REQUIRE(xWin == 720);
  REQUIRE(yWin == 360);
}

// ===========================================================================
// CalculateMapCoords
// ===========================================================================

TEST_CASE("CalculateMapCoords center of map", "[event]") {
  GsSaver sav;
  gs.xWin = 360;
  gs.yWin = 180;
  gs.rRot = 0.0;
  gi.xOffset = 0;
  gi.yOffset = 0;

  real lon, lat;
  // Center of map: x=180, y=90
  CalculateMapCoords(180, 90, &lon, &lat);
  REQUIRE(lat == Approx(0.0).margin(1.0));
}

TEST_CASE("CalculateMapCoords clamps latitude", "[event]") {
  GsSaver sav;
  gs.xWin = 360;
  gs.yWin = 180;
  gs.rRot = 0.0;
  gi.xOffset = 0;
  gi.yOffset = 0;

  real lon, lat;
  // y far below map -> should clamp to -90
  CalculateMapCoords(180, 500, &lon, &lat);
  REQUIRE(lat == Approx(-90.0));

  // y far above map -> should clamp to +90
  CalculateMapCoords(180, -500, &lon, &lat);
  REQUIRE(lat == Approx(90.0));
}

// ===========================================================================
// UpdateRotationTilt
// ===========================================================================

TEST_CASE("UpdateRotationTilt standard mode", "[event]") {
  GsSaver sav;
  gs.rRot = 180.0;
  gs.rTilt = 0.0;
  gs.xWin = 360;
  gs.yWin = 180;

  // Drag right by 36 pixels on a 360-wide window -> +18° rotation
  UpdateRotationTilt(36, 0, gGlobe);
  REQUIRE(gs.rRot == Approx(198.0).margin(0.1));
  REQUIRE(gs.rTilt == Approx(0.0).margin(0.1));
}

TEST_CASE("UpdateRotationTilt globe tilt inverted", "[event]") {
  GsSaver sav;
  gs.rRot = 180.0;
  gs.rTilt = 0.0;
  gs.xWin = 360;
  gs.yWin = 180;

  // Drag down by 18 pixels -> tilt decreases (globe inverted Y)
  UpdateRotationTilt(0, 18, gGlobe);
  REQUIRE(gs.rTilt == Approx(-18.0).margin(0.1));
}

TEST_CASE("UpdateRotationTilt clamps results", "[event]") {
  GsSaver sav;
  gs.rRot = 350.0;
  gs.rTilt = 85.0;
  gs.xWin = 360;
  gs.yWin = 180;

  // Large drag that would exceed bounds
  UpdateRotationTilt(100, -100, gSphere);
  REQUIRE(gs.rRot >= 0.0);
  REQUIRE(gs.rRot < 360.0);
  REQUIRE(gs.rTilt >= -90.0);
  REQUIRE(gs.rTilt <= 90.0);
}
