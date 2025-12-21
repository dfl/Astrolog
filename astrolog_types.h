/*
** Astrolog (Version 7.80) File: astrolog_types.h
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
** Type Definitions.
******************************************************************************
*/

#ifndef ASTROLOG_TYPES_H
#define ASTROLOG_TYPES_H

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef long word4;
typedef double real;
typedef unsigned char uchar;
typedef unsigned short wchar;
typedef unsigned int uint;
typedef int flag;
typedef byte * pbyte;

typedef int KI;
typedef unsigned long KV;

typedef struct _StrLook {
  char *sz;  // The string in the table
  int isz;   // Number the string maps to
} StrLook;

typedef struct _StrLookR {
  char *sz;  // The string in the table
  real r;    // Real number the string maps to
} StrLookR;

typedef struct _PT2S {
  word x;
  word y;
} PT2S;

typedef struct _PT3R {
  real x;
  real y;
  real z;
} PT3R;

typedef struct _CIRC {
  int xc;  // Center coordinates of circle
  int yc;
  int xr;  // Radius of circle
  int yr;
} CIRC;

typedef struct _TELE {
  real xCent;   // Center pixel coordinates of chart
  real yCent;
  real xBase;   // Chart coordinates of center
  real yBase;
  real xScale;  // Scale of chart
  real yScale;
  real lon;     // Final coordinates used for point
  real lat;
} TELE;

typedef struct _GridInfo {
  byte n[objMax][objMax];  // Name of aspect, or sign of midpoint
  real v[objMax][objMax];  // Value of aspect orb, or degree within sign
} GridInfo;

typedef struct _CrossInfo {
  short obj1;  // First planet making crossing
  short ang1;  // Angle in question of first planet
  short obj2;  // Second planet making crossing
  short ang2;  // Angle in question of second planet
  real lon;    // Longitude of crossing location
  real lat;    // Latitude of crossing location
} CrossInfo;

typedef struct _InDayInfo {
  short source;  // First planet
  short aspect;  // The aspect first planet makes with second planet
  int dest;      // Larger type because may store fraction of degree
  char mon;      // Month of event
  char day;      // Day of event
  short yea;     // Year of event
  real time;     // Time of event in hours
  real pos1;     // Zodiac position of first planet
  real pos2;     // Zodiac position of second planet
  real ret1;     // First planet's zodiac position velocity
  real ret2;     // Second planet's zodiac position velocity
} InDayInfo;

typedef struct _TransInfo {
  short source;  // Transiting planet
  short aspect;  // The aspect transiting planet makes to natal planet
  int dest;      // Natal planet
  real time;     // Time of transit in minutes
  real posT;     // Zodiac position of transiting planet
  real posN;     // Zodiac position of natal planet
  real retT;     // Transiting planet's zodiac position velocity
} TransInfo;

typedef struct _ExoplanetData {
  char *sz;      // The name of the exoplanet
  real ra;       // RA of exoplanet's location
  real dec;      // Declination of exoplanet's location
  real mag;      // Magnitude of exoplanet's star
  real epoch;    // Epoch of middle of exoplanet transit
  real epochU;   // Uncertainty offset of exoplanet epoch
  real period;   // Period of exoplanet transit in days
  real periodU;  // Uncertainty of exoplanet period in days
  real dur;      // Duration of exoplanet transit in hours
  real JDLoop;   // Time of current exoplanet transit
  KI kiLoop;     // Color used for current exoplanet transit
} ExoData;

typedef struct _AtlasEntry {
  real lon;              // Longitude of city
  real lat;              // Latitude of city
  short icn;             // Country or region of city
  short istate;          // State or province of city, if US or CA
  char szNam[cchSzAtl];  // Name of city
  short izn;             // Time zone area of city
} AtlasEntry;

typedef struct _TimezoneChange {
  int zon;      // Time zone value (in seconds before UTC)
  int irun;     // Daylight Saving rule (if any)
  int dst;      // Daylight offset to always use (if no rule)
  short yea;    // Year time zone value ends
  char mon;     // Month time zone value ends
  char day;     // Day time zone value ends
  int tim;      // Time that time zone value ends (in seconds)
  int timtype;  // Type of time (0=local, 1=standard, 2=UTC)
} ZoneChange;

typedef struct _TimezoneRuleName {
  char szNam[cchSzZon];  // Name of rule
  int irue;              // Start index of rule entries
} RuleName;

typedef struct _TimezoneRuleEntry {
  short yea1;    // Start year rule applies to
  short yea2;    // End year rule applies to
  char mon;      // Month in each year rule takes place
  char daytype;  // Type of day (0=num, 1=lastDOW, 2=DOW>=num, 3=DOW<=num)
  char daynum;   // Day within month rule takes place
  char dayweek;  // Day of week (DOW) rule takes place
  int tim;       // Time within day rule takes place
  int timtype;   // Type of time (0=local, 1=standard, 2=UTC)
  int dst;       // Rule applies this Daylight offset (in seconds before UTC)
} RuleEntry;

