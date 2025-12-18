/*
** Astrolog (Version 7.80) File: astrolog_macros.h
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
** Macro Functions.
******************************************************************************
*/

#ifndef ASTROLOG_MACROS_H
#define ASTROLOG_MACROS_H

// Byte/Word/DWord manipulation
#define BLo(w) ((byte)(w))
#define BHi(w) ((byte)((word)(w) >> 8 & 0xFF))
#define WLo(l) ((word)(dword)(l))
#define WHi(l) ((word)((dword)(l) >> 16 & 0xFFFF))
#define WFromBB(bLo, bHi) ((word)BLo(bLo) | (word)((byte)(bHi)) << 8)
#define LFromWW(wLo, wHi) ((dword)WLo(wLo) | (dword)((word)(wHi)) << 16)
#define LFromBB(b1, b2, b3, b4) LFromWW(WFromBB(b1, b2), WFromBB(b3, b4))
#define Rgb(bR, bG, bB) \
  (((dword)(bR)) | ((dword)(bG)<<8) | ((dword)(bB)<<16))
#define RgbR(l) BLo(l)
#define RgbG(l) BHi(l)
#define RgbB(l) ((byte)((dword)(l) >> 16 & 0xFF))
#define ChHex(n) (char)((n) < 10 ? '0' + (n) : 'a' + (n) - 10)

// Math utility macros
#define Max(v1, v2) ((v1) > (v2) ? (v1) : (v2))
#define Min(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#define NSgn(n) ((n) < 0 ? -1 : (n) > 0)
#define NSgn2(n) ((n) < 0 ? -1 : 1)
#define RSgn2(r) ((r) < 0.0 ? -1.0 : 1.0)
#define FOdd(n) ((n) & 1)
#define FBetween(v, v1, v2) ((v) >= (v1) && (v) <= (v2))
#define FSameR(r1, r2) (RAbs((r1) - (r2)) < rSmall)
#define FNearR(r1, r2) (RAbs((r1) - (r2)) < 0.001)

// Character case macros
#define ChCap(ch) (FUncapCh(ch) ? (ch) - 'a' + 'A' : (ch))
#define ChUncap(ch) (FCapCh(ch) ? (ch) - 'A' + 'a' : (ch))
#define FCapCh(ch) FBetween(ch, 'A', 'Z')
#define FUncapCh(ch) FBetween(ch, 'a', 'z')
#define FNumCh(ch) FBetween(ch, '0', '9')
#define NHex(ch) ((int)((ch) <= '9' ? (ch) - '0' : (ch) - 'a' + 10) & 15)
#define NHex2(ch) ((int)((ch) <= '9' ? (ch) - '0' : (ch) - 'A' + 10) & 15)

// String utility macros
#define FSzSet(sz) ((sz) != NULL && *(sz) != chNull)
#define SzSet(sz) ((sz) != NULL ? (sz) : "")

// Math operations
#define NMultDiv(n1, n2, n3) ((int)((long)(n1) * (n2) / (n3)))
#define Ratio(v1, v2, v3) ((v1) + ((v2) - (v1)) * (v3))
#define ZFromS(s) ((real)(((s)-1)*30))
#define SFromZ(r) (((int)(r))/30+1)
#define RFromD(r) ((r)/rDegRad)
#define DFromR(r) ((r)*rDegRad)
#define GFromO(o) ((rDegMax - (o))/10.0)
#define RAbs(r) fabs(r)
#define NAbs(n) abs(n)
#define RFloor(r) floor(r)
#define NFloor(r) ((int)RFloor(r))
#define RFract(r) ((r) - RFloor(r))
#define RMod(r1, r2) fmod(r1, r2)
#define Sq(n) ((n)*(n))
#define RSqr(r) sqrt(r)
#define RLog(r) log(r)
#define RLog10(r) (RLog(r) / rLog10)

// Trigonometric functions
#define RSin(r) sin(r)
#define RCos(r) cos(r)
#define RTan(r) tan(r)
#define RAsin(r) asin(r)
#define RAcos(r) acos(r)
#define RAtn(r) atan(r)
#define RSinD(r) RSin(RFromD(r))
#define RCosD(r) RCos(RFromD(r))
#define RTanD(r) RTan(RFromD(r))
#define RAsinD(r) DFromR(RAsin(r))
#define RAcosD(r) DFromR(RAcos(r))
#define RAtnD(r) DFromR(RAtn(r))
#define NSinD(nR, nD) ((int)((real)(nR)*RSinD((real)nD)))
#define NCosD(nR, nD) ((int)((real)(nR)*RCosD((real)nD)))
#define RAngleD(x, y) DFromR(RAngle(x, y))
#define VAngleD(v1, v2) DFromR(VAngle(v1, v2))
#define RLength2(x, y) RSqr(Sq(x) + Sq(y))
#define RLength3(x, y, z) RSqr(Sq(x) + Sq(y) + Sq(z))
#define RStarBright(b1, d1, d2) \
  ((d1) == 0.0 ? (b1) : ((b1) + 5.0*RLog10((d2) / (d1))))

