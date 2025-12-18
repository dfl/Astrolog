/*
** Astrolog (Version 7.80) File: xevent.h
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** Shared Event Handling Helpers
**
** This module provides common event handling functions used by
** X11 (xscreen.cpp), Windows (wdriver.cpp), and FLTK (fdriver.cpp).
*/

#ifndef _XEVENT_H
#define _XEVENT_H

// Note: This header assumes astrolog.h has already been included.

/*
** Map Coordinate Calculation
** Convert mouse position to map longitude/latitude coordinates.
*/

void CalculateMapCoords(int x, int y, real *pLon, real *pLat);

/*
** Rotation and Tilt Updates
** Update rotation and tilt values from mouse drag delta, with clamping.
** Parameters:
**   deltaX, deltaY - mouse movement since last position
**   nMode - current graphics mode (gGlobe, gLocal, etc.)
*/

void UpdateRotationTilt(int deltaX, int deltaY, int nMode);

/*
** Rotation/Tilt Value Clamping
** Ensure rotation and tilt values are within valid bounds.
*/

void ClampRotation(void);
void ClampTilt(void);

/*
** Zoom Control
** Adjust zoom level proportionally.
** Parameters:
**   nDirection - positive to zoom in, negative to zoom out
** Returns fTrue if zoom changed, fFalse otherwise.
*/

flag FAdjustZoom(int nDirection);

/*
** Mode Toggling
** Handle chart mode toggle keys.
** Returns the new mode if changed, or -1 if key not handled.
*/

int NHandleModeKey(int ch);

/*
** Animation Control
*/

void TogglePause(void);
void StepAnimation(int nDir);  // +1 forward, -1 backward

/*
** Scale Adjustment
** Adjust graphics scale by delta.
** Returns fTrue if scale changed.
*/

flag FAdjustScale(int nDelta);

/*
** Aspect Ratio Helpers
*/

// Calculate required window dimensions for map mode
void GetMapDimensions(int *pxWin, int *pyWin);

// Check if current mode is a map type
flag FIsMapMode(int nMode);

// Check if current mode supports rotation
flag FSupportsRotation(int nMode);

#endif // _XEVENT_H
