/*
** Astrolog (Version 7.80) File: astrolog_config.h
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
** Feature Flags and Platform Configuration.
******************************************************************************
*/

#ifndef ASTROLOG_CONFIG_H
#define ASTROLOG_CONFIG_H

/*
** SYSTEM SECTION: These settings describe platform and hardware specifics.
** They are all required to be set properly or the program probably won't
** even compile.
*/

//#define PC /* Comment out this #define if you have a Unix, Mac, or other */
           /* system that isn't a generic PC running DOS or MS Windows.  */

#ifndef FLTK  /* X11 is auto-disabled if FLTK is defined by build system */
#define X11 /* Comment out this #define if you don't have X windows, or */
            /* else have them and don't wish to compile in X graphics.  */
#endif

//#define WIN /* Comment out this #define if you don't have MS Windows, or */
            /* else have them but want a command line version instead.   */

//#define WCLI /* Comment out this #define if you don't want to compile a    */
             /* command line Windows version that can still popup windows. */

//#define WSETUP /* Comment out this #define if you don't want to compile a */
               /* modern Windows version that can do its own (un)setup.   */

//#define FLTK /* Define this for cross-platform FLTK GUI. Comment out X11 */
             /* and WIN above when using FLTK.                             */

#define JPLWEB /* Comment out this #define if you don't want to compile in */
               /* features to access the JPL Horizons Website online.      */

#define TIME /* Comment out this #define if your compiler can't take the  */
             /* calls to the 'time' or 'localtime' functions as in time.h */

#define SWITCHES /* Comment out this #define if your system can not handle */
                 /* parameters on the command line (such as old Mac's).    */

#define ENVIRON /* Comment out this #define if your system doesn't have  */
                /* environment variables or can't compile calls to them. */

//#define ATOF /* Comment out this #define if you have a system in which  */
             /* 'atof' and related functions aren't defined in stdio.h, */
             /* such as most PC's, Linux, VMS compilers, and NeXT's.    */

#define PROTO /* Comment out this #define if you have an ancient compiler */
              /* which doesn't allow full Ansi function prototypes. This  */
              /* is for programmers only and has no effect on executable. */

/*
** FEATURES SECTION: These settings describe features that are always
** available to be compiled into the program no matter what platform or
** hardware is available.
*/

/* Use #ifndef guards to avoid redefinition warnings when macros are
** defined via compiler flags (e.g., from CMake or command line).
*/

#ifndef GRAPH
#define GRAPH /* Comment out this #define if you don't want any graphics   */
#endif        /* in the program. This switch allows at least generation of */
              /* bitmap files and must be set if any of the more advanced  */
              /* graphics feature additions are also compiled in.          */

#ifndef SWISS
#define SWISS /* Comment out this #define if you don't want the Swiss      */
#endif        /* Ephemeris most accurate calculation features and formulas */
              /* to be compiled into the program (as accessed with -b).    */

#ifndef PLACALC
#define PLACALC /* Comment out this #define if you don't want the Placalc */
#endif          /* less accurate calculation features and formulas to be  */
                /* compiled into the program (as accessed with -bp).      */

#ifndef MATRIX
#define MATRIX /* Comment out this #define if you don't want the Matrix  */
#endif         /* much less accurate calculation formulas to be compiled */
               /* into the program (as accessed with -bm).               */

#ifndef PS
#define PS /* Comment out this #define if you don't want the ability to */
#endif     /* generate charts in the PostScript graphics format.        */

#ifndef META
#define META /* Comment out this #define if you don't want the ability to  */
#endif       /* generate charts in the MS Windows metafile picture format. */

#ifndef WIRE
#define WIRE /* Comment out this #define if you don't want the ability to */
#endif       /* generate charts in the Daedalus wireframe vector format.  */

#ifndef ATLAS
#define ATLAS /* Comment out this #define if you don't want the built in  */
#endif        /* city atlas and time zone change features in the program. */

