/*
** Astrolog (Version 7.80) File: xbackend.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** Platform Abstraction Layer - Backend Implementations
*/

#include "astrolog.h"

#ifdef GRAPH
#include "xbackend.h"

// Global backend pointer - set during initialization
GB *gpBackend = NULL;

/*
******************************************************************************
** X11 Backend Implementation
******************************************************************************
*/

#ifdef X11
static void X11SetColor(int ki)
{
  XSetForeground(gi.disp, gi.gc, rgbind[ki]);
}

static void X11DrawPixel(int x, int y)
{
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y);
}

static void X11DrawPixelThick(int x, int y)
{
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x+1, y);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y+1);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x+1, y+1);
}

static void X11DrawLine(int x1, int y1, int x2, int y2)
{
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1, x2, y2);
  // Some XDrawLine implementations don't draw the last pixel
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2);
}

static void X11DrawLineThick(int x1, int y1, int x2, int y2)
{
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1, x2, y2);
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1+1, y1, x2+1, y2);
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1+1, x2, y2+1);
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1+1, y1+1, x2+1, y2+1);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2+1, y2);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2+1);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2+1, y2+1);
}

static void X11DrawRect(int x, int y, int w, int h)
{
  XFillRectangle(gi.disp, gi.pmap, gi.gc, x, y, w, h);
}

static void X11DrawArc(int x, int y, int w, int h, double deg1, double deg2)
{
  XDrawArc(gi.disp, gi.pmap, gi.gc, x, y, w, h,
    (int)(deg1 * 64.0), (int)((deg2 - deg1) * 64.0));
}

static void X11DrawEllipse(int x, int y, int w, int h)
{
  XFillArc(gi.disp, gi.pmap, gi.gc, x, y, w, h, 0, 360*64);
}

static void X11ClearScreen(int ki)
{
  XSetForeground(gi.disp, gi.gc, rgbind[ki]);
  XFillRectangle(gi.disp, gi.pmap, gi.gc, 0, 0, gs.xWin, gs.yWin);
}

static void X11Flush(void)
{
  XSync(gi.disp, 0);
}

static GB gbX11 = {
  "X11",
  X11SetColor,
  X11DrawPixel,
  X11DrawPixelThick,
  X11DrawLine,
  X11DrawLineThick,
  X11DrawRect,
  X11DrawArc,
  X11DrawEllipse,
  X11ClearScreen,
  X11Flush,
  NULL
};

void InitBackendX11(void)
{
  gpBackend = &gbX11;
}
#endif // X11

/*
******************************************************************************
** Windows Backend Implementation
******************************************************************************
*/

#ifdef WINANY
static void WinSetColor(int ki)
{
  HPEN hpenT;
  if (gi.kiCur != ki) {
    hpenT = wi.hpen;
    wi.hpen = CreatePen(PS_SOLID, gi.nScaleT
#ifdef WIN
      * (1 + (gs.fThick && wi.hdcPrint != NULL))
#endif
      , (COLORREF)rgbbmp[ki]);
    SelectObject(wi.hdc, wi.hpen);
    if (hpenT != (HPEN)NULL)
      DeleteObject(hpenT);
    gi.kiCur = ki;
  }
}

static void WinDrawPixel(int x, int y)
{
#ifdef WIN
  if (wi.hdcPrint == hdcNil) {
#endif
    SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
#ifdef WIN
  } else {
    MoveTo(wi.hdc, x, y);
    LineTo(wi.hdc, x+1, y);
  }
#endif
}

static void WinDrawPixelThick(int x, int y)
{
#ifdef WIN
  if (wi.hdcPrint == hdcNil) {
#endif
    SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
    SetPixel(wi.hdc, x+1, y, (COLORREF)rgbbmp[gi.kiCur]);
    SetPixel(wi.hdc, x, y+1, (COLORREF)rgbbmp[gi.kiCur]);
    SetPixel(wi.hdc, x+1, y+1, (COLORREF)rgbbmp[gi.kiCur]);
#ifdef WIN
  } else {
    MoveTo(wi.hdc, x, y);
    LineTo(wi.hdc, x+1, y);
  }
#endif
}

static void WinDrawLine(int x1, int y1, int x2, int y2)
{
  MoveTo(wi.hdc, x1, y1);
  LineTo(wi.hdc, x2, y2);
#ifdef WIN
  if (wi.hdcPrint != NULL)
    return;
#endif
  // For Windows lines, have to manually draw the last pixel
  SetPixel(wi.hdc, x2, y2, (COLORREF)rgbbmp[gi.kiCur]);
}