typedef struct _ChartInfo {
  int mon;    // Month
  int day;    // Day
  int yea;    // Year
  real tim;   // Time in hours
  real dst;   // Daylight offset
  real zon;   // Time zone
  real lon;   // Longitude
  real lat;   // Latitude
  char *nam;  // Name for chart
  char *loc;  // Name of location
} CI;

typedef struct _ChartPositions {
  real obj[objMax];     // The zodiac positions
  real alt[objMax];     // Ecliptic declination
  real dir[objMax];     // Retrogradation velocity
  real diralt[objMax];  // Latitude velocity
  real dirlen[objMax];  // Distance velocity
  PT3R pt[objMax];      // X,Y,Z coordintes in space
  real dist[objMax];    // Distance to X,Y,Z coordinates
  real cusp[cSign+1];   // House cusp positions
  real cusp3[cSign+1];  // 3D house cusp positions
  int house[objMax];    // House each object is in
  real lonMC;           // 0 longitude converted to equatorial coordinates
} CP;

#ifdef GRAPH
typedef struct _ObjDraw {
  int obj;  // The object to draw
  int x;    // Horizontal pixel position of object
  int y;    // Vertical pixel position of object
  int yg;   // Vertical pixel position of glyph
  KV kv;    // Color to use when drawing object
  flag f;   // Whether to draw object at all
} ObjDraw;
#endif

typedef word * TransGraInfo[objMax][objMax][cAspect+1];

typedef struct _ExtraStar {
  real lon;           // Zodiac position.
  real lat;           // Vertical latitude.
  real dir;           // Velocity or speed.
  real mag;           // Magnitude or brightness.
  PT3R pt;            // Coordinates in 3D space.
  char sz[cchSzDef];  // Buffer for name of star.
  char *pchNam;       // Star's classic name.
  char *pchDes;       // Star's designation name.
  char *pchBest;      // Best available name.
  KI ki;              // Color to use for star.
} ES;

