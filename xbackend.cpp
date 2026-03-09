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

// Include macOS frameworks before astrolog.h to avoid macro conflicts
// (astrolog defines 'space' as a macro which conflicts with macOS SDK headers)
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#endif

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
static void X11SetColor(int ki) { XSetForeground(gi.disp, gi.gc, rgbind[ki]); }

static void X11DrawPixel(int x, int y) {
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y);
}

static void X11DrawPixelThick(int x, int y) {
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x + 1, y);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y + 1);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x + 1, y + 1);
}

static void X11DrawLine(int x1, int y1, int x2, int y2) {
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1, x2, y2);
  // Some XDrawLine implementations don't draw the last pixel
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2);
}

static void X11DrawLineThick(int x1, int y1, int x2, int y2) {
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1, x2, y2);
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1 + 1, y1, x2 + 1, y2);
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1 + 1, x2, y2 + 1);
  XDrawLine(gi.disp, gi.pmap, gi.gc, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2 + 1, y2);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2 + 1);
  XDrawPoint(gi.disp, gi.pmap, gi.gc, x2 + 1, y2 + 1);
}

static void X11DrawRect(int x, int y, int w, int h) {
  XFillRectangle(gi.disp, gi.pmap, gi.gc, x, y, w, h);
}

static void X11DrawArc(int x, int y, int w, int h, double deg1, double deg2) {
  XDrawArc(gi.disp, gi.pmap, gi.gc, x, y, w, h, (int)(deg1 * 64.0),
           (int)((deg2 - deg1) * 64.0));
}

static void X11DrawEllipse(int x, int y, int w, int h) {
  XFillArc(gi.disp, gi.pmap, gi.gc, x, y, w, h, 0, 360 * 64);
}

static void X11ClearScreen(int ki) {
  XSetForeground(gi.disp, gi.gc, rgbind[ki]);
  XFillRectangle(gi.disp, gi.pmap, gi.gc, 0, 0, gs.xWin, gs.yWin);
}

static void X11Flush(void) { XSync(gi.disp, 0); }

static GB gbX11 = {"X11",
                   X11SetColor,
                   NULL, // PutColorAlpha
                   NULL, // PutColorAlphaKV
                   NULL, // PutLineWidth
                   X11DrawPixel,
                   X11DrawPixelThick,
                   X11DrawLine,
                   NULL, // PutLineF
                   X11DrawLineThick,
                   X11DrawRect,
                   X11DrawArc,
                   X11DrawEllipse,
                   NULL, // PutSector
                   NULL, // PutGlyph
                   NULL, // PutText
                   X11ClearScreen,
                   X11Flush,
                   NULL};

void InitBackendX11(void) { gpBackend = &gbX11; }
#endif // X11

/*
******************************************************************************
** Windows Backend Implementation
******************************************************************************
*/

#ifdef WINANY
static void WinSetColor(int ki) {
  HPEN hpenT;
  if (gi.kiCur != ki) {
    hpenT = wi.hpen;
    wi.hpen = CreatePen(PS_SOLID,
                        gi.nScaleT
#ifdef WIN
                            * (1 + (gs.fThick && wi.hdcPrint != NULL))
#endif
                            ,
                        (COLORREF)rgbbmp[ki]);
    SelectObject(wi.hdc, wi.hpen);
    if (hpenT != (HPEN)NULL)
      DeleteObject(hpenT);
    gi.kiCur = ki;
  }
}

static void WinDrawPixel(int x, int y) {
#ifdef WIN
  if (wi.hdcPrint == hdcNil) {
#endif
    SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
#ifdef WIN
  } else {
    MoveTo(wi.hdc, x, y);
    LineTo(wi.hdc, x + 1, y);
  }
#endif
}