#ifndef INTERPRET
#define INTERPRET /* Comment out this #define if you don't want the ability */
#endif            /* to display interpretations of the various chart types. */

#ifndef ARABIC
#define ARABIC /* Comment out this #define if you don't want any chart     */
#endif         /* lists that include Arabic parts included in the program. */

#ifndef CONSTEL
#define CONSTEL /* Comment out this #define if you don't want any of the */
#endif          /* astronomical constellation charts in the program.     */

#ifndef BIORHYTHM
#define BIORHYTHM /* Comment out this #define if you don't want the    */
#endif            /* non-astrological biorhythm charts in the program. */

#ifndef EXPRESS
#define EXPRESS /* Comment out this #define if you don't want programmable */
#endif          /* AstroExpression customization options in the program.   */

/*
** DATA CONFIGURATION SECTION: These settings describe particulars of
** your own location and where the program looks for certain info.
*/

#ifndef PC
#define DEFAULT_DIR "~/astrolog"
#else
#define DEFAULT_DIR "C:\\Astrolog"
#endif
  // Change this string to directory path program should look in for the
  // astrolog.as default file, if one is not in the executable directory, the
  // current directory, or in directories indicated by Astrolog environment
  // variables.

#define CHART_DIR DEFAULT_DIR
  // This string is the directory the program looks in for chart info files.

#define EPHE_DIR DEFAULT_DIR
  // This string is the directory the program looks in for ephemeris files.

#define DEFAULT_LONG DMS(122, 19, 55)
#define DEFAULT_LAT  DMS(47,  36, 22)
  // Default longitude and latitude of your current location.

#define DEFAULT_ZONE 8.00
  // Default time zone (hours west of UTC).

/*
** OPTIONAL CONFIGURATION SECTION: Default parameters and related options.
*/

#define DEFAULT_SYSTEM 0
  // Default house system (Placidus = 0).

#define DEFAULT_ASPECTS 5
  // Default number of aspects to include in charts.

#define DIVISIONS 48
  // Accuracy parameter for exact aspect and transit times.

#define DEFAULT_INFOFILE "astrolog.as"
  // Name of file to look in for default program settings.

#define DEFAULT_ATLASFILE "atlas.as"
  // Name of file to look in for default atlas city list.

#define DEFAULT_TIMECHANGE "timezone.as"
  // Name of file to look in for time zone changes.

#define BITMAP_EARTH "earth.bmp"
  // Name of file to look in for bitmap of world map.

#define ENVIRONALL "ASTROLOG"
#define ENVIRONVER "ASTR"
  // Name of environment variables.

#define WHEELCOLS 15    // Affects width of each house in wheel display.
#define WHEELROWS 11    // Max no. of objects that can be in a wheel house.
#define SCREENWIDTH 80  // Number of columns to print interpretations in.
#define MONTHSPACE 3    // Number of spaces between each calendar column.
#define MAXINDAY 300    // Max number of aspects or transits displayable.
#define MAXCROSS 750    // Max number of latitude crossings displayable.
#define BIODAYS 14      // Days to include in graphic biorhythms.
#define CREDITWIDTH 74  // Number of text columns in the -Hc credit screen.
#define MAXSWITCHES 100 // Max number of switch parameters per input line.
#define PSGUTTER 9      // Points of white space on PostScript page edge.

#ifdef GRAPH            // For graphics, this char affects how bitmaps are
#ifndef PC              // written. 'N' is written like with the 'bitmap
#define BITMAPMODE 'C'  // program, 'C' is compacted somewhat (files have
#else                   // less spaces), and 'V' is compacted even more.
#define BITMAPMODE 'B'  // 'A' means write as rectangular Ascii text file.
#endif                  // 'B' means write as Windows bitmap (.bmp) file.
#endif // GRAPH

/*
** Derived compile-time settings - don't change these.
*/