typedef struct _UserSettings {

  // Chart types
  flag fListing;        // -v
  flag fWheel;          // -w
  flag fGrid;           // -g
  flag fAspList;        // -a
  flag fMidpoint;       // -m
  flag fHorizon;        // -Z
  flag fOrbit;          // -S
  flag fSector;         // -l
  flag fInfluence;      // -j
  flag fEsoteric;       // -7
  flag fAstroGraph;     // -L
  flag fCalendar;       // -K
  flag fInDay;          // -d
  flag fInDayInf;       // -D
  flag fEphemeris;      // -E
  flag fArabic;         // -P
  flag fHorizonSearch;  // -Zd
  flag fTransit;        // -t
  flag fTransitInf;     // -T
  flag fInDayGra;       // -B
  flag fTransitGra;     // -V
  flag fAtlasLook;      // -N
  flag fAtlasNear;      // -Nl
  flag fZoneChange;     // -Nz
  flag fMoonChart;      // -8
  flag fExoTransit;     // -Ux

  // Chart suboptions
  flag fVelocity;       // -v0
  flag fListDecan;      // -v3
  flag fWheelReverse;   // -w0
  flag fGridConfig;     // -g0 (on by default)
  flag fGridMidpoint;   // -gm
  flag nAppSep;         // -ga
  flag fParallel;       // -gp
  flag fDistance;       // -gd
  flag fAspSummary;     // -a0 (on by default)
  flag fMidSummary;     // -m0 (on by default)
  flag fMidAspect;      // -ma
  flag fPrimeVert;      // -Z0
  flag fSectorApprox;   // -l0
  flag fInfluenceSign;  // -j0 (on by default)
  flag fLatitudeCross;  // -L0 (on by default)
  flag fCalendarYear;   // -Ky
  flag fInDayMonth;     // -dm
  flag fInDayYear;      // -dy
  flag fGraphAll;       // -B0
  flag fArabicFlip;     // -P0
  flag fMoonChartSep;   // -80

  // Table chart types
  flag fCredit;          // -Hc
  flag fSwitch;          // -H
  flag fSwitchRare;      // -Y
  flag fKeyGraph;        // -HX
  flag fSign;            // -HC
  flag fObject;          // -HO
  flag fAspect;          // -HA
  flag fConstel;         // -HF
  flag fOrbitData;       // -HS
  flag fRay;             // -H7
  flag fMeaning;         // -HI

  // Main flags
  flag fLoop;        // -Q
  flag fSidereal;    // -s
  flag fCusp;        // -C
  flag fUranian;     // -u
  flag fDwarf;       // -u0
  flag fMoons;       // -u8
  flag fCOB;         // -ub
  flag fStar;        // -U
  flag fProgress;    // Are we doing a -p progressed chart?
  flag fInterpret;   // Is -I interpretation switch in effect?
  flag fHouse3D;     // -c3
  flag fAspect3D;    // -A3
  flag fAspectLat;   // -Ap
  flag fParallel2;   // -AP
  flag fDecan;       // -3
  flag fFlip;        // -f
  flag fGeodetic;    // -G
  flag fIndian;      // -J
  flag fNavamsa;     // -9
  flag fEphemFiles;  // -b
  flag fWriteFile;   // -o
  flag fAnsiColor;   // -k
  flag fGraphics;    // -X

  // Main subflags
  flag fNoSwitches;
  flag fLoopInit;    // -Q0
  flag fSeconds;     // -b0
  flag fSecond1K;    // -b1
  flag fPlacalcAst;  // -ba
  flag fPlacalcPla;  // -bp
  flag fMatrixPla;   // -bm
  flag fMatrixStar;  // -bU
  flag fEquator;     // -sr
  flag fEquator2;    // -sr0
  flag fAnsiChar;    // -k0
  flag fTextHTML;    // -kh
  flag fSolarWhole;  // -10
  flag fListAuto;    // -5

  // Obscure flags
  flag fTruePos;       // -YT
  flag fTopoPos;       // -YV
  flag fRefract;       // -Yf
  flag fBarycenter;    // -Yh
  flag fMoonMove;      // -Ym
  flag fSidereal2;     // -Ys
  flag fTrueNode;      // -Yn
  flag fNoNutation;    // -Yn0
  flag fOffsetOnly;    // -Yn1
  flag fNaturalNode;   // -Ynn
  flag fEuroDate;      // -Yd
  flag fEuroTime;      // -Yt
  flag fEuroDist;      // -Yv
  flag fRound;         // -Yr
  flag fSmartCusp;     // -YC
  flag fSmartSave;     // -YO
  flag fClip80;        // -Y8
  flag fWriteOld;      // -Yo
  flag fHouseAngle;    // -Yc
  flag fPolarAsc;      // -Yp
  flag fEclipse;       // -Yu
  flag fEclipseAny;    // -Yu0
  flag fObjRotWhole;   // -Y10
  flag fIgnoreSign;    // -YR0
  flag fIgnoreDir;     // -YR0
  flag fIgnoreDiralt;  // -YR1
  flag fIgnoreDirlen;  // -YR1
  flag fIgnoreAlt0;    // -YR2
  flag fIgnoreDisequ;  // -YR2
  flag fIgnoreAuto;    // -YRh
  flag fStarsList;     // -YRU0
  flag fStarMagDist;   // -YUb
  flag fStarMagAbs;    // -YUb0
  flag fNoDisplay;     // -Y0
  flag fNoWrite;       // -0o
  flag fNoRead;        // -0i
  flag fNoQuit;        // -0q
  flag fNoGraphics;    // -0X
  flag fNoPlacalc;     // -0b
  flag fNoNetwork;     // -0n
  flag fNoExp;         // -0~
  flag fExpOff;        // -~0

  // Value settings
  int   nDecanType;    // -v3
  int   nAspectSort;   // -a
  int   nEphemYears;   // -Ey
  int   nEphemRate;    // -E0
  int   nEphemFactor;  // -E0
  int   nArabicSort;   // -P
  int   nRel;          // What relationship chart is in effect, if any?
  int   nSwissEph;     // -bs
  int   nHouseSystem;  // -c
  int   nHouse3D;      // -c3
  int   nAsp;          // -A
  int   objCenter;     // -h
  int   nDwad;         // -4
  int   nStarSort;     // -U
  real  rHarmonic;     // Harmonic chart value passed to -x switch.
  int   objOnAsc;      // Planet value passed to -1 or -2 switch.
  int   objRequire;    // Required object passed to -RO switch.
  int   nDegForm;      // -s
  int   nProgress;     // -p0
  int   nDivision;     // -d
  int   nScreenWidth;  // -I
  int   nWriteFormat;  // -o
  int   nListAll;      // -5e
  real  elvDef;        // -zv
  real  tmpDef;        // -zf
  char *rgszPath[10];  // -Yi
  char *szADB;         // -Y5i
  char *szAstColor;    // -YkE
  char *szStarsColor;  // -YkU
  char *szStarsList;   // -YRU
  char *szExoList;     // -YUx

  // Value subsettings
  int   nWheelRows;        // Number of rows per house to use for -w wheel.
  int   nAstroGraphStep;   // Latitude step rate passed to -L switch.
  int   nAstroGraphDist;   // Maximum crossing distance passed to -L0 switch.
  int   nArabicParts;      // Arabic parts to include value passed to -P.
  int   nAtlasList;        // Number of rows to display value passed to -N.
  real  rZodiacOffset;     // Position shifting value passed to -s switch.
  real  rZodiacOffsetAll;  // Position shifting value passed to -Ys switch.
  real  rProgDay;          // Progression day value passed to -pd switch.
  real  rProgCusp;         // Progression cusp ratio value passed to -pC.
  int   nRatio1;           // Chart ratio factors passed to -rc or -rm.
  int   nRatio2;
  int   nCharset;          // -Ya
  int   nCharsetOut;       // -Yao
  int   nScrollRow;        // -YQ
  int   cSequenceLine;     // -Yq
  long  lTimeAddition;     // -Yz
  real  rDeltaT;           // -Yz0
  real  rStation;          // -Yw
  real  rObjAddition;      // -YzO
  real  rCuspAddition;     // -YzC
  int   objRot1;           // -Y1
  int   objRot2;           // -Y1
  int   nHorizon;          // -YZ
  int   nArabicNight;      // -YP
  int   nBioday;           // -Yb
  int   nSignDiv;          // -YRd
  int   iExpADB;           // -~5i
  int   cExpADB;           // -~5i

  // AstroExpression hooks
  char *szExpConfig;   // -~g
  char *szExpAspList;  // -~a
  char *szExpAspSumm;  // -~a0
  char *szExpMid;      // -~m
  char *szExpMidAsp;   // -~ma
  char *szExpInf;      // -~j
  char *szExpInf0;     // -~j0
  char *szExpEso;      // -~7
  char *szExpCross;    // -~L
  char *szExpEph;      // -~E
  char *szExpRis;      // -~Zd
  char *szExpDay;      // -~d
  char *szExpVoid;     // -~dv
  char *szExpTra;      // -~t
  char *szExpPart;     // -~P
  char *szExpObj;      // -~O
  char *szExpHou;      // -~C
  char *szExpAsp;      // -~A
  char *szExpProg;     // -~p
  char *szExpProg0;    // -~p0
  char *szExpColObj;   // -~kO
  char *szExpColAsp;   // -~kA
  char *szExpColFill;  // -~kv
  char *szExpFontSig;  // -~F
  char *szExpFontHou;  // -~FC
  char *szExpFontObj;  // -~FO
  char *szExpFontAsp;  // -~FA
  char *szExpFontNak;  // -~FN
  char *szExpSort;     // -~v
  char *szExpDecan;    // -~v3
  char *szExpDegree;   // -~sd
  char *szExpStar;     // -~U
  char *szExpAst;      // -~U0
  char *szExpExo;      // -~Ux
  char *szExpIntV;     // -~Iv
  char *szExpIntV2;    // -~IV
  char *szExpIntA;     // -~Ia
  char *szExpIntA2;    // -~IA
  char *szExpCity;     // -~XL
  char *szExpSidebar;  // -~Xt
  char *szExpKey;      // -~XQ
  char *szExpMenu;     // -~WQ
  char *szExpCast1;    // -~q1
  char *szExpCast2;    // -~q2
  char *szExpDisp1;    // -~Q1
  char *szExpDisp2;    // -~Q2
  char *szExpDisp3;    // -~Q3
  char *szExpListS;    // -~5s
  char *szExpListF;    // -~5f
  char *szExpListY;    // -~5Y
  char *szExpADB;      // -~5i
} US;