static void WinDrawPixelThick(int x, int y) {
#ifdef WIN
  if (wi.hdcPrint == hdcNil) {
#endif
    SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
    SetPixel(wi.hdc, x + 1, y, (COLORREF)rgbbmp[gi.kiCur]);
    SetPixel(wi.hdc, x, y + 1, (COLORREF)rgbbmp[gi.kiCur]);
    SetPixel(wi.hdc, x + 1, y + 1, (COLORREF)rgbbmp[gi.kiCur]);
#ifdef WIN
  } else {
    MoveTo(wi.hdc, x, y);
    LineTo(wi.hdc, x + 1, y);
  }
#endif
}

static void WinDrawLine(int x1, int y1, int x2, int y2) {
  MoveTo(wi.hdc, x1, y1);
  LineTo(wi.hdc, x2, y2);
#ifdef WIN
  if (wi.hdcPrint != NULL)
    return;
#endif
  // For Windows lines, have to manually draw the last pixel
  SetPixel(wi.hdc, x2, y2, (COLORREF)rgbbmp[gi.kiCur]);
}

static void WinDrawLineThick(int x1, int y1, int x2, int y2) {
  MoveTo(wi.hdc, x1, y1);
  LineTo(wi.hdc, x2, y2);
#ifdef WIN
  if (wi.hdcPrint != NULL)
    return;
#endif
  // Make the line thicker by drawing it four times
  LineTo(wi.hdc, x2 + 1, y2);
  LineTo(wi.hdc, x1 + 1, y1);
  LineTo(wi.hdc, x1, y1 + 1);
  LineTo(wi.hdc, x2, y2 + 1);
  LineTo(wi.hdc, x2 + 1, y2 + 1);
  LineTo(wi.hdc, x1 + 1, y1 + 1);
  LineTo(wi.hdc, x1, y1);
}

static void WinDrawRect(int x, int y, int w, int h) {
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
  SelectObject(wi.hdc, wi.hbrush);
  PatBlt(wi.hdc, x, y, w + gi.nScaleT, h + gi.nScaleT, PATCOPY);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}

static void WinDrawArc(int x, int y, int w, int h, double deg1, double deg2) {
  // Windows Arc() uses start/end points, not angles
  int xc = x + w / 2, yc = y + h / 2;
  int r = Max(w, h) / 2;
  int xs = xc + (int)(r * RCosD(deg1));
  int ys = yc - (int)(r * RSinD(deg1));
  int xe = xc + (int)(r * RCosD(deg2));
  int ye = yc - (int)(r * RSinD(deg2));
  Arc(wi.hdc, x, y, x + w, y + h, xs, ys, xe, ye);
}

static void WinDrawEllipse(int x, int y, int w, int h) {
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
  SelectObject(wi.hdc, wi.hbrush);
  Ellipse(wi.hdc, x, y, x + w, y + h);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}

static void WinDrawSector(int cx, int cy, double ux, double uy, double r1,
                          double r2, double deg1, double deg2) {
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
  SelectObject(wi.hdc, wi.hbrush);
  Pie(wi.hdc, cx - (int)(ux * r2), cy - (int)(uy * r2), cx + (int)(ux * r2),
      cy + (int)(uy * r2), cx + (int)(ux * r2 * RCosD(deg1)),
      cy - (int)(uy * r2 * RSinD(deg1)), cx + (int)(ux * r2 * RCosD(deg2)),
      cy - (int)(uy * r2 * RSinD(deg2)));
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
  if (r1 > 0) {
    wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiOff]);
    SelectObject(wi.hdc, wi.hbrush);
    Pie(wi.hdc, x - r1, y - r1, x + r1, y + r1, x + (int)(r1 * RCosD(deg1)),
        y - (int)(r1 * RSinD(deg1)), x + (int)(r1 * RCosD(deg2)),
        y - (int)(r1 * RSinD(deg2)));
    SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
    DeleteObject(wi.hbrush);
  }
}

static void WinClearScreenImpl(int ki) {
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[ki]);
  SelectObject(wi.hdc, wi.hbrush);
  PatBlt(wi.hdc, -gi.xOffset, -gi.yOffset, wi.xClient, wi.yClient, PATCOPY);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}

static void WinFlush(void) {
  // Windows doesn't need explicit flush for GDI operations
}

