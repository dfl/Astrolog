/*
** Astrolog (Version 7.80) File: astrolog_enums.h
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
** Enumerations.
******************************************************************************
*/

#ifndef ASTROLOG_ENUMS_H
#define ASTROLOG_ENUMS_H

// Month index values
enum _months {
  mJan = 1,
  mFeb = 2,
  mMar = 3,
  mApr = 4,
  mMay = 5,
  mJun = 6,
  mJul = 7,
  mAug = 8,
  mSep = 9,
  mOct = 10,
  mNov = 11,
  mDec = 12,
};

// Elements
enum _elements {
  eFir = 0,
  eEar = 1,
  eAir = 2,
  eWat = 3,
  cElem = 4,
};

// Zodiac signs
enum _signs {
  sAri = 1,
  sTau = 2,
  sGem = 3,
  sCan = 4,
  sLeo = 5,
  sVir = 6,
  sLib = 7,
  sSco = 8,
  sSag = 9,
  sCap = 10,
  sAqu = 11,
  sPis = 12,
  cSign = 12,
};

// Objects
enum _objects {
  oEar = 0,
  oSun = 1,
  oMoo = 2,
  oMer = 3,
  oVen = 4,
  oMar = 5,
  oJup = 6,
  oSat = 7,
  oUra = 8,
  oNep = 9,
  oPlu = 10,
  oChi = 11,
  oCer = 12,
  oPal = 13,
  oJun = 14,
  oVes = 15,
  oNod = 16,
  oSou = 17,
  oLil = 18,
  oFor = 19,
  oVtx = 20,
  oEP  = 21,
  oAsc = (cuspLo-1 + 1),
  o2nd = (cuspLo-1 + 2),
  o3rd = (cuspLo-1 + 3),
  oNad = (cuspLo-1 + 4),
  o5th = (cuspLo-1 + 5),
  o6th = (cuspLo-1 + 6),
  oDes = (cuspLo-1 + 7),
  o8th = (cuspLo-1 + 8),
  o9th = (cuspLo-1 + 9),
  oMC  = (cuspLo-1 + 10),
  o11h = (cuspLo-1 + 11),
  o12h = (cuspLo-1 + 12),
  oVul = (uranLo + 0),    // Vulcan
  oVlk = (uranLo + 7),    // Vulkanus
  oHyg = (dwarfLo + 0),   // Hygiea
  oPho = (dwarfLo + 1),   // Pholus
  oEri = (dwarfLo + 2),   // Eris
  oHau = (dwarfLo + 3),   // Haumea
  oMak = (dwarfLo + 4),   // Makemake
  oGon = (dwarfLo + 5),   // Gonggong
  oQua = (dwarfLo + 6),   // Quaoar
  oSed = (dwarfLo + 7),   // Sedna
  oOrc = (dwarfLo + 8),   // Orcus
  oUrT = (moonsLo + 14),  // Uranus' Titania
  oJuC = (cobLo + 0),     // Jupiter COB
  oSaC = (cobLo + 1),     // Saturn COB
  oUrC = (cobLo + 2),     // Uranus COB
  oNeC = (cobLo + 3),     // Neptune COB
  oPlC = (cobLo + 4),     // Pluto COB
  oOri = (starLo-1 + 28), // Alnilam (Orion)
  oAlr = (starLo-1 + 30), // Alnair
  oPle = (starLo-1 + 45), // Alcyone (Pleiades)
  oAnd = (starLo-1 + 46), // Andromeda (M31)
  oGal = (starLo-1 + 49), // Galactic Center (Milky Way)
  cObj = 133,
};

// Aspects
enum _aspects {
  aDis = -8,  // Distances equal
  aNod = -7,  // Latitude zero/node crossing
  aLen = -6,  // Direction change (distance)
  aAlt = -5,  // Direction change (latitude)
  aHou = -4,  // 3D House change
  aDeg = -3,  // Degree change
  aDir = -2,  // Direction change (longitude)
  aSig = -1,  // Sign change
  aCon = 1,
  aOpp = 2,
  aSqu = 3,
  aTri = 4,
  aSex = 5,
  aInc = 6,
  aSSx = 7,
  aSSq = 8,
  aSes = 9,
  aQui = 10,
  aBQn = 11,
  aSQn = 12,
  aSep = 13,
  aNov = 14,
  aBNv = 15,
  aBSp = 16,
  aTSp = 17,
  aQNv = 18,
  aDc3 = 19,
  aUd1 = 20,
  aUd2 = 21,
  aUd3 = 22,
  aUd4 = 23,
  aUd5 = 24,
  cAspect = 24,
  cAspect2 = cAspect + aOpp,   // Parallel and Contraparallel
  cAspect3 = cAspect2 + aOpp,  // Eclipse Conjunct and Opposite
};