//#define BETA  // Uncomment to compile in beta message on startup.
#define ASTROLOG
#ifdef _DEBUG
#define DEBUG
#endif
#ifdef SWISS
#define EPHEM
#ifdef GRAPH
#define SWISSGRAPH
#endif
#endif // SWISS
#ifdef PLACALC
#define EPHEM
#endif
#ifdef GRAPH
#ifdef CONSTEL
#define CONSTELGRAPH
#endif
#endif // GRAPH

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#ifdef PS
#define VECTOR
#endif
#ifdef META
#define VECTOR
#endif
#ifdef WIRE
#define VECTOR
#endif

#ifdef PC
#define sprintf2 snprintf
#define S(sz) (sz), (int)sizeof(sz)
#define SO(pch, sz) (pch), (sizeof(sz) - ((pch) - (sz)))
#else
#define sprintf2 sprintf
#define S(sz) (sz)
#define SO(pch, sz) (pch)
#endif

/*
** Make sure only legal combinations of options are active.
*/

#ifdef PLACALC
#ifndef MATRIX
#error "If 'PLACALC' is defined 'MATRIX' must be too"
#endif
#endif // PLACALC

#ifdef X11
#ifndef GRAPH
#error "If 'X11' is defined 'GRAPH' must be too"
#endif
#ifdef WIN
#error "If 'X11' is defined 'WIN' must not be as well"
#endif
#ifdef WCLI
#error "If 'X11' is defined 'WCLI' must not be as well"
#endif
#ifdef FLTK
#error "If 'X11' is defined 'FLTK' must not be as well"
#endif
#ifdef PC
#error "If 'X11' is defined 'PC' must not be as well"
#endif
#endif // X11

#ifdef WIN
#ifndef GRAPH
#error "If 'WIN' is defined 'GRAPH' must be too"
#endif
#ifdef X11
#error "If 'WIN' is defined 'X11' must not be as well"
#endif
#ifdef WCLI
#error "If 'WIN' is defined 'WCLI' must not be as well"
#endif
#ifdef FLTK
#error "If 'WIN' is defined 'FLTK' must not be as well"
#endif
#ifndef PC
#error "If 'WIN' is defined 'PC' must be too"
#endif
#endif // WIN

#ifdef WSETUP
#ifndef WIN
#error "If 'WSETUP' is defined 'WIN' must be too"
#endif
#endif // WSETUP

#ifdef WCLI
#ifndef GRAPH
#error "If 'WCLI' is defined 'GRAPH' must be too"
#endif
#ifdef X11
#error "If 'WCLI' is defined 'X11' must not be as well"
#endif
#ifdef WIN
#error "If 'WCLI' is defined 'WIN' must not be as well"
#endif
#ifdef FLTK
#error "If 'WCLI' is defined 'FLTK' must not be as well"
#endif
#ifndef PC
#error "If 'WCLI' is defined 'PC' must be too"
#endif
#endif // WCLI

#ifdef FLTK
#ifndef GRAPH
#error "If 'FLTK' is defined 'GRAPH' must be too"
#endif
#ifdef X11
#error "If 'FLTK' is defined 'X11' must not be as well"
#endif
#ifdef WIN
#error "If 'FLTK' is defined 'WIN' must not be as well"
#endif
#ifdef WCLI
#error "If 'FLTK' is defined 'WCLI' must not be as well"
#endif
#endif // FLTK

#ifdef PS
#ifndef GRAPH
#error "If 'PS' is defined 'GRAPH' must be too"
#endif
#endif // PS

#ifdef META
#ifndef GRAPH
#error "If 'META' is defined 'GRAPH' must be too"
#endif
#endif // META

#ifdef WIRE
#ifndef GRAPH
#error "If 'WIRE' is defined 'GRAPH' must be too"
#endif
#endif // WIRE

#endif // ASTROLOG_CONFIG_H

/* astrolog_config.h */