typedef struct _InternalSettings {
  flag fHaveInfo;      // Do we need to prompt user for chart info?
  flag fDst;           // Has Daylight Saving Time been autodetected?
  flag fProgress;      // Are we doing a chart involving progression?
  flag fReturn;        // Are we doing a transit chart for returns?
  flag fMult;          // Have we already printed at least one text chart?
  flag fSzInteract;    // Are we in middle of chart so some setting fixed?
  flag fNoEphFile;     // Have we already had a ephem file not found error?
  flag fSwissPathSet;  // Has the Swiss Ephemeris path been set yet?
  char *szProgName;    // The name and path of the executable running.
  char *rgszLine[9];   // The command lines to run before each -Yq chart.
  char *szFileScreen;  // The file to send text output to as passed to -os.
  char *szFileOut;     // The output chart filename string as passed to -o.
  char **rgszComment;  // Points to any comment strings after -o filename.
  int nContext;        // Context of current or most recent chart cast.
  int nObj;            // Index of highest unrestricted object.
  int cszComment;      // The number of strings after -o that are comments.
  int cchRow;          // The current row text charts have scrolled to.
  int cchCol;          // The current column text charts are printing at.
  int cchColMax;       // Max column current text chart has printed at.
  int nHTML;           // HTML text output context mode for -kh.
  int nHouseSystem;    // Actual house system used to compute cusps for -c.
  int nWheelRows;      // Actual number of rows per house to use for -w.
  int cae;             // Number of atlas entries of city locations loaded.
  int czcn;            // Number of time zone change areas loaded.
  int czce;            // Total number of change entries in all zone areas.
  int crun;            // Number of time zone Daylight rule categories.
  int crue;            // Total number of rule entries in all categories.
  int cci;             // Number of user visible charts in chart list.
  int cciAlloc;        // Number of allocated positions in chart list.
  int iciCur;          // Index of most recently used chart in chart list.
  int iciIndex1;       // Index into chart list used for chart #1 with -5e.
  int iciIndex2;       // Index into chart list used for chart #2 with -5e3.
  int cexod;           // Number of exoplanet transit stars loaded from file.
  int cszMacro;        // Number of command switch macro strings in list.
  int cesSort;         // Number of extra star structures in sorted list.
  int cAlloc;          // Number of memory allocations currently allocated.
  int cAllocTotal;     // Total memory allocations allocated this session.
  int cbAllocSize;     // Total bytes in all memory allocations allocated.
  real rOff;           // Offset between sidereal and tropical zodiacs.
  real rSid;           // Sidereal offset degrees to be added to locations.
  real JD;             // Fractional Julian day for current chart.
  real JDp;            // Julian day that a progressed chart indicates.
  real Tp;             // Julian time used for progressed chart cusps.
  AtlasEntry *rgae;    // List of atlas entries for city coordinates.
  ZoneChange *rgzc;    // List of time zone change entries for zone areas.
  RuleName *rgrun;     // List of Daylight Saving change rule names.
  RuleEntry *rgrue;    // List of all Daylight Saving change rule entries.
  real *rgzonCol;      // Cache of time zone offsets for each zone area.
  CI *rgci;            // List of chart information records for chart list.
  ExoData *rgexod;     // List of exoplanet transit stars loaded from file.
  char **rgszMacro;    // List of command switch macro strings.
  ES *rgesSort;        // List of sorted extra stars or extra asteroids.
  FILE *fileIn;        // The switch file currently being read from.
  FILE *S;             // File to write text to.
  real T;              // Julian time for chart.
  real MC;             // Midheaven at chart time.
  real Asc;            // Ascendant at chart time.
  real EP;             // East Point at chart time.
  real Vtx;            // Vertex at chart time.
  real RA;             // Right ascension at time.
  real OB;             // Obliquity of ecliptic.
  real rDeltaT;        // Delta-T at chart time, in days.
  real jdDeltaT;       // JD for cached Delta-T offset above.
  real rNut;           // Nutation offset.
} IS;

