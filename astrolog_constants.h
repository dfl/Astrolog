/*
** Astrolog (Version 7.80) File: astrolog_constants.h
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
*/

/*
******************************************************************************
** Program Constants.
******************************************************************************
*/

#ifndef ASTROLOG_CONSTANTS_H
#define ASTROLOG_CONSTANTS_H

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif
#define fFalse FALSE
#define fTrue  TRUE

// Version and application info strings
#define szAppNameCore "Astrolog"
#define szVersionCore "7.80"
#define szVerCore     "780"
#define szDateCore    "June 2025"
#define szAddressCore \
  "Astara@msn.com - http://www.astrolog.org/astrolog.htm"
#define szNowCore     "now"
#define szTtyCore     "tty"
#define szSetCore     "set"
#define szNulCore     "nul"
#define szObjUnknown  "???"

// String size constants
#define cchSzDef  80
#define cchSzMax  255
#define cchSzLine (cchSzMax*4)
#define dwCanary  0x87654321

// Degree constants
#define nDegMax   360
#define nDegHalf  180

// Numeric constants
#define nMillion  1000000
#define nLarge    9999999
#define nNegative -1000

// Julian/Gregorian calendar constants
#define yeaJ2G    1582
#define monJ2G    mOct
#define dayJ2G1   4
#define dayJ2G2   15

// Timezone constants
#define zonLMT    24.0
#define zonLAT    23.0
#define dstAuto   24.0

// Star constants
#define rStarLite -1.46
#define rStarSpan 7.0
#define rStarNot  999.99

// Animation constants
#define iAnimDay  4
#define iAnimNow  10
#define iFillMax  255

// Math and conversion constants
#define rSqr2      1.41421356237309504880
#define rSqr3      1.73205080756887729353
#define rPhi       1.61803398874989484820
#define rLog10     2.30258509299404568402
#define rLog101    4.61512051684125945088
#define rPi        3.14159265358979323846
#define rPi2       (rPi*2.0)
#define rPiHalf    (rPi/2.0)
#define rDegMax    360.0
#define rDegHalf   180.0
#define rDegQuad   90.0
#define rDegRad    (rDegHalf/rPi)
#define rMiToKm    1.609344
#define rFtToM     0.3048
#define rInToCm    2.54
#define rAUToKm    149597870.7
#define rLYToAU    63241.07708427
#define rPCToAU    206264.8062471
#define rDayInYear 365.24219
#define rEarthDist 149.59787
#define rEpoch2000 -24.736467
#define rJD2000    2451545.0
#define rAxis      23.44578889
#define rSmall     (1.7453E-09)
#define rLarge     10000.0
#define rInvalid   (1.23456789E-09)
#define rRound     0.5

// Character constants
#define chNull   '\0'
#define chEscape '\33' // 27
#define chBell   '\7'
#define chReturn '\r'
#define chTab    '\t'
#define chDelete '\b'
#define chBreak  '\3'
#define chDegL   '\260' // 176 (Latin-1)
#define chDegI   '\370' // 248 (IBM)
#define chRet    'R'
#define chSep    ','
#define chSep2   ';'

// Array index limits
#define objMax    (cObj+1)
#define objMaxG   (objMax+11)
#define cCnstl    88
#define cZone     73
#define cSector   36
#define cPart     177
#define cWeek     7
#define cColor    16
#define cRainbow  7
#define cRay      7
#define cRing     6
#define cHasMoons 11
#define cNakshat  27

// Font size constants
#define xFont  6
#define yFont  10
#define xFont2 (xFont >> 1)
#define yFont2 (yFont >> 1)
#define xFontT (xFont2 * gi.nScaleTextT2)
#define yFontT (yFont2 * gi.nScaleTextT2)
#define xSideT (SIDESIZE * gi.nScaleTextT2 >> 1)

// Atlas values
#define cchSzAtl 54
#define cchSzZon 13
#define icnewMax 252
#define icnusMax 51
#define icncaMax 13
#define icnUS    234  // United States
#define icnCA    38   // Canada
#define icnFR    76   // France
#define iznMax   425
#define ilistMax 200