static GB gbWin = {"Windows",
                   WinSetColor,
                   NULL, // PutColorAlpha
                   NULL, // PutColorAlphaKV
                   NULL, // PutLineWidth
                   WinDrawPixel,
                   WinDrawPixelThick,
                   WinDrawLine,
                   NULL, // PutLineF
                   WinDrawLineThick,
                   WinDrawRect,
                   WinDrawArc,
                   WinDrawEllipse,
                   WinDrawSector,
                   NULL, // PutGlyph
                   NULL, // PutText
                   WinClearScreenImpl,
                   WinFlush,
                   NULL};

void InitBackendWin(void) { gpBackend = &gbWin; }
#endif // WINANY

/*
******************************************************************************
** FLTK Backend Implementation
******************************************************************************
*/

#ifdef FLTK

// Astrology font indices for custom FLTK fonts (start after built-in fonts)
// Initialize to -1 (unavailable) - will be set when fonts are loaded
static Fl_Font rgFltkAstroFont[cFont] = {
    (Fl_Font)-1, (Fl_Font)-1, (Fl_Font)-1, (Fl_Font)-1, (Fl_Font)-1,
    (Fl_Font)-1, (Fl_Font)-1, (Fl_Font)-1, (Fl_Font)-1, (Fl_Font)-1};
static flag fAstroFontsLoaded = fFalse;

#ifdef __APPLE__
// Load bundled fonts from the app bundle on macOS
static void LoadBundledFonts(void) {
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  if (!mainBundle)
    return;
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  if (!resourcesURL)
    return;

  static const struct {
    const char *filename;
    int fontIndex;
    const char *fontName;
  } fontFiles[] = {
      {"Fonts/Astro.ttf", fiAstro, "Astro"},
      {"Fonts/EnigmaAstrology.ttf", fiEnigma, "EnigmaAstrology"},
      {"Fonts/HamburgSymbols.ttf", fiHamburg, "HamburgSymbols"},
      {"Fonts/Astronomicon.ttf", fiAstronom, "Astronomicon"},
      {"Fonts/HanksNakshatra.ttf", fiNakshatr, "HanksNakshatra"},
  };

  for (int i = 0; i < (int)(sizeof(fontFiles) / sizeof(fontFiles[0])); i++) {
    CFStringRef fontFileName = CFStringCreateWithCString(
        NULL, fontFiles[i].filename, kCFStringEncodingUTF8);
    if (!fontFileName)
      continue;
    CFURLRef fontURL = CFURLCreateCopyAppendingPathComponent(
        NULL, resourcesURL, fontFileName, false);
    CFRelease(fontFileName);
    if (!fontURL)
      continue;

    CFErrorRef error = NULL;
    if (CTFontManagerRegisterFontsForURL(fontURL, kCTFontManagerScopeProcess,
                                         &error)) {
      Fl_Font fltkFont = FL_FREE_FONT + fontFiles[i].fontIndex;
      Fl::set_font(fltkFont, fontFiles[i].fontName);
      rgFltkAstroFont[fontFiles[i].fontIndex] = fltkFont;
    } else {
      if (error)
        CFRelease(error);
      rgFltkAstroFont[fontFiles[i].fontIndex] = (Fl_Font)-1;
    }
    CFRelease(fontURL);
  }
  CFRelease(resourcesURL);
  fAstroFontsLoaded = fTrue;
}
#endif // __APPLE__

static void FltkSetColor(int ki) { fl_color(FltkColorFromKI(ki)); }

static void FltkSetColorAlpha(int ki, int alpha) {
  if (alpha >= 255) {
    fl_color(FltkColorFromKI(ki));
    return;
  }
  KV kvFg = rgbbmp[ki];
  KV kvBg = rgbbmp[gi.kiOff];
  unsigned char r =
      (unsigned char)((RgbR(kvFg) * alpha + RgbR(kvBg) * (255 - alpha)) / 255);
  unsigned char g =
      (unsigned char)((RgbG(kvFg) * alpha + RgbG(kvBg) * (255 - alpha)) / 255);
  unsigned char b =
      (unsigned char)((RgbB(kvFg) * alpha + RgbB(kvBg) * (255 - alpha)) / 255);
  fl_color(fl_rgb_color(r, g, b));
}