#ifdef GRAPH
typedef struct _Bitmap {
  int x;      // Horizontal pixel size of bitmap
  int y;      // Vertical pixel size of bitmap
  int clRow;  // Longs per row in bitmap
  byte *rgb;  // Bytes of bitmap bits
} Bitmap;

typedef struct _GraphicsSettings {
  int ft;            // File type being created (-Xb, -Xp, -XM, or -X3).
  flag fPSComplete;  // Is PostScript file not encapsulated (-Xp0 set).
  flag fColor;       // Are we drawing a color chart (-Xm not set).
  flag fInverse;     // Are we drawing in reverse video (-Xr set).
  flag fRoot;        // Are we drawing on the X11 background (-XB set).
  flag fText;        // Are we printing chart info on chart (-Xt set).
  flag fDoSidebar;   // Are we drawing wheel chart sidebar (-Xv0 set).
  int nFontAll;      // Which fonts to use for sign/house/obj/asp (-YXf).
  int nFontTxt;      // Which font to use for graphics text       (-YXft).
  int nFontSig;      // Which font to use for signs of the zodiac (-YXfs).
  int nFontHou;      // Which font to use for labelling houses    (-YXfh).
  int nFontObj;      // Which font to use for planets and objects (-YXfo).
  int nFontAsp;      // Which font to use for aspects             (-YXfa).
  int nFontNak;      // Which font to use for Vedic Nakshatras    (-YXfn).
  flag fAlt;         // Are we drawing in alternate mode (-Xi set).
  flag fBorder;      // Are we drawing borders around charts (-Xu set).
  flag fLabel;       // Are we labeling objects in charts (-Xl not set).
  flag fLabelAsp;    // Are we drawing aspect glyphs on lines (-XA set).
  flag fLabelCity;   // Are we plotting cities on maps (-XL set).
  flag fJetTrail;    // Are we not clearing screen on updates (-Xj set).
  flag fConstel;     // Are we drawing maps as constellations (-XF set).
  flag fSouth;       // Are we focus on south hemisphere (-XX0/XP0 set).
  flag fMollewide;   // Are we drawing maps scaled correctly (-XW0 set).
  flag fEquator;     // Are we showing equator on maps/globes (-Xe set).
  flag fEcliptic;    // Are we drawing oriented to ecliptic (-YXe set).
  flag fAllStar;     // Are we drawing all sefstars.txt stars (-XU set).
  flag fAllExo;      // Are we drawing astexo.csv exoplanets (-XUx set).
  flag fHouseExtra;  // Are we showing additional house info (-XC set).
  flag fPrintMap;    // Are we printing globe names on draw (-XPv set).
  flag fKeepSquare;  // Are we preserving chart aspect ratio (-XQ set).
  flag fAnimMap;     // Are we animating map instead of time (-XN set).
  flag fThick;       // Are we drawing thicker lines in charts (-Xx set).
  flag fAntialias;   // Are we drawing lines antialiased (-Xx0 set).
  int nDashStyle;    // Line style: 0=dotted, 1=alpha, 2=lens (-Xx1 <n>).
  flag fIndianWheel; // Are wheel charts North/South Indian (-XJ set).
  flag fMoonWheel;   // Are moons drawn around planets in wheels (-X8 set).
  int xWin;          // Current hor. size of graphic chart (-Xw).
  int yWin;          // Current ver. size of graphic chart (-Xw).
  int nAnim;         // Current animation mode jump rate (-Xn).
  int nScale;        // Current character scale factor (-Xs).
  int nScaleText;    // Current graphics text scale factor (-XS).
  int nAllStar;      // Extra star size and labelling (-XU).
  int nAstLo;        // Extra asteroid min ephemeris file (-XE).
  int nAstHi;        // Extra asteroid max ephemeris file (-XE).
  int nAstLabel;     // How extra asteroids get labeled (-XE).
  int nLabelCity;    // How city plottings get colored (-XL).
  int objLeft;       // Current object to place on Asc (-X1).
  real rRot;         // Current rotation degree of globe (-XG).
  real rTilt;        // Current vertical tilt of rotating globe (-XG).
  int objTrack;      // Object being telescope tracked, if any (-XZ).
  char chBmpMode;    // Current bitmap file type (-Xb).
  real rBackPct;     // Background image transparency percentage (-XI).
  int nBackOrient;   // Background image wallpaper orientation (-XI).
  int nOrient;       // PostScript paper orientation indicator.
  real xInch;        // PostScript horizontal paper size inches.
  real yInch;        // PostScript vertical paper size inches.
  char *szDisplay;   // Current X11 display name (-Xd).
  int nDecaType;     // Type of wheel chart decoration (-YXv).
  int nDecaSize;     // Size of wheel chart decoration (-YXv).
  int nDecaLine;     // Lines in wheel chart decoration (-YXv).
  int nDecaFill;     // Fill method for wheel chart sections (-Xv).
  char *szSidebar;   // Extra text to append to bottom of sidebar (-YXt)
  int nGridCell;     // Number of cells in -g grids (-YXg).
  real rspace;       // Radius in AU of -S orbit chart (-YXS).
  int cspace;        // Number of -S orbit trails allowed (-YXj).
  int zspace;        // Height diff of each orbit trail (-YXj0).
  int nRayWidth;     // Column width in -7 esoteric chart (-YX7).
  int nGlyphCap;     // Glyph to use for sign Capricorn (-YXGc).
  int nGlyphUra;     // Glyph to use for planet Uranus (-YXGu).
  int nGlyphPlu;     // Glyph to use for planet Pluto (-YXGp).
  int nGlyphLil;     // Glyph to use for object Lilith (-YXGl).
  int nGlyphVer;     // Glyph to use for object Vertex (-YXGv).
  int nGlyphEri;     // Glyph to use for planet Eris (-YXGe).
  flag fColorSign;   // More color for sign boundaries. (-YXk).
  flag fColorHouse;  // More color for house boundaries. (-YXk0).
  flag fAltPalette;  // Use alternate palette for white backgrounds (-YXK0).
  int nReduceContrast; // Reduce color contrast (0-100%) (-YXKr).
  int nDashMax;      // Maximum dash allowed for lines (-YXA).
  int nTriangles;    // Triangles/cubes grid to draw on maps, if any (-YXW).
  char *szStarsLin;  // Names of extra stars for linking (-YXU).
  char *szStarsLnk;  // Indexes of star pairs to link up (-YXU).
} GS;