// House systems
enum _housesystem {
  hsPlacidus      = 0,
  hsKoch          = 1,
  hsEqual         = 2,
  hsCampanus      = 3,
  hsMeridian      = 4,
  hsRegiomontanus = 5,
  hsPorphyry      = 6,
  hsMorinus       = 7,
  hsTopocentric   = 8,
  hsAlcabitius    = 9,
  hsKrusinski     = 10,
  hsEqualMC       = 11,
  hsSineRatio     = 12,
  hsSineDelta     = 13,
  hsWhole         = 14,
  hsVedic         = 15,
  hsSripati       = 16,
  hsHorizon       = 17,
  hsAPC           = 18,
  hsCarter        = 19,
  hsSunshine      = 20,
  hsSavardA       = 21,
  hsNull          = 22,
  // New experimental house systems follow:
  hsWholeMC       = 23,
  hsVedicMC       = 24,
  hsEqualBalanced = 25,
  hsWholeBalanced = 26,
  hsVedicBalanced = 27,
  hsEqualEP       = 28,
  hsWholeEP       = 29,
  hsVedicEP       = 30,
  hsEqualVertex   = 31,
  hsWholeVertex   = 32,
  hsVedicVertex   = 33,
  hsPorphyryEP    = 34,
  hsPorphyryVtx   = 35,
  hsSineRatioEP   = 36,
  hsSineRatioVtx  = 37,
  hsSineDeltaEP   = 38,
  hsSineDeltaVtx  = 39,
  cSystem = 40,
};

// House system models
enum _housemodel {
  hmEcliptic = 0,   // Standard 2D: Equator of houses is ecliptic
  hmPrime    = 1,   // 3D: Equator of houses is prime vertical
  hmHorizon  = 2,   // 3D: Equator of houses is local horizon
  hmEquator  = 3,   // 3D: Equator of houses is celestial equator
  cMethod = 3,
};

// Progressed chart types
enum _progressiontype {
  ptCast     = 0,
  ptSolarArc = 1,
  ptMixed    = 2,
};

// Relationship chart modes
enum _relationshipchart {
  rcNone       = 0,
  rcSynastry   = 1,
  rcComposite  = 2,
  rcMidpoint   = 3,   // Time space midpoint
  rcDifference = 4,   // Date difference
  rcBiorhythm  = 5,
  rcDual       = -1,  // Bi-wheel
  rcTriWheel   = -2,
  rcQuadWheel  = -3,
  rcQuinWheel  = -4,
  rcHexaWheel  = -5,
  rcTransit    = -6,  // Transit and natal
  rcProgress   = -7,  // Progressed and natal
};

// Aspect configurations
enum _aspectconfigurations {
  acS3 = 0,  // Stellium (3 planets)
  acGT = 1,  // Grand Trine
  acTS = 2,  // T-Square
  acY  = 3,  // Yod
  acGC = 4,  // Grand Cross
  acC  = 5,  // Cradle
  acMR = 6,  // Mystic Rectangle
  acS4 = 7,  // Stellium (4 planets)
  cAspConfig = 8,
};

// Aspect sorting methods
enum _aspectsorting {
  asj = 0,  // By power
  aso = 1,  // By orb magnitude (+/- merged)
  asn = 2,  // By orb value (+/- separated)
  asO = 3,  // By 1st object name
  asP = 4,  // By 2nd object name
  asA = 5,  // By aspect
  asC = 6,  // By 1st object position
  asD = 7,  // By 2nd object position
  asM = 8,  // By midpoint
  asMax = 9,
};

// Angle restrictions
enum _angles {
  arAsc = 0,  // Ascendant
  arMC  = 1,  // Midheaven
  arDes = 2,  // Descendant
  arIC  = 3,  // Nadir
  arMax = 4,
};

