/*
** Astrolog (Version 7.80) File: xevent.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** Shared Event Handling Helpers Implementation
*/

#include "astrolog.h"

#ifdef GRAPH
#include "xevent.h"

/*
******************************************************************************
** Map Coordinate Calculation
******************************************************************************
*/

// Convert mouse position to map longitude/latitude coordinates.
// This is used when Alt+clicking on the map to relocate the chart.

void CalculateMapCoords(int x, int y, real *pLon, real *pLat)
{
  real lon, lat;

  // Calculate longitude from x position
  lon = rDegHalf - Mod((real)(x - gi.xOffset) / (real)gs.xWin * rDegMax - gs.rRot);

  // Clamp longitude to valid range
  if (lon < -rDegHalf)
    lon = -rDegHalf;
  else if (lon > rDegHalf)
    lon = rDegHalf;

  // Calculate latitude from y position
  lat = rDegQuad - (real)(y - gi.yOffset) / (real)gs.yWin * rDegHalf;

  // Clamp latitude to valid range
  if (lat < -rDegQuad)
    lat = -rDegQuad;
  else if (lat > rDegQuad)
    lat = rDegQuad;

  *pLon = lon;
  *pLat = lat;
}

/*
******************************************************************************
** Rotation and Tilt Updates
******************************************************************************
*/

// Ensure rotation value is in range [0, 360)
void ClampRotation(void)
{
  while (gs.rRot >= rDegMax)
    gs.rRot -= rDegMax;
  while (gs.rRot < 0.0)
    gs.rRot += rDegMax;
}

// Ensure tilt value is in range [-90, 90]
void ClampTilt(void)
{
  if (gs.rTilt > rDegQuad)
    gs.rTilt = rDegQuad;
  else if (gs.rTilt < -rDegQuad)
    gs.rTilt = -rDegQuad;
}

// Update rotation and tilt values from mouse drag delta
void UpdateRotationTilt(int deltaX, int deltaY, int nMode)
{
  real rFactor;

  // Calculate rotation factor based on mode
  if (nMode == gLocal) {
    rFactor = -gs.rTilt / rDegQuad;
    gs.rRot += (real)deltaX * rDegHalf / (real)gs.xWin * (-rFactor);
  } else {
    gs.rRot += (real)deltaX * rDegHalf / (real)gs.xWin;
  }

  // Calculate tilt change
  if (nMode == gLocal) {
    rFactor = gs.rTilt / rDegQuad;
    gs.rTilt += (real)deltaY * rDegHalf / (real)gs.yWin * rFactor;
  } else if (nMode == gGlobe) {
    gs.rTilt += (real)deltaY * rDegHalf / (real)gs.yWin * (-1.0);
  } else {
    gs.rTilt += (real)deltaY * rDegHalf / (real)gs.yWin;
  }

  // Clamp both values
  ClampRotation();
  ClampTilt();
}

/*
******************************************************************************
** Zoom Control
******************************************************************************
*/

// Adjust zoom level proportionally (±10%)
flag FAdjustZoom(int nDirection)
{
  int nOldScale = gs.nScale;

  if (nDirection > 0) {
    // Zoom in: multiply by 11/10 (10% increase)
    gs.nScale = gs.nScale * 11 / 10;
    if (gs.nScale > MAXSCALE)
      gs.nScale = MAXSCALE;
  } else if (nDirection < 0) {
    // Zoom out: multiply by 10/11 (~9% decrease)
    gs.nScale = gs.nScale * 10 / 11;
    if (gs.nScale < 100)
      gs.nScale = 100;
  }

  if (gs.nScale != nOldScale) {
    gi.nScale = gs.nScale / 100;
    return fTrue;
  }
  return fFalse;
}

// Adjust graphics scale by fixed delta
flag FAdjustScale(int nDelta)
{
  int nOldScale = gs.nScale;

  gs.nScale += nDelta;
  if (gs.nScale < 100)
    gs.nScale = 100;
  else if (gs.nScale > MAXSCALE)
    gs.nScale = MAXSCALE;

  if (gs.nScale != nOldScale) {
    gi.nScale = gs.nScale / 100;
    return fTrue;
  }
  return fFalse;
}