static void WinDrawLineThick(int x1, int y1, int x2, int y2)
{
  MoveTo(wi.hdc, x1, y1);
  LineTo(wi.hdc, x2, y2);
#ifdef WIN
  if (wi.hdcPrint != NULL)
    return;
#endif
  // Make the line thicker by drawing it four times
  LineTo(wi.hdc, x2+1, y2);
  LineTo(wi.hdc, x1+1, y1);
  LineTo(wi.hdc, x1, y1+1);
  LineTo(wi.hdc, x2, y2+1);
  LineTo(wi.hdc, x2+1, y2+1);
  LineTo(wi.hdc, x1+1, y1+1);
  LineTo(wi.hdc, x1, y1);
}

static void WinDrawRect(int x, int y, int w, int h)
{
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
  SelectObject(wi.hdc, wi.hbrush);
  PatBlt(wi.hdc, x, y, w + gi.nScaleT, h + gi.nScaleT, PATCOPY);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}

static void WinDrawArc(int x, int y, int w, int h, double deg1, double deg2)
{
  // Windows Arc() uses start/end points, not angles
  int xc = x + w/2, yc = y + h/2;
  int r = Max(w, h) / 2;
  int xs = xc + (int)(r * RCosD(deg1));
  int ys = yc - (int)(r * RSinD(deg1));
  int xe = xc + (int)(r * RCosD(deg2));
  int ye = yc - (int)(r * RSinD(deg2));
  Arc(wi.hdc, x, y, x+w, y+h, xs, ys, xe, ye);
}

static void WinDrawEllipse(int x, int y, int w, int h)
{
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
  SelectObject(wi.hdc, wi.hbrush);
  Ellipse(wi.hdc, x, y, x+w, y+h);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}

static void WinClearScreenImpl(int ki)
{
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[ki]);
  SelectObject(wi.hdc, wi.hbrush);
  PatBlt(wi.hdc, -gi.xOffset, -gi.yOffset, wi.xClient, wi.yClient, PATCOPY);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}

static void WinFlush(void)
{
  // Windows doesn't need explicit flush for GDI operations
}

static GB gbWin = {
  "Windows",
  WinSetColor,
  WinDrawPixel,
  WinDrawPixelThick,
  WinDrawLine,
  WinDrawLineThick,
  WinDrawRect,
  WinDrawArc,
  WinDrawEllipse,
  WinClearScreenImpl,
  WinFlush,
  NULL
};

void InitBackendWin(void)
{
  gpBackend = &gbWin;
}
#endif // WINANY

/*
******************************************************************************
** FLTK Backend Implementation
******************************************************************************
*/

#ifdef FLTK
static void FltkSetColor(int ki)
{
  fl_color(FltkColorFromKI(ki));
}

static void FltkDrawPixel(int x, int y)
{
  fl_point(x, y);
}

static void FltkDrawPixelThick(int x, int y)
{
  fl_point(x, y);
  fl_point(x+1, y);
  fl_point(x, y+1);
  fl_point(x+1, y+1);
}

static void FltkDrawLine(int x1, int y1, int x2, int y2)
{
  fl_line(x1, y1, x2, y2);
}

static void FltkDrawLineThick(int x1, int y1, int x2, int y2)
{
  fl_line(x1, y1, x2, y2);
  fl_line(x1+1, y1, x2+1, y2);
  fl_line(x1, y1+1, x2, y2+1);
  fl_line(x1+1, y1+1, x2+1, y2+1);
}

static void FltkDrawRect(int x, int y, int w, int h)
{
  fl_rectf(x, y, w, h);
}

static void FltkDrawArc(int x, int y, int w, int h, double deg1, double deg2)
{
  fl_arc(x, y, w, h, deg1, deg2);
}

static void FltkDrawEllipse(int x, int y, int w, int h)
{
  fl_pie(x, y, w, h, 0.0, 360.0);
}

static void FltkClearScreen(int ki)
{
  fl_color(FltkColorFromKI(ki));
  fl_rectf(0, 0, gs.xWin, gs.yWin);
}

static void FltkFlush(void)
{
  Fl::flush();
}

static GB gbFltk = {
  "FLTK",
  FltkSetColor,
  FltkDrawPixel,
  FltkDrawPixelThick,
  FltkDrawLine,
  FltkDrawLineThick,
  FltkDrawRect,
  FltkDrawArc,
  FltkDrawEllipse,
  FltkClearScreen,
  FltkFlush,
  NULL
};

void InitBackendFltk(void)
{
  gpBackend = &gbFltk;
}
#endif // FLTK

/*
******************************************************************************
** Backend Selection
******************************************************************************
*/

// Initialize the appropriate backend based on compile-time configuration
void InitGraphicsBackend(void)
{
#ifdef FLTK
  InitBackendFltk();
#elif defined(X11)
  InitBackendX11();
#elif defined(WINANY)
  InitBackendWin();
#else
  gpBackend = NULL;
#endif
}

#endif // GRAPH