// Eclipse types
enum _eclipses {
  etUndefined = -1, // Not checked
  etNone      = 0,  // No eclipse
  etPenumbra  = 1,  // Penumbral eclipse
  etPenumbra2 = 2,  // Total penumbral eclipse
  etPartial   = 3,  // Partial eclipse
  etAnnular   = 4,  // Annular eclipse
  etTotal     = 5,  // Total eclipse
  etMax       = 6,
};

// Rulership restrictions
enum _rulerships {
  rrStd = 0,  // Standard exoteric
  rrEso = 1,  // Esoteric
  rrHie = 2,  // Hierarchical
  rrExa = 3,  // Exaltation
  rrRay = 4,  // Ray rulership
  rrMax = 5,
};

// Decan (or other) division types
enum _decandivisiontype {
  ddNone    = 0,  // No division
  ddDecanR  = 1,  // Decan/Face (ruler)
  ddDecanS  = 2,  // Decan/Face (sign)
  ddChaldea = 3,  // Chaldean Decan
  ddEgypt   = 4,  // Egyptian Terms/Bounds
  ddPtolemy = 5,  // Ptolemaic Terms/Bounds
  ddNavamsa = 6,  // Navamsa division
  dd12      = 7,  // 12th harmonic
  ddDwad    = 8,  // Dwad
  dd27      = 9,  // 27 Nakshatras
  ddConstel = 10, // Constellations
  ddMax     = 11,
};

// Graphics chart modes
enum _graphicschart {
  gWheel      = 1,
  gHouse      = 2,
  gGrid       = 3,
  gMidpoint   = 4,
  gHorizon    = 5,
  gOrbit      = 6,
  gSector     = 7,
  gCalendar   = 8,
  gDisposit   = 9,
  gEsoteric   = 10,
  gAstroGraph = 11,
  gEphemeris  = 12,
  gRising     = 13,
  gLocal      = 14,
  gTraTraGra  = 15,
  gTraNatGra  = 16,
  gMoons      = 17,
  gExo        = 18,
  gSphere     = 19,
  gWorldMap   = 20,
  gGlobe      = 21,
  gPolar      = 22,
  gTelescope  = 23,
  gBiorhythm  = 24,
#if defined(WIN) || defined(FLTK)
  gAspect     = 25,
  gArabic     = 26,
  gTraTraTim  = 27,
  gTraTraInf  = 28,
  gTraNatTim  = 29,
  gTraNatInf  = 30,
  gSign       = 31,
  gObject     = 32,
  gHelpAsp    = 33,
  gConstel    = 34,
  gPlanet     = 35,
  gRay        = 36,
  gMeaning    = 37,
  gSwitch     = 38,
  gObscure    = 39,
  gKeystroke  = 40,
  gCredit     = 41,
  gMax        = 42,
#endif
};

// Colors
enum _colors {
  kReverse = -2,  // Reverse video
  kDefault = -1,  // Default color
  kBlack   = 0,   // Black
  kMaroon  = 1,   // Dark Red
  kDkGreen = 2,   // Dark Green
  kOrange  = 3,   // Dark Yellow
  kDkBlue  = 4,   // Dark Blue
  kPurple  = 5,   // Purple
  kDkCyan  = 6,   // Dark Cyan
  kLtGray  = 7,   // Light Gray
  kDkGray  = 8,   // Dark Gray
  kRed     = 9,   // Red
  kGreen   = 10,  // Green
  kYellow  = 11,  // Yellow
  kBlue    = 12,  // Blue
  kMagenta = 13,  // Magenta
  kCyan    = 14,  // Cyan
  kWhite   = 15,  // White
  kElement = 16,  // Color of element
  kRay     = 17,  // Color of Ray
  kStar    = 18,  // Color of star's brightness
  kPlanet  = 19,  // Color of planet orbited
  kNull    = 16,  // Invalid color
};

// Arabic parts
enum _arabicparts {
  apFor = 0,  // Part of Fortune
  apSpi = 1,  // Part of Spirit
};