static void FltkSetColorAlphaKV(KV kv, int alpha) {
  KV kvBg = rgbbmp[gi.kiOff];
  unsigned char r =
      (unsigned char)((RgbR(kv) * alpha + RgbR(kvBg) * (255 - alpha)) / 255);
  unsigned char g =
      (unsigned char)((RgbG(kv) * alpha + RgbG(kvBg) * (255 - alpha)) / 255);
  unsigned char b =
      (unsigned char)((RgbB(kv) * alpha + RgbB(kvBg) * (255 - alpha)) / 255);
  fl_color(fl_rgb_color(r, g, b));
}

static void FltkSetLineWidth(double width) {
  int w = (int)(width + 0.5);
  if (w < 1)
    w = 1;
  fl_line_style(FL_SOLID, w == 1 ? 0 : w);
}

static void FltkDrawPixel(int x, int y) { fl_point(x, y); }

static void FltkDrawPixelThick(int x, int y) {
  fl_point(x, y);
  fl_point(x + 1, y);
  fl_point(x, y + 1);
  fl_point(x + 1, y + 1);
}

static void FltkDrawLine(int x1, int y1, int x2, int y2) {
  fl_line(x1, y1, x2, y2);
}

static void FltkDrawLineF(double x1, double y1, double x2, double y2) {
  fl_line((int)(x1 + 0.5), (int)(y1 + 0.5), (int)(x2 + 0.5), (int)(y2 + 0.5));
}

static void FltkDrawLineThick(int x1, int y1, int x2, int y2) {
  fl_line(x1, y1, x2, y2);
  fl_line(x1 + 1, y1, x2 + 1, y2);
  fl_line(x1, y1 + 1, x2, y2 + 1);
  fl_line(x1 + 1, y1 + 1, x2 + 1, y2 + 1);
}

static void FltkDrawRect(int x, int y, int w, int h) { fl_rectf(x, y, w, h); }

static void FltkDrawArc(int x, int y, int w, int h, double deg1, double deg2) {
  fl_arc(x, y, w, h, deg1, deg2);
}

static void FltkDrawEllipse(int x, int y, int w, int h) {
  fl_pie(x, y, w, h, 0.0, 360.0);
}

static void FltkDrawSector(int cx, int cy, double ux, double uy, double r1,
                           double r2, double deg1, double deg2) {
  fl_pie((int)(cx - ux * r2 + 0.5), (int)(cy - uy * r2 + 0.5),
         (int)(ux * r2 * 2 + 0.5), (int)(uy * r2 * 2 + 0.5), deg1, deg2);
  if (r1 > 0) {
    Fl_Color clr = fl_color();
    fl_color(FltkColorFromKI(gi.kiOff));
    fl_pie((int)(cx - ux * r1 + 0.5), (int)(cy - uy * r1 + 0.5),
           (int)(ux * r1 * 2 + 0.5), (int)(uy * r1 * 2 + 0.5), deg1, deg2);
    fl_color(clr);
  }
}

static Fl_Font FltkFontFromFI(int fi) {
  static flag fMonoFontSet = fFalse;
  static Fl_Font fltkMonoFont = FL_COURIER;
  if (!fMonoFontSet) {
#ifdef __APPLE__
    Fl::set_font(FL_FREE_FONT, "Menlo");
    fltkMonoFont = FL_FREE_FONT;
#endif
    fMonoFontSet = fTrue;
  }
  switch (fi) {
  case fiCourier:
  case fiConsolas:
    return fltkMonoFont;
  case fiArial:
    return FL_HELVETICA;
  case fiAstro:
  case fiEnigma:
  case fiHamburg:
  case fiAstronom:
  case fiNakshatr:
    if (fAstroFontsLoaded && rgFltkAstroFont[fi] != (Fl_Font)-1)
      return rgFltkAstroFont[fi];
    break;
  }
  return (Fl_Font)-1;
}