// Object array index values
#define cPlanet oVes
#define cThing  oLil
#define oMain   10
#define oCore   21
#define cUran   9
#define cDwarf  9
#define cMoons  27
#define cCOB    5
#define cMoons2 (cMoons + cCOB)
#define cCust   (cUran + cDwarf + cMoons2)
#define cStar   50
#define cuspLo  (oCore+1)
#define cuspHi  (cuspLo+cSign-1)
#define uranLo  (cuspHi+1)
#define uranHi  (uranLo+cUran-1)
#define dwarfLo (uranHi+1)
#define dwarfHi (dwarfLo+cDwarf-1)
#define moonsLo (dwarfHi+1)
#define moonsHi (moonsLo+cMoons-1)
#define cobLo   (moonsHi+1)
#define cobHi   (cobLo+cCOB-1)
#define starLo  (cobHi+1)
#define starHi  (starLo+cStar-1)
#define custLo  uranLo
#define custHi  cobHi
#define oNorm   cobHi
#define oNorm1  starLo

// Biorhythm cycle constants
#define brPhy 23.0
#define brEmo 28.0
#define brInt 33.0

#ifdef GRAPH
#define BITMAPX    4096  // Maximum window size allowed.
#define BITMAPY    4096
#define BITMAPX1    180  // Minimum window size allowed.
#define BITMAPY1    180
#define DEFAULTX    600  // Default window size.
#define DEFAULTY    600
#define SIDESIZE    160  // Size of wheel chart information sidebar.
#define MAXMETA 8000000  // Max bytes allowed in a metafile.
#define METAMUL      12  // Metafile coordinate to chart pixel ratio.
#define PSMUL        11  // PostScript coordinate to chart pixel ratio.
#define WIREMUL      10  // Wireframe coordinate to chart pixel ratio.
#define CELLSIZE     14  // Size for each cell in the aspect grid.
#define DEFORB      7.0  // Min distance glyphs can be from each other.
#define MAXSCALE    400  // Max scale factor as passed to -Xs swtich.
#endif // GRAPH

// Ascii and other characters used to display text charts.
#define chH    (char)(us.fAnsiChar ? 196 : '-')
#define chV    (char)(us.fAnsiChar ? 179 : '|')
#define chC    (char)(us.fAnsiChar ? 197 : '|')
#define chNW   (char)(us.fAnsiChar ? 218 : '+')
#define chNE   (char)(us.fAnsiChar ? 191 : '+')
#define chSW   (char)(us.fAnsiChar ? 192 : '+')
#define chSE   (char)(us.fAnsiChar ? 217 : '+')
#define chJN   (char)(us.fAnsiChar ? 193 : '-')
#define chJS   (char)(us.fAnsiChar ? 194 : '-')
#define chJW   (char)(us.fAnsiChar ? 180 : '|')
#define chJE   (char)(us.fAnsiChar ? 195 : '|')
#define chDegS (char)(us.fAnsiChar ? chDegT : ' ')
#define chDegC (char)(us.fAnsiChar ? chDegT : ':')
#define chDegT (char)(us.nCharset <= ccIBM ? chDegI : chDegL)

// Memory and path constants
#define CONST const
#define PAllocateCore(cb) malloc(cb)
#define DeallocatePCore(p) free(p)
#define DeallocatePIf(p) if ((p) != NULL) DeallocateP(p);
#define ldTime 2440588L
#ifndef PC
#define chDirSep '/'
#define chSwitch '-'
#else // PC
#define chDirSep '\\'
#define chSwitch '/'
#endif // PC

#ifdef PC
#ifdef _WIN64
#define szArchCore "64 bit"
#else
#define szArchCore "32 bit"
#endif
#define INLINE __forceinline
#else
#define INLINE inline
#endif // PC

#endif // ASTROLOG_CONSTANTS_H

/* astrolog_constants.h */