// Calculation methods
enum _calculationmethod {
  cmSwiss   = 0,  // Swiss Ephemeris (standard ephemeris files)
  cmMoshier = 1,  // Swiss Ephemeris (Moshier formulas)
  cmJPL     = 2,  // Swiss Ephemeris (JPL ephemeris file)
  cmPlacalc = 3,  // Old Placalc ephemeris
  cmMatrix  = 4,  // Very old Matrix formulas
  cmJPLWeb  = 5,  // JPL Horizons internet Web query
  cmNone    = 6,  // No calculation method
  cmMax     = 7,
};

// Position Display Format
enum _displayformat {
  dfZod = 0,  // Zodiac Position
  dfHM  = 1,  // Hours & Minutes
  df360 = 2,  // 360 Degrees
  dfNak = 3,  // 27 Nakshatras
};

// Draw text formatting flags
enum _drawtext {
  dtCent   = 0x0,   // Default: Center text at coordinates
  dtLeft   = 0x1,   // Left justify text at X coordinate
  dtTop    = 0x2,   // Y coordinate is top of text
  dtBottom = 0x4,   // Y coordinate is bottom of text
  dtErase  = 0x8,   // Erase background behind text
  dtScale  = 0x10,  // Scale text by -Xs character scale
  dtScale2 = 0x20,  // Scale text by -XS text scale
};

// User string parse modes
enum _parsemode {
  pmMon    = 1,   // Month
  pmDay    = 2,   // Day
  pmYea    = 3,   // Year
  pmTim    = 4,   // Time
  pmDst    = 5,   // Daylight Saving offset
  pmZon    = 6,   // Time zone
  pmLon    = 7,   // Longitude
  pmLat    = 8,   // Latitude
  pmElv    = 9,   // Elevation above sea level (in m or feet)
  pmTmp    = 10,  // Temperature of location (in C or F)
  pmDist   = 11,  // Distance (in km or miles)
  pmLength = 12,  // Length (in cm or inches)
  pmObject = 13,  // Planet or other object
  pmAspect = 14,  // Aspect
  pmSystem = 15,  // House system
  pmSign   = 16,  // Sign of the zodiac
  pmColor  = 17,  // Color index
  pmRGB    = 18,  // RGB color value
  pmWeek   = 19,  // Day of week
  pmOffset = 20,  // Zodiac offset ayanamsa
};

// File types
enum _filetype {
  ftNone = 0,  // Not creating a file
  ftBmp  = 1,  // Windows bitmap file (.bmp)
  ftPS   = 2,  // PostScript file (.ps)
  ftWmf  = 3,  // Windows metafile file (.wmf)
  ftWire = 4,  // Daedalus wireframe file (.dw)
};

// File codepage
enum _charactercodepage {
  ccNone  = 0,  // Unknown (don't ever convert)
  ccIBM   = 1,  // Codepage 437 (IBM/DOS/OEM)
  ccLatin = 2,  // Codepage ISO-8859-1 (Latin-1 or Windows-1252)
  ccUTF8  = 3,  // Codepage UTF8 (up to 3 bytes per character)
};

// Font index
enum _fontindex {
  fiAstrolog = 0,  // Astrolog's internal vector drawn characters
  fiWingding = 1,  // Windows Wingdings font
  fiAstro    = 2,  // Astro astrology font
  fiEnigma   = 3,  // EnigmaAstrology astrology font
  fiHamburg  = 4,  // HamburgSymbols astrology font
  fiAstronom = 5,  // Astronomicon astrology font
  fiCourier  = 6,  // Courier (New) text font
  fiConsolas = 7,  // Consolas text font
  fiArial    = 8,  // Arial text font
  fiNakshatr = 9,  // HanksNakshatra font
  cFont = 10,
};

// Letters
enum _letter {
  iLetterQ = 17,
  iLetterR = 18,
  iLetterS = 19,
  iLetterT = 20,
  iLetterU = 21,
  iLetterV = 22,
  iLetterW = 23,
  iLetterX = 24,
  iLetterY = 25,
  iLetterZ = 26,
  cLetter = 26,
};

// Termination codes
enum _terminationcode {
  tcError = -1,
  tcOK    = 0,
  tcFatal = 1,
  tcForce = 2,
};

#endif // ASTROLOG_ENUMS_H

/* astrolog_enums.h */