typedef struct _GraphicsInternal {
  int nMode;          // Current type of chart to create.
  flag fMono;         // Is this a monochrome display.
  int kiCur;          // Current color drawing with.
  pbyte bm;           // Pointer to allocated memory.
  int cbBmpRow;       // Horizontal size of bitmap array in memory.
  char *szFileOut;    // Current name of bitmap file (-Xo).
  FILE *file;         // Actual file handle writing graphics to.
  real rAsc;          // Degree to be at left edge in wheel charts.
  flag fFile;         // Are we making a graphics file.
  flag fDidSphere;    // Has a chart sphere been drawn once yet?
  real zViewRatio;    // Offset to adjust chart view (based on zoom).
  int nScale;         // Scale ratio, e.g. percentage / 100.
  int nScaleText;     // Text scale ratio, i.e. percentage / 50.
  int nScaleT;        // Internal units per pixel (1 for screen).
  int nScaleTextT;    // Internal text scale, i.e. nScaleText * nScaleT.
  int nScaleTextT2;   // Twice internal text scale, i.e. nScaleTextT * 2.
  real rScaleX;       // Floating-point X scale for smooth map resizing.
  real rScaleY;       // Floating-point Y scale for smooth map resizing.
  int nGridCell;      // Actual number of cells in -g grids.
  int nPenWid;        // Pen width to use when creating metafiles.
  int nFontPrev;      // System fonts to restore when turning fonts on.
  KI kiOn;            // Foreground color.
  KI kiOff;           // Background color.
  KI kiLite;          // Hilight color.
  KI kiGray;          // A "dim" color.
  int xOffset;        // Viewport origin.
  int yOffset;
  int xTurtle;        // Current coordinates of drawing pen.
  int yTurtle;
  int xPen;           // Cached coordinates where last line ended.
  int yPen;
  PT3R *rgspace;      // List of orbit trail coordinates (-S -X).
  int ispace;         // Index of most recent coordinate (-S -X).
  int cspace;         // Coordinates within table so far (-S -X).
  int *rgzCalendar;   // Aspect coordinates in calendar (-K -X).
  char *rgConstel;    // Constellation equatorial coordinate lookup array.
  flag fBmp;          // Are 24 bit bitmaps being used? (-Xbw set).
  Bitmap bmp;         // Bitmap storing chart contents, sized appropriately.
  Bitmap bmpBack;     // Bitmap storing background, as loaded from file.
  Bitmap bmpBack2;    // Bitmap storing background, at current transparency.
  Bitmap bmpWorld;    // Bitmap storing world map, as loaded from file.
  Bitmap bmpRising;   // Bitmap storing rising chart, as drawn within it.
#ifdef SWISS
  ES *rges;           // List of extra star coordinates (-YXU).
  int cStarsLin;      // Count of extra star coordinates (-YXU).
#endif
#ifdef ISG
  KI kiPen;           // The current pen scribble color (-Xk).
  flag fPause;        // Is animation paused? (-Xnp set).
  int nDir;           // Animation step factor and direction (-Xnf).
  int xWinResize;     // Manually resized hor. size of graphic chart.
  int yWinResize;     // Manually resized ver. size of graphic chart.
#endif
#ifdef X11
  Display *disp;      // The opened X11 display (-Xd).
  GC gc, pmgc;
  Pixmap pmap, icon;
  Window wind, root;
  int screen;
  int depth;          // Number of active color bits.
#endif
#ifdef PS             // Variables used by the PostScript generator.
  flag fEps;          // Are we doing Encapsulated PostScript.
  int cStroke;        // Number of items drawn without flushing.
  flag fLineCap;      // Are line ends rounded instead of square.
  int nDash;          // How much long are dashes in lines drawn.
  int nFontPS;        // What system font are we drawing text in.
  real rLineWid;      // How wide are lines, et al, drawn with.
#endif
#ifdef META           // Variables used by the metafile generator.
  word *pwMetaCur;    // Current mem position when making metafile.
  long cbMeta;        // Maximum size allowed for metafile.
  word *pwPoly;       // Position for start of current polyline.
  KI kiPoly;          // Line color for current polyline.
  KI kiLineAct;       // Desired and actual line color.
  KI kiLineDes;
  KI kiFillAct;       // Desired and actual fill color.
  KI kiFillDes;
  int nFontAct;       // Desired and actual text font.
  int nFontDes;
  KI kiTextAct;       // Desired and actual text color.
  KI kiTextDes;
  int nAlignAct;      // Desired/actual text alignment.
  int nAlignDes;
#endif
#ifdef WIRE           // Variables used by the wireframe generator.
  word *pwWireCur;    // Current memory position when doing wireframe.
  long cbWire;
  int cWire;          // Number of lines in wireframe file.
  KI kiInFile;        // Actual line color currently in file.
  int zDefault;       // Default elevation for 2D drawing.
#endif
} GI;
#endif // GRAPH

