/*
** Astrolog (Version 7.80) File: xdialog.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** Shared Dialog Helpers Implementation
*/

#include "astrolog.h"
#include "xdialog.h"
#include <stdlib.h>
#include <string.h>

/*
******************************************************************************
** Field Validation Functions
******************************************************************************
*/

VR ValidateMon(int mon)
{
  VR vr;
  vr.szField = "month";
  vr.fValid = FValidMon(mon);
  vr.szError = vr.fValid ? NULL : "Month must be between 1 and 12.";
  return vr;
}

VR ValidateYea(int yea)
{
  VR vr;
  vr.szField = "year";
  vr.fValid = FValidYea(yea);
  vr.szError = vr.fValid ? NULL : "Invalid year value.";
  return vr;
}

VR ValidateDay(int day, int mon, int yea)
{
  VR vr;
  vr.szField = "day";
  vr.fValid = FValidDay(day, mon, yea);
  vr.szError = vr.fValid ? NULL : "Invalid day for the given month and year.";
  return vr;
}

VR ValidateTim(real tim)
{
  VR vr;
  vr.szField = "time";
  vr.fValid = FValidTim(tim);
  vr.szError = vr.fValid ? NULL : "Time must be between 0:00 and 23:59.";
  return vr;
}

VR ValidateDst(real dst)
{
  VR vr;
  vr.szField = "daylight saving";
  vr.fValid = FValidDst(dst);
  vr.szError = vr.fValid ? NULL : "Invalid daylight saving value.";
  return vr;
}

VR ValidateZon(real zon)
{
  VR vr;
  vr.szField = "time zone";
  vr.fValid = FValidZon(zon);
  vr.szError = vr.fValid ? NULL : "Time zone must be between -24 and 24.";
  return vr;
}

VR ValidateLon(real lon)
{
  VR vr;
  vr.szField = "longitude";
  vr.fValid = FValidLon(lon);
  vr.szError = vr.fValid ? NULL : "Longitude must be between -180 and 180.";
  return vr;
}

VR ValidateLat(real lat)
{
  VR vr;
  vr.szField = "latitude";
  vr.fValid = FValidLat(lat);
  vr.szError = vr.fValid ? NULL : "Latitude must be between -90 and 90.";
  return vr;
}

// Validate all chart info fields at once
flag FValidateCI(CI *pci, const char **pszError)
{
  VR vr;

  vr = ValidateMon(pci->mon);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateYea(pci->yea);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateDay(pci->day, pci->mon, pci->yea);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateTim(pci->tim);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateDst(pci->dst);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateZon(pci->zon);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateLon(pci->lon);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  vr = ValidateLat(pci->lat);
  if (!vr.fValid) { *pszError = vr.szError; return fFalse; }

  *pszError = NULL;
  return fTrue;
}

/*
******************************************************************************
** Value Parsing Functions
******************************************************************************
*/

int NParseIntSafe(const char *sz, int nDefault)
{
  if (sz == NULL || *sz == '\0')
    return nDefault;
  return atoi(sz);
}

real RParseRealSafe(const char *sz, real rDefault)
{
  if (sz == NULL || *sz == '\0')
    return rDefault;
  return atof(sz);
}