static int FltkPutGlyph(int ch, int x, int y, int nFont, int nScale) {
  fl_line_style(0);
  Fl_Font font = FltkFontFromFI(nFont);
  if (font == (Fl_Font)-1)
    return 0;
  int baseSize = (nFont >= fiAstro && nFont <= fiNakshatr) ? 16 : 12;
  int fontSize = baseSize * gs.nScale * nScale / 10000;
  if (fontSize < 6)
    fontSize = 6;
  fl_font(font, fontSize);
  char sz[8];
  sz[0] = (char)ch;
  sz[1] = '\0';
  int w, h;
  fl_measure(sz, w, h, 0);
  fl_draw(sz, x - w / 2, y + h / 2 - fl_descent());
  return 1;
}

static int FltkPutText(const char *sz, int x, int y, int nFont, int nScale) {
  fl_line_style(0);
  Fl_Font font = FltkFontFromFI(nFont);
  if (font == (Fl_Font)-1)
    return 0;
  int fontSize = 6 * nScale;
  if (fontSize < 6)
    fontSize = 6;
  fl_font(font, fontSize);
  int w, h;
  fl_measure(sz, w, h, 0);
  fl_draw(sz, x - w / 2, y + h / 2 - fl_descent());
  return 1;
}

static void FltkClearScreen(int ki) {
  fl_color(FltkColorFromKI(ki));
  fl_rectf(0, 0, gs.xWin, gs.yWin);
}

static void FltkFlush(void) { Fl::flush(); }

static GB gbFltk = {"FLTK",
                    FltkSetColor,
                    FltkSetColorAlpha,
                    FltkSetColorAlphaKV,
                    FltkSetLineWidth,
                    FltkDrawPixel,
                    FltkDrawPixelThick,
                    FltkDrawLine,
                    FltkDrawLineF,
                    FltkDrawLineThick,
                    FltkDrawRect,
                    FltkDrawArc,
                    FltkDrawEllipse,
                    FltkDrawSector,
                    FltkPutGlyph,
                    FltkPutText,
                    FltkClearScreen,
                    FltkFlush,
                    NULL};

void FltkInitMapGlyphScale(void) { gi.nGlyphScale = 100; }

void FltkAdjustMapGlyphScale(int delta) {
  gi.nGlyphScale += delta;
  if (gi.nGlyphScale < 50)
    gi.nGlyphScale = 50;
  if (gi.nGlyphScale > 400)
    gi.nGlyphScale = 400;
}

void InitBackendFltk(void) {
#ifdef __APPLE__
  LoadBundledFonts();
#endif
  FltkInitMapGlyphScale();
  gpBackend = &gbFltk;
}
#endif // FLTK

/*
******************************************************************************
** Cairo Backend Implementation
******************************************************************************
*/

#ifdef CAIRO
#include <cairo.h>
static cairo_t *gi_cr = NULL;
static cairo_surface_t *gi_surface = NULL;
static GB *gpBackendPrev = NULL;

static void CairoSetColor(int ki) {
  if (ki < 0 || ki >= cColor)
    ki = 0;
  KV kv = rgbbmp[ki];
  cairo_set_source_rgb(gi_cr, (double)RgbR(kv) / 255.0,
                       (double)RgbG(kv) / 255.0, (double)RgbB(kv) / 255.0);
}

static void CairoSetColorAlphaKV(KV kv, int alpha) {
  cairo_set_source_rgba(gi_cr, (double)RgbR(kv) / 255.0,
                        (double)RgbG(kv) / 255.0, (double)RgbB(kv) / 255.0,
                        (double)alpha / 255.0);
}

static void CairoSetColorAlpha(int ki, int alpha) {
  if (ki < 0 || ki >= cColor)
    ki = 0;
  CairoSetColorAlphaKV(rgbbmp[ki], alpha);
}