typedef struct _ArabicInfo {
  char *form;                 // The formula to calculate it.
  char *name;                 // The name of the Arabic part.
} AI;

typedef struct _ElementTable {
  int coSum;             // Total objects considered.
  int coHemi;            // Number that can be in hemispheres.
  int coSign[cSign];     // Number of objects in each sign.
  int coHouse[cSign];    // Number of objects in each house.
  int coElemMode[4][3];  // Objects in each elem/mode combo.
  int coElem[4];         // Objects in each element.
  int coMode[3];         // Objects in each sign mode.
  int coModeH[3];        // Objects in each house mode.
  int coYang;            // Objects in Fire/Air signs.
  int coYin;             // Objects in Earth/Water signs.
  int coLearn;           // Objects in first six signs.
  int coShare;           // Objects in last six signs.
  int coAsc;             // Objects in Eastern houses.
  int coDes;             // Objects in Western houses.
  int coMC;              // Objects in Southern houses.
  int coIC;              // Objects in Northern houses.
} ET;

typedef struct _OrbitalElements {
  real ma0, ma1, ma2;  // Mean anomaly.
  real ec0, ec1, ec2;  // Eccentricity.
  real sma;            // Semi-major axis.
  real ap0, ap1, ap2;  // Argument of perihelion.
  real an0, an1, an2;  // Ascending node.
  real in0, in1, in2;  // Inclination.
} OE;