/*
******************************************************************************
** Mode Toggling
******************************************************************************
*/

// Handle chart mode toggle keys
// Returns the new mode if changed, or -1 if key not handled
int NHandleModeKey(int ch)
{
  int nMode = -1;

  switch (ch) {
  case 'V':
    // Toggle between wheel and house wheel
    nMode = (gi.nMode == gWheel) ? gHouse : gWheel;
    break;
  case 'v':
    // Toggle grid
    nMode = (gi.nMode == gGrid) ? gWheel : gGrid;
    break;
#ifdef WIN
  case 'A':
    // Toggle aspect grid (Windows only)
    nMode = (gi.nMode == gAspect) ? gWheel : gAspect;
    break;
#endif
  case 'M':
    // Toggle midpoint grid
    nMode = (gi.nMode == gMidpoint) ? gWheel : gMidpoint;
    break;
  case 'Z':
    // Toggle local horizon
    nMode = (gi.nMode == gHorizon) ? gWheel : gHorizon;
    break;
  case 'S':
    // Toggle orbit view
    nMode = (gi.nMode == gOrbit) ? gWheel : gOrbit;
    break;
  case 'J':
    // Toggle dispositor graph
    nMode = (gi.nMode == gDisposit) ? gWheel : gDisposit;
    break;
  case 'L':
    // Toggle astro-graph
    nMode = (gi.nMode == gAstroGraph) ? gWheel : gAstroGraph;
    break;
  case 'K':
    // Toggle calendar
    nMode = (gi.nMode == gCalendar) ? gWheel : gCalendar;
    break;
  case 'g':
    // Toggle globe
    nMode = (gi.nMode == gGlobe) ? gWheel : gGlobe;
    break;
  case 'a':
    // Toggle polar globe
    nMode = (gi.nMode == gPolar) ? gWheel : gPolar;
    break;
  // Note: 'F' key toggles constellation display via gs.fConstel flag,
  // not a separate graphics mode. Handle in platform-specific code.
  case 'N':
    // Toggle world map
    nMode = (gi.nMode == gWorldMap) ? gWheel : gWorldMap;
    break;
  case 'G':
    // Toggle graphic ephemeris
    nMode = (gi.nMode == gEphemeris) ? gWheel : gEphemeris;
    break;
#ifdef BIORHYTHM
  case 'Y':
    // Toggle biorhythm
    nMode = (gi.nMode == gBiorhythm) ? gWheel : gBiorhythm;
    break;
#endif
  }

  if (nMode >= 0)
    gi.nMode = nMode;

  return nMode;
}

/*
******************************************************************************
** Animation Control
******************************************************************************
*/

void TogglePause(void)
{
  gi.fPause = !gi.fPause;
}

void StepAnimation(int nDir)
{
  if (nDir > 0)
    Animate(gs.nAnim, NAbs(gi.nDir));
  else if (nDir < 0)
    Animate(gs.nAnim, -NAbs(gi.nDir));
}

/*
******************************************************************************
** Mode Helper Functions
******************************************************************************
*/

// Calculate required window dimensions for map mode
void GetMapDimensions(int *pxWin, int *pyWin)
{
  int length;

  length = nDegMax * gi.nScale;
  *pxWin = length;
  *pyWin = length / 2;
}

// Check if current mode is a map type
flag FIsMapMode(int nMode)
{
  return nMode == gAstroGraph || nMode == gWorldMap ||
         (nMode == gGlobe && gs.fMollewide);
}

// Check if current mode supports rotation (globe, local horizon, etc.)
flag FSupportsRotation(int nMode)
{
  return nMode == gGlobe || nMode == gPolar || nMode == gLocal ||
         nMode == gAstroGraph || nMode == gWorldMap ||
         nMode == gMidpoint || nMode == gTelescope;
}

#endif // GRAPH