static void CairoSetLineWidth(double width) {
  cairo_set_line_width(gi_cr, width);
}
static void CairoDrawPixel(int x, int y) {
  cairo_rectangle(gi_cr, x, y, 1, 1);
  cairo_fill(gi_cr);
}
static void CairoDrawPixelThick(int x, int y) {
  cairo_rectangle(gi_cr, x, y, 2, 2);
  cairo_fill(gi_cr);
}
static void CairoDrawLine(int x1, int y1, int x2, int y2) {
  cairo_move_to(gi_cr, (double)x1, (double)y1);
  cairo_line_to(gi_cr, (double)x2, (double)y2);
  cairo_stroke(gi_cr);
}
static void CairoDrawLineF(double x1, double y1, double x2, double y2) {
  cairo_move_to(gi_cr, x1, y1);
  cairo_line_to(gi_cr, x2, y2);
  cairo_stroke(gi_cr);
}
static void CairoDrawLineThick(int x1, int y1, int x2, int y2) {
  double oldWidth = cairo_get_line_width(gi_cr);
  cairo_set_line_width(gi_cr, 2.0);
  cairo_move_to(gi_cr, (double)x1, (double)y1);
  cairo_line_to(gi_cr, (double)x2, (double)y2);
  cairo_stroke(gi_cr);
  cairo_set_line_width(gi_cr, oldWidth);
}
static void CairoDrawRect(int x, int y, int w, int h) {
  cairo_rectangle(gi_cr, x, y, w, h);
  cairo_fill(gi_cr);
}

static void CairoDrawArc(int x, int y, int w, int h, double deg1, double deg2) {
  double cx = x + w / 2.0, cy = y + h / 2.0, rx = w / 2.0, ry = h / 2.0;
  cairo_save(gi_cr);
  cairo_translate(gi_cr, cx, cy);
  cairo_scale(gi_cr, rx, ry);
  cairo_arc(gi_cr, 0, 0, 1.0, -deg2 * rPi / 180.0, -deg1 * rPi / 180.0);
  cairo_restore(gi_cr);
  cairo_stroke(gi_cr);
}

static void CairoDrawEllipse(int x, int y, int w, int h) {
  double cx = x + w / 2.0, cy = y + h / 2.0, rx = w / 2.0, ry = h / 2.0;
  cairo_save(gi_cr);
  cairo_translate(gi_cr, cx, cy);
  cairo_scale(gi_cr, rx, ry);
  cairo_arc(gi_cr, 0, 0, 1.0, 0, 2 * rPi);
  cairo_restore(gi_cr);
  cairo_fill(gi_cr);
}

static void CairoDrawSector(int cx, int cy, double ux, double uy, double r1,
                            double r2, double deg1, double deg2) {
  // Astrolog degrees: 0=East, 90=South (Y-down screen). Cairo: 0=East,
  // positive=CCW. Screen Y is inverted, so we use arc_negative for visual CCW
  // (which is CW in Cairo).
  double a1 = deg1 * rPi / 180.0, a2 = deg2 * rPi / 180.0;
  cairo_save(gi_cr);
  cairo_translate(gi_cr, (double)cx, (double)cy);
  cairo_scale(gi_cr, ux, uy);
  cairo_new_path(gi_cr);
  cairo_arc_negative(gi_cr, 0.0, 0.0, r2, a1, a2);
  if (r1 > 0) {
    cairo_line_to(gi_cr, r1 * cos(a2), r1 * sin(a2));
    cairo_arc(gi_cr, 0.0, 0.0, r1, a2, a1);
  } else
    cairo_line_to(gi_cr, 0.0, 0.0);
  cairo_close_path(gi_cr);
  cairo_fill(gi_cr);
  cairo_restore(gi_cr);
}

static void CairoClearScreen(int ki) {
  CairoSetColor(ki);
  cairo_paint(gi_cr);
}
static void CairoFlush(void) { cairo_surface_flush(gi_surface); }