// Object type check macros
#define FItem(obj)    FBetween(obj, 0, cObj)
#define FNorm(obj)    FBetween(obj, 0, oNorm)
#define FCusp(obj)    FBetween(obj, cuspLo, cuspHi)
#define FAngle(obj)   (FCusp(obj) && ((obj)-cuspLo)%3 == 0)
#define FMinor(obj)   (FCusp(obj) && ((obj)-cuspLo)%3 != 0)
#define FUranian(obj) FBetween(obj, uranLo, uranHi)
#define FDwarf(obj)   FBetween(obj, dwarfLo, dwarfHi)
#define FMoons(obj)   FBetween(obj, moonsLo, moonsHi)
#define FCob(obj)     FBetween(obj, cobLo, cobHi)
#define FCust(obj)    FBetween(obj, custLo, custHi)
#define FStar(obj)    FBetween(obj, starLo, starHi)
#define FObject(obj)  ((obj) <= cPlanet || (obj) >= uranLo)
#define FThing(obj)   ((obj) <= cThing || (obj) >= uranLo)
#define FThing2(obj)  (FThing(obj) || (obj) == oFor)
#define FHelio(obj)   (FNorm(obj) && FObject(obj) && !FGeo(obj))
#define FNodal(obj)   FBetween(obj, oNod, oLil)
#define FGeo(obj)     ((obj) == oMoo || FNodal(obj))
#define FHasMoon(obj) ((obj) >= 0 && (obj) != oSun)
#define FAspect(asp)  FBetween(asp, 1, cAspect)
#define FAspect2(asp) FBetween(asp, 1, cAspect2)
#define FAspect3(asp) FBetween(asp, 1, cAspect3)
#define FSector(s)    FBetween(s, 1, cSector)

// Display helpers
#define ChDashF(f)    (f ? '=' : '_')
#define SzNumF(f)     (f ? "1 " : "0 ")
#define ChDst(dst)    ((dst) == 0.0 ? 'S' : ((dst) == 1.0 ? 'D' : \
  ((dst) != dstAuto ? 'A' : (is.fDst ? 'D' : 'S'))))
#define DstReal(dst)  ((dst) != dstAuto ? (dst) : (real)is.fDst)
#define DayInYear(yea) (365-28+DayInMonth(2, yea))
#define FChSwitch(ch) ((ch) == '-' || (ch) == '/' || (ch) == '_' || \
  (ch) == '=' || (ch) == ':')

