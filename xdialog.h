/*
** Astrolog (Version 7.80) File: xdialog.h
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** Shared Dialog Helpers
**
** This module provides common validation and formatting functions used by
** both Windows (wdialog.cpp) and FLTK (fdialog.cpp) dialog implementations.
*/

#ifndef _XDIALOG_H
#define _XDIALOG_H

// Note: This header assumes astrolog.h has already been included.
// Do not include astrolog.h here as it lacks include guards.

/*
** Validation Result Structure
** Used to return validation status with descriptive error messages.
*/

typedef struct _ValidationResult {
  flag fValid;           // Whether the value is valid
  const char *szField;   // Field name for error messages
  const char *szError;   // Error message if invalid (NULL if valid)
} VR;

/*
** Field Validation Functions
** These wrap the FValid* functions and return structured results.
*/

VR ValidateMon(int mon);
VR ValidateYea(int yea);
VR ValidateDay(int day, int mon, int yea);
VR ValidateTim(real tim);
VR ValidateDst(real dst);
VR ValidateZon(real zon);
VR ValidateLon(real lon);
VR ValidateLat(real lat);

// Validate all chart info fields at once
// Returns fTrue if all valid, sets szError to first error found
flag FValidateCI(CI *pci, const char **pszError);

/*
** Value Parsing Functions
** Parse string values into typed values with error checking.
*/

int NParseIntSafe(const char *sz, int nDefault);
real RParseRealSafe(const char *sz, real rDefault);

// Parse date/time fields from strings
void ParseDateFields(const char *szMon, const char *szDay, const char *szYea,
  int *pmon, int *pday, int *pyea);
void ParseTimeField(const char *szTim, real *ptim);
void ParseZoneFields(const char *szDst, const char *szZon,
  real *pdst, real *pzon);
void ParseCoordFields(const char *szLon, const char *szLat,
  real *plon, real *plat);

/*
** Value Formatting Functions
** Format typed values into strings for display.
*/

// Format date components
void FormatMon(char *sz, int cchMax, int mon);
void FormatDay(char *sz, int cchMax, int day);
void FormatYea(char *sz, int cchMax, int yea);
void FormatTim(char *sz, int cchMax, real tim);

// Format zone/DST
void FormatDst(char *sz, int cchMax, real dst);
void FormatZon(char *sz, int cchMax, real zon);

// Format coordinates
void FormatLon(char *sz, int cchMax, real lon);
void FormatLat(char *sz, int cchMax, real lat);

// Format all chart info fields into a set of strings
typedef struct _CIStrings {
  char szMon[8];
  char szDay[8];
  char szYea[12];
  char szTim[16];
  char szDst[16];
  char szZon[16];
  char szLon[16];
  char szLat[16];
} CIS;

void FormatCI(CI *pci, CIS *pcis);

/*
** Color Helpers
*/

// Format RGB components
void FormatRGB(char *szR, char *szG, char *szB, int cchMax, KV kv);

// Parse RGB components
KV KvParseRGB(const char *szR, const char *szG, const char *szB);

/*
** Numeric Range Helpers
*/

// Clamp value to valid range
int NClamp(int n, int nMin, int nMax);
real RClamp(real r, real rMin, real rMax);

// Check if value is in range
flag FInRange(int n, int nMin, int nMax);
flag FInRangeR(real r, real rMin, real rMax);

#endif // _XDIALOG_H