void ParseDateFields(const char *szMon, const char *szDay, const char *szYea,
  int *pmon, int *pday, int *pyea)
{
  char sz[cchSzDef];

  if (szMon != NULL) {
    strncpy(sz, szMon, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *pmon = NParseSz(sz, pmMon);
  }
  if (szDay != NULL) {
    strncpy(sz, szDay, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *pday = NParseSz(sz, pmDay);
  }
  if (szYea != NULL) {
    strncpy(sz, szYea, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *pyea = NParseSz(sz, pmYea);
  }
}

void ParseTimeField(const char *szTim, real *ptim)
{
  char sz[cchSzDef];

  if (szTim != NULL) {
    strncpy(sz, szTim, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *ptim = RParseSz(sz, pmTim);
  }
}

void ParseZoneFields(const char *szDst, const char *szZon,
  real *pdst, real *pzon)
{
  char sz[cchSzDef];

  if (szDst != NULL) {
    strncpy(sz, szDst, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *pdst = RParseSz(sz, pmDst);
  }
  if (szZon != NULL) {
    strncpy(sz, szZon, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *pzon = RParseSz(sz, pmZon);
  }
}

void ParseCoordFields(const char *szLon, const char *szLat,
  real *plon, real *plat)
{
  char sz[cchSzDef];

  if (szLon != NULL) {
    strncpy(sz, szLon, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *plon = RParseSz(sz, pmLon);
  }
  if (szLat != NULL) {
    strncpy(sz, szLat, sizeof(sz)-1);
    sz[sizeof(sz)-1] = '\0';
    *plat = RParseSz(sz, pmLat);
  }
}

/*
******************************************************************************
** Value Formatting Functions
******************************************************************************
*/

void FormatMon(char *sz, int cchMax, int mon)
{
  if (FBetween(mon, 1, 12))
    snprintf(sz, cchMax, "%.3s", szMonth[mon]);
  else
    snprintf(sz, cchMax, "%d", mon);
}

void FormatDay(char *sz, int cchMax, int day)
{
  snprintf(sz, cchMax, "%d", day);
}

void FormatYea(char *sz, int cchMax, int yea)
{
  snprintf(sz, cchMax, "%d", yea);
}

void FormatTim(char *sz, int cchMax, real tim)
{
  snprintf(sz, cchMax, "%s", SzTim(tim));
}

void FormatDst(char *sz, int cchMax, real dst)
{
  snprintf(sz, cchMax, "%s", SzZone(dst));
}

void FormatZon(char *sz, int cchMax, real zon)
{
  snprintf(sz, cchMax, "%s", SzZone(-zon));
}

void FormatLon(char *sz, int cchMax, real lon)
{
  snprintf(sz, cchMax, "%s", SzLocation(lon, 0.0));
  // Trim after the longitude part
  char *pch = strchr(sz, ',');
  if (pch != NULL)
    *pch = '\0';
}

void FormatLat(char *sz, int cchMax, real lat)
{
  char szFull[cchSzDef];
  snprintf(szFull, sizeof(szFull), "%s", SzLocation(0.0, lat));
  // Extract just the latitude part after the comma
  char *pch = strchr(szFull, ',');
  if (pch != NULL)
    snprintf(sz, cchMax, "%s", pch + 2);  // Skip ", "
  else
    snprintf(sz, cchMax, "%.2f", lat);
}

void FormatCI(CI *pci, CIS *pcis)
{
  FormatMon(pcis->szMon, sizeof(pcis->szMon), pci->mon);
  FormatDay(pcis->szDay, sizeof(pcis->szDay), pci->day);
  FormatYea(pcis->szYea, sizeof(pcis->szYea), pci->yea);
  FormatTim(pcis->szTim, sizeof(pcis->szTim), pci->tim);
  FormatDst(pcis->szDst, sizeof(pcis->szDst), pci->dst);
  FormatZon(pcis->szZon, sizeof(pcis->szZon), pci->zon);
  FormatLon(pcis->szLon, sizeof(pcis->szLon), pci->lon);
  FormatLat(pcis->szLat, sizeof(pcis->szLat), pci->lat);
}

/*
******************************************************************************
** Color Helpers
******************************************************************************
*/

void FormatRGB(char *szR, char *szG, char *szB, int cchMax, KV kv)
{
  snprintf(szR, cchMax, "%d", RgbR(kv));
  snprintf(szG, cchMax, "%d", RgbG(kv));
  snprintf(szB, cchMax, "%d", RgbB(kv));
}

KV KvParseRGB(const char *szR, const char *szG, const char *szB)
{
  int r = NParseIntSafe(szR, 0);
  int g = NParseIntSafe(szG, 0);
  int b = NParseIntSafe(szB, 0);
  return Rgb(NClamp(r, 0, 255), NClamp(g, 0, 255), NClamp(b, 0, 255));
}

/*
******************************************************************************
** Numeric Range Helpers
******************************************************************************
*/

int NClamp(int n, int nMin, int nMax)
{
  if (n < nMin) return nMin;
  if (n > nMax) return nMax;
  return n;
}

real RClamp(real r, real rMin, real rMax)
{
  if (r < rMin) return rMin;
  if (r > rMax) return rMax;
  return r;
}

flag FInRange(int n, int nMin, int nMax)
{
  return n >= nMin && n <= nMax;
}

flag FInRangeR(real r, real rMin, real rMax)
{
  return r >= rMin && r <= rMax;
}