#define szFileJPLCore  "astrolog.jpl"
#define szFileExoCore  "astexo.csv"
#ifdef WIN
#define szFileTempCore "astrolog.tmp"
#define szFileAutoCore "astrolog.bmp"
#define nScrollDiv 24
#define nScrollPage 6
#define cchSzMaxFile 128

typedef struct _WindowInternal {
  HINSTANCE hinst;   // Instance of the Astrolog window class.
  HWND hwndMain;     // The outer created frame window.
  HWND hwnd;         // The current window being dealt with.
  HMENU hmenu;       // The Astrolog main menu bar.
  HACCEL haccel;     // Keyboard accelerator or shortcut table.
  HDC hdc;           // The current DC bring drawn upon.
  HDC hdcPrint;      // The current DC being printed upon.
  HDC hdcBack;       // The current DC for the background bitmap.
  HWND hwndAbort;    // Window of the printing abort dialog.
  HPEN hpen;         // Pen with the current line color.
  HBRUSH hbrush;     // Fill if any with the current color.
  HFONT hfont;       // Font of current text size being printed.
  HBITMAP hbmp;      // Bitmap storing contents to be copied to window.
  HBITMAP hbmpBack;  // Bitmap for the background image.
  HBITMAP hbmpPrev;  // Bitmap to restore after using background.
  HANDLE hMutex;     // To ensure output file isn't already open.
  size_t lTimer;     // Identifier for the animation timer.
  short xScroll;     // Horizontal & vertical scrollbar position.
  short yScroll;
  short xClient;     // Horizontal & vertical window size.
  short yClient;
  int xChar;         // Horizontal & vertical font character size.
  int yChar;
  int xMouse;        // Horizontal & vertical mouse position.
  int yMouse;
  WORD wCmd;         // The currently invoked menu command.
  LPARAM lParamRC;   // Coordinates where right click originated.
  int nMode;         // New chart type to switch to if any.
  int cmdCur;        // Menu command for current chart type.
  flag fMenu;        // Do we need to repaint the menu bar?
  flag fMenuAll;     // Do we need to redetermine all menu checks?
  flag fNotManual;   // Is window being resized automatically?
  flag fRedraw;      // Do we need to redraw the screen?
  flag fCast;        // Do we need to recast the chart positions?
  flag fAbort;       // Did the user cancel printing in progress?
  flag fMoved;       // Has mouse been dragged since down click?
  flag fSkipSystem;  // If should suppress the next system command.
  int nDlgChart;     // Which chart to set in Open or Info dialog.
  flag fWindowFull;  // Is the window in full screen mode?
  RECT rcFull;       // Former window coordinates if full screen.
  flag fSaverExt;    // Is executable .scr screen saver extension?
  flag fSaverCfg;    // Is screen saver in configuration mode?
  Bitmap bmpWin;     // Bitmap storing contents to be copied to window.

  // Window user settings.
  flag fBuffer;        // Are we drawing updates off screen?
  flag fHourglass;     // Bring up hourglass cursor on redraws?
  flag fChartWindow;   // Does chart change cause window resize?
  flag fWindowChart;   // Does window resize cause chart change?
  flag fNoUpdate;      // Do we not automatically update screen?
  flag fNoPopup;       // Do we not display warning messages?
  flag fStarLine;      // Do we display all constellation lines?
  flag fSaverRun;      // Do we run Astrolog as a screen saver?
  flag fAutoSave;      // Are we saving chart after win draw?
  flag fAutoSaveNum;   // Autosave charts are incremental files?
  flag fAutoSaveWire;  // Autosave wireframe instead of bitmap?
  int nAutoSaveNum;    // Number of incremental bitmap save.
  UINT nTimerDelay;    // Milliseconds between animation draws.
} WI;
#endif

#ifdef WCLI
typedef struct _WindowInternal {
  HINSTANCE hinst;   // Instance of the Astrolog window class.
  HWND hwndMain;     // The outer created frame window.
  HWND hwnd;         // The current window being dealt with.
  HDC hdc;           // The current DC bring drawn upon.
  HDC hdcBack;       // The current DC for the background bitmap.
  HPEN hpen;         // Pen with the current line color.
  HBRUSH hbrush;     // Fill if any with the current color.
  HBITMAP hbmp;      // Bitmap storing contents to be copied to window.
  HBITMAP hbmpBack;  // Bitmap for the background image.
  HBITMAP hbmpPrev;  // Bitmap to restore after using background.
  short xClient;     // Horizontal & vertical window size.
  short yClient;
  flag fNotManual;   // Is window being resized automatically?
  flag fDoRedraw;
  flag fDoResize;
  flag fDoCast;
  flag fWndclass;
  int xMouse;        // Horizontal & vertical mouse position.
  int yMouse;
  LPARAM lParamRC;   // Coordinates where right click originated.
  Bitmap bmpWin;     // Bitmap storing contents to be copied to window.
} WI;
#endif

#endif // ASTROLOG_TYPES_H

/* astrolog_types.h */