// Validation macros
#define FValidMon(mon) FBetween(mon, 1, cSign)
#define FValidDay(day, mon, yea) ((day) >= 1 && (day) <= DayInMonth(mon, yea))
#define FValidYea(yea) FBetween(yea, -nLarge, nLarge)
#define FValidTim(tim) ((tim) > -2.0 && (tim) < 24.0)
#define FValidDst(dst) FValidZon(dst)
#define FValidZon(zon) FBetween(zon, -24.0, 24.0)
#define FValidLon(lon) FBetween(lon, -rDegMax, rDegMax)
#define FValidLat(lat) FBetween(lat, -rDegQuad, rDegQuad)
#define FValidWeek(day) FBetween(day, 0, cWeek-1)
#define FValidObj(obj) FBetween(obj, 0, cObj)
#define FValidSign(n) FBetween(n, 1, cSign)
#define FValidAspect(asp) FBetween(asp, 0, cAspect)
#define FValidSystem(n) FBetween(n, 0, cSystem-1)
#define FValidMethod(n) FBetween(n, 0, cMethod)
#define FValidDivision(n) FBetween(n, 1, 2880)
#define FValidOffset(r) FBetween(r, -rDegMax, rDegMax)
#define FValidCenter(obj) (FBetween(obj, oEar, cObj) && FThing(obj))
#define FValidHarmonic(r) FBetween(r, -10000000.0, 10000000.0)
#define FValidDecan(n) FBetween(n, 0, ddMax-1)
#define FValidWheel(n) FBetween(n, 0, WHEELROWS)
#define FValidAppSep(n) FBetween(n, 0, 2)
#define FValidAstrograph(n) ((n) > 0 && (n) < 90)
#define FValidAstrograph2(n) FBetween(n, 0, 40000)
#define FValidPart(n) FBetween(n, 1, cPart)
#define FValidDwad(n) FBetween(n, 0, 12)
#define FValidBioday(n) FBetween(n, 1, 199)
#define FValidScreen(n) FBetween(n, 20, 200)
#define FValidMacro(n) ((n) >= 0)
#define FValidList(n) FBetween(n, 0, is.cci-1)
#define FValidGlyphs(n) FBetween(n, 0, 223222)
#define FValidDecaType(n) FBetween(n, 0, 5)
#define FValidDecaSize(n) FBetween(n, 0, 100)
#define FValidDecaLine(n) FBetween(n, 1, 1000)
#define FValidGrid(n) FBetween(n, 0, cObj)
#define FValidRay(n) FBetween(n, 1, cRay)
#define FValidEsoteric(n) FBetween(n, 1, 32000)
#define FValidScale(n) (FBetween(n, 100, MAXSCALE) && (n)%100 == 0)
#define FValidScaleText(n) (FBetween(n, 100, MAXSCALE) && (n)%50 == 0)
#define FValidBackPct(r) FBetween(r, 0.0, 100.0)
#define FValidBackOrient(n) FBetween(n, -1, 1)
#define FValidZoom(r) (FBetween(r, 0.0001, rDegMax) || (r) == 0.0)
#define FValidGraphX(x) (FBetween(x, BITMAPX1, BITMAPX) || (x) == 0)
#define FValidGraphY(y) (FBetween(y, BITMAPY1, BITMAPY) || (y) == 0)
#define FValidRotation(n) FBetween(n, 0, rDegMax-rSmall)
#define FValidTilt(n) FBetween(n, -rDegQuad, rDegQuad)
#define FValidColor(n) FBetween(n, 0, cColor-1)
#define FValidColor2(n) FBetween(n, 0, cColor-1 + 2)
#define FValidColorS(n) (FValidColor(n) || (n) == kStar)
#define FValidColorM(n) (FValidColor(n) || (n) == kPlanet)
#define FValidBmpmode(ch) \
  ((ch) == 'N' || (ch) == 'C' || (ch) == 'V' || (ch) == 'A' || (ch) == 'B')
#define FValidTimer(n) FBetween(n, 1, 32000)
#define FValidTelescope(obj) (FItem(obj) || (obj) == -1)

// Coordinate/degree macros
#define DM(d, m) ((d) + (m)/60.0)
#define DMS(d, m, s) (DM(d, m) + (s)/3600.0)
#define ZD(z, d) ((real)(((z)-1)*30) + (d))
#define ZDMS(z, d, m, s) ZD(z, DMS(d, m, s))
#define HM(h, m) ((h) + (m)/60.0)
#define HMS(h, m, s) (HM(h, m) + (s)/3600.0)

// Color helper macros
#define kSignA(s) kObjA[cuspLo-1+(s)]
#define kSignB(s) kObjB[cuspLo-1+(s)]
#define kModeA(m) kElemA[(m) <= 1 ? (m) : eWat]
#define kModeB(m) kElemB[(m) <= 1 ? (m) : eWat]

// Interpretation macros
#define FInterpretObj(obj) (szMindPart[obj][0] != chNull)
#define FInterpretAsp(asp) ((asp) > 0 && szInteract[asp][0])
#define szPerson  (FSzSet(ciMain.nam) ? ciMain.nam : "This person")
#define szPerson0 (FSzSet(ciMain.nam) ? ciMain.nam : "the person")
#define szPerson1 (FSzSet(ciMain.nam) ? ciMain.nam : "Person1")
#define szPerson2 (FSzSet(ciTwin.nam) ? ciTwin.nam : "Person2")

// Settings accessors
#define FIgnore(i) ignore[i]
#define FIgnore2(i) ignore2[i]
#define FAllow(i) (us.objRequire < 0 || (i) == us.objRequire)
#define FNoTimeOrSpace(ci) ((ci).mon < mJan)
#define f1K (us.fSeconds && us.fSecond1K)
#define VSeconds(i, j, k) (!us.fSeconds ? (i) : (!us.fSecond1K ? (j) : (k)))

// General utility macros
#define loop for (;;)
#define inv(v) v = !(v)
#define neg(v) v = -(v)
#define PrintL() PrintCh('\n')
#define PrintL2() PrintSz("\n\n")
#define PrintF(sz) fprintf(file, "%s", sz)
#define SwapTemp(v1, v2, vT) vT = v1; v1 = v2; v2 = vT
#define SwapN(n1, n2) do { int _swapT = (n1); (n1) = (n2); (n2) = _swapT; } while(0)
#define FSwitchF(f) ((((f) || fOr) && !fAnd) != fNot)
#define FSwitchF2(f) (((f) || (fOr || fNot)) && !fAnd)
#define SwitchF(f) f = FSwitchF(f)
#define SwitchF2(f) f = FSwitchF2(f)
#define SetCI(ci, M, D, Y, T, S, Z, O, A) \
  ci.mon = M; ci.day = D; ci.yea = Y; \
  ci.tim = T; ci.dst = S; ci.zon = Z; ci.lon = O; ci.lat = A