static const char *CairoFontName(int fi) {
  switch (fi) {
  case fiAstro:
    return "Astro";
  case fiEnigma:
    return "EnigmaAstrology";
  case fiHamburg:
    return "HamburgSymbols";
  case fiAstronom:
    return "Astronomicon";
  case fiNakshatr:
    return "HanksNakshatra";
#ifdef __APPLE__
  case fiCourier:
  case fiConsolas:
    return "Menlo";
#else
  case fiCourier:
    return "Courier New";
  case fiConsolas:
    return "Consolas";
#endif
  case fiArial:
    return "Arial";
  }
  return NULL;
}

static int CairoPutGlyph(int ch, int x, int y, int nFont, int nScale) {
  const char *fontName = CairoFontName(nFont);
  if (!fontName)
    return 0;
  int baseSize = (nFont >= fiAstro && nFont <= fiNakshatr) ? 16 : 12;
  double fontSize = (double)(baseSize * gs.nScale * nScale) / 10000.0;
  if (fontSize < 6.0)
    fontSize = 6.0;
  cairo_select_font_face(gi_cr, fontName, CAIRO_FONT_SLANT_NORMAL,
                         gs.fThick ? CAIRO_FONT_WEIGHT_BOLD
                                   : CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(gi_cr, fontSize);
  char sz[8];
  if (ch < 0x80) {
    sz[0] = (char)ch;
    sz[1] = '\0';
  } else if (ch < 0x100) {
    sz[0] = (char)(0xC0 | ((ch >> 6) & 0x1F));
    sz[1] = (char)(0x80 | (ch & 0x3F));
    sz[2] = '\0';
  } else
    return 0;
  cairo_text_extents_t ext;
  cairo_text_extents(gi_cr, sz, &ext);
  if (ext.width < 0.5)
    return 0;
  cairo_move_to(gi_cr, x - (ext.width / 2.0 + ext.x_bearing),
                y + (ext.height / 2.0));
  cairo_show_text(gi_cr, sz);
  return 1;
}

static int CairoPutText(const char *sz, int x, int y, int nFont, int nScale) {
  const char *fontName = CairoFontName(nFont);
  if (!fontName)
    return 0;
  double fontSize = 5.0 * nScale;
  if (fontSize < 8.0)
    fontSize = 8.0;
  cairo_select_font_face(gi_cr, fontName, CAIRO_FONT_SLANT_NORMAL,
                         gs.fThick ? CAIRO_FONT_WEIGHT_BOLD
                                   : CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(gi_cr, fontSize);
  cairo_text_extents_t ext;
  cairo_text_extents(gi_cr, sz, &ext);
  cairo_move_to(gi_cr, x - ext.x_bearing, y - ext.y_bearing - ext.height / 2.0);
  cairo_show_text(gi_cr, sz);
  return 1;
}

static GB gbCairo = {"Cairo",
                     CairoSetColor,
                     CairoSetColorAlpha,
                     CairoSetColorAlphaKV,
                     CairoSetLineWidth,
                     CairoDrawPixel,
                     CairoDrawPixelThick,
                     CairoDrawLine,
                     CairoDrawLineF,
                     CairoDrawLineThick,
                     CairoDrawRect,
                     CairoDrawArc,
                     CairoDrawEllipse,
                     CairoDrawSector,
                     CairoPutGlyph,
                     CairoPutText,
                     CairoClearScreen,
                     CairoFlush,
                     NULL};

void InitBackendCairo(cairo_surface_t *surface) {
  gpBackendPrev = gpBackend;
  gi_surface = surface;
  gi_cr = cairo_create(gi_surface);
  cairo_set_line_width(gi_cr, 1.0);
  cairo_set_line_cap(gi_cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(gi_cr, CAIRO_LINE_JOIN_ROUND);
  cairo_set_antialias(gi_cr, CAIRO_ANTIALIAS_BEST);
  gpBackend = &gbCairo;
}

void EndBackendCairo(void) {
  if (gi_cr) {
    cairo_destroy(gi_cr);
    gi_cr = NULL;
  }
  gi_surface = NULL;
  gpBackend = gpBackendPrev;
  gpBackendPrev = NULL;
}

cairo_t *CairoContext(void) { return gi_cr; }
#endif // CAIRO

void InitGraphicsBackend(void) {
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