#ifdef GRAPH
#ifdef WINANY
#define API FAR PASCAL
#define hdcNil ((HDC)NULL)
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, NULL)
#define SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, NULL)
#define SetViewportOrg(hdc, x, y) SetViewportOrgEx(hdc, x, y, NULL)
#define SetViewportExt(hdc, x, y) SetViewportExtEx(hdc, x, y, NULL)
#define SetWindowPosition(xo, yo, xs, ys) SetWindowPos(wi.hwnd, \
  wi.fWindowFull ? GetTopWindow(NULL) : HWND_NOTOPMOST, xo, yo, xs, ys, 0)
#define MoveTo(hdc, x, y) MoveToEx(hdc, x, y, NULL)
#define HourglassOn \
  if (wi.fHourglass) hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT))
#define HourglassOff if (hcurPrev != NULL) SetCursor(hcurPrev)
#endif

// Should an object in the outer wheel be restricted?
#define FProper2(i) (!(us.nRel <= rcTransit ? ignore2[i] : ignore[i]))

// Are particular coordinates on the chart?
#define FInRect(x, y, x1, y1, x2, y2) \
  ((x) >= (x1) && (x) < (x2) && (y) >= (y1) && (y) < (y2))
#define FOnWin(X, Y) FInRect((X), (Y), 0, 0, gs.xWin, gs.yWin)

// Get a coordinate based on chart radius, a fraction, and (co)sin value.
#define POINT0(U, R, S) ((int)(RFloor((U)*(R)+rRound)*(S)))
#define POINT1(U, R, S) ((int)((RFloor((U)*(R)+rRound)+0.25)*(S)))
#define POINT2(U, R, S) ((int)((RFloor((U)*(R)+rRound)-0.25)*(S)))

// Determine (co)sine factors based on zodiac angle and chart orientation.
#define PX(A) RCosD(A)
#define PY(A) RSinD(A)
#define PZ(A) PlaceInX(A)

// Compute Mollewide projection in pixel scale given latitude.
#define RMollewide(y) RSqr((real)Sq(180*nScl) - 4.0*Sq((y)*(real)nScl))
#define NMollewide(y) \
  ((int)(RSqr((real)(Sq(180*nScl) - 4*Sq((y)*nScl))) + rRound))

// Do settings indicate the current chart should have the info sidebar?
#define fSidebar \
  ((gi.nMode == gWheel || gi.nMode == gHouse || gi.nMode == gMidpoint || \
  gi.nMode == gSector || gi.nMode == gSphere) && gs.fText && gs.fDoSidebar)

// Is the current chart most properly displayed as a square graphic?
#define fSquare \
  (gi.nMode == gWheel || gi.nMode == gHouse || gi.nMode == gGrid || \
  gi.nMode == gMidpoint || (gi.nMode == gHorizon && us.fPrimeVert) || \
  gi.nMode == gDisposit || gi.nMode == gOrbit || gi.nMode == gSector || \
  gi.nMode == gMoons || \
  gi.nMode == gSphere || gi.nMode == gGlobe || gi.nMode == gPolar)

// Does the current chart have to be displayed in a map rectangle?
#define fMap \
  (gi.nMode == gAstroGraph || gi.nMode == gWorldMap)

// Do settings indicate the current chart should have an outer border?
#define fDrawBorder \
  ((gs.fBorder || gi.nMode == gGrid) && gi.nMode != gTraTraGra && \
  gi.nMode != gTraNatGra && gi.nMode != gGlobe && gi.nMode != gPolar && \
  (gi.nMode != gWorldMap || !gs.fMollewide))

// Do settings indicate current chart should have chart info at its bottom?
#define fDrawText \
  (gs.fText && \
  (gi.nMode != gCalendar || (gs.fLabelAsp && us.nRel < rcNone)) && \
  !((gi.nMode == gWorldMap || gi.nMode == gGlobe || gi.nMode == gPolar) && \
    (gs.fAlt || gs.fConstel)) && \
  !((gi.nMode == gWheel || gi.nMode == gHouse || gi.nMode == gMidpoint || \
    gi.nMode == gSector || gi.nMode == gSphere) && gs.fDoSidebar))
#endif // GRAPH

#endif // ASTROLOG_MACROS_H

/* astrolog_macros.h */
