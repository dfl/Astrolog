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
                   NULL, // PutColorAlpha - X11 doesn't support alpha
                   X11DrawPixel,
                   X11DrawPixelThick,
                   X11DrawLine,
                   X11DrawLineThick,
                   X11DrawRect,
                   X11DrawArc,
                   X11DrawEllipse,
                   NULL, // PutGlyph - X11 uses vector fallback
                   NULL, // PutText - X11 uses vector fallback
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
                   NULL, // PutColorAlpha - Windows doesn't support alpha
                   WinDrawPixel,
                   WinDrawPixelThick,
                   WinDrawLine,
                   WinDrawLineThick,
                   WinDrawRect,
                   WinDrawArc,
                   WinDrawEllipse,
                   NULL, // PutGlyph - Windows uses DrawGlyph directly for now
                   NULL, // PutText - Windows uses direct GDI calls for now
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
  printf("LoadBundledFonts: starting...\n");
  fflush(stdout);

  CFBundleRef mainBundle = CFBundleGetMainBundle();
  if (!mainBundle) {
    printf("LoadBundledFonts: no main bundle found\n");
    return;
  }

  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  if (!resourcesURL) {
    printf("LoadBundledFonts: no resources directory found\n");
    return;
  }

  // Debug: print resources path
  char resourcesPath[1024];
  if (CFURLGetFileSystemRepresentation(
          resourcesURL, true, (UInt8 *)resourcesPath, sizeof(resourcesPath))) {
    printf("LoadBundledFonts: resources path = %s\n", resourcesPath);
  }

  // Font file names and their corresponding font indices
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
    if (!fontFileName) {
      printf("LoadBundledFonts: failed to create string for %s\n",
             fontFiles[i].filename);
      continue;
    }

    CFURLRef fontURL = CFURLCreateCopyAppendingPathComponent(
        NULL, resourcesURL, fontFileName, false);
    CFRelease(fontFileName);
    if (!fontURL) {
      printf("LoadBundledFonts: failed to create URL for %s\n",
             fontFiles[i].filename);
      continue;
    }

    // Debug: print font URL
    char fontPath[1024];
    if (CFURLGetFileSystemRepresentation(fontURL, true, (UInt8 *)fontPath,
                                         sizeof(fontPath))) {
      printf("LoadBundledFonts: trying to load %s\n", fontPath);
    }

    // Register the font with Core Text (process-scope only)
    CFErrorRef error = NULL;
    if (CTFontManagerRegisterFontsForURL(fontURL, kCTFontManagerScopeProcess,
                                         &error)) {
      // Font registered successfully - now set up FLTK to use it
      Fl_Font fltkFont = FL_FREE_FONT + fontFiles[i].fontIndex;
      Fl::set_font(fltkFont, fontFiles[i].fontName);
      rgFltkAstroFont[fontFiles[i].fontIndex] = fltkFont;
      printf("Loaded font: %s (FLTK font %d)\n", fontFiles[i].fontName,
             fltkFont);
    } else {
      if (error) {
        CFStringRef desc = CFErrorCopyDescription(error);
        if (desc) {
          char buf[256];
          CFStringGetCString(desc, buf, sizeof(buf), kCFStringEncodingUTF8);
          printf("Failed to load font %s: %s\n", fontFiles[i].fontName, buf);
          CFRelease(desc);
        }
        CFRelease(error);
      }
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
  // FLTK doesn't support true alpha blending, so we simulate it by
  // blending the foreground color with the background color (gi.kiOff)
  if (alpha >= 255) {
    fl_color(FltkColorFromKI(ki));
    return;
  }
  if (ki < 0 || ki >= cColor)
    ki = 0;

  // Get foreground and background colors
  KV kvFg = rgbbmp[ki];
  KV kvBg = rgbbmp[gi.kiOff];

  // Blend based on alpha (alpha/255 * fg + (1 - alpha/255) * bg)
  uchar r = (uchar)((RgbR(kvFg) * alpha + RgbR(kvBg) * (255 - alpha)) / 255);
  uchar g = (uchar)((RgbG(kvFg) * alpha + RgbG(kvBg) * (255 - alpha)) / 255);
  uchar b = (uchar)((RgbB(kvFg) * alpha + RgbB(kvBg) * (255 - alpha)) / 255);

  fl_color(fl_rgb_color(r, g, b));
}

static void FltkSetLineWidth(double width) {
  // FLTK line width is integer-based, round to nearest
  int w = (int)(width + 0.5);
  if (w < 1)
    w = 1;
  // Reset to default solid line style with given width
  // Using 0 for default width to let FLTK use its default
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
  // FLTK doesn't support sub-pixel coordinates, round at the last moment
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

// Map Astrolog font index to FLTK font. Returns Fl_Font or -1 if not available.
static Fl_Font FltkFontFromFI(int fi) {
  // Fonts: 0=Astrolog vector, 1=Wingdings, 2=Astro, 3=EnigmaAstrology,
  //        4=HamburgSymbols, 5=Astronomicon, 6=Courier New, 7=Consolas,
  //        8=Arial, 9=HanksNakshatra
  static flag fMonoFontSet = fFalse;
  static Fl_Font fltkMonoFont = FL_COURIER;

  // On first call, set up Menlo on macOS (native monospace font)
  if (!fMonoFontSet) {
#ifdef __APPLE__
    Fl::set_font(FL_FREE_FONT, "Menlo");
    fltkMonoFont = FL_FREE_FONT;
#endif
    fMonoFontSet = fTrue;
  }

  switch (fi) {
  case fiCourier:
    return fltkMonoFont;
  case fiConsolas:
    return fltkMonoFont; // Use same monospace font
  case fiArial:
    return FL_HELVETICA; // Sans-serif alternative
  case fiAstro:
  case fiEnigma:
  case fiHamburg:
  case fiAstronom:
  case fiNakshatr:
    // Check if bundled astrology fonts were loaded
    if (fAstroFontsLoaded && rgFltkAstroFont[fi] != (Fl_Font)-1)
      return rgFltkAstroFont[fi];
    break;
  default:
    break;
  }
  return (Fl_Font)-1;
}

// Map specific glyph scale (percentage)
// This is separate from gs.nScaleText so map glyphs can be scaled independently
// of the UI text/information/legends.
static int nMapGlyphScale = 100;

void FltkInitMapGlyphScale(void) { nMapGlyphScale = 100; }

void FltkAdjustMapGlyphScale(int delta) {
  nMapGlyphScale += delta;
  if (nMapGlyphScale < 50)
    nMapGlyphScale = 50;
  if (nMapGlyphScale > 400)
    nMapGlyphScale = 400;
}

// Draw a single glyph using FLTK fonts
// Returns 1 if drawn, 0 to fall back to vector rendering
static int FltkPutGlyph(int ch, int x, int y, int nFont, int nScale) {
  // Reset line style to default before text rendering to avoid interference
  fl_line_style(0);

  Fl_Font font;
  int fontSize;
  char sz[8];
  int w, h;

  // Check if font is available
  font = FltkFontFromFI(nFont);
  if (font == (Fl_Font)-1)
    return 0; // Font not available, use vector fallback

  // Calculate font size with smooth scaling based on window size.
  // Same sidebar detection logic as CairoPutGlyph.
  int baseSize = (nFont >= fiAstro && nFont <= fiNakshatr) ? 16 : 12;

  // Sidebar detection: sidebar glyphs are drawn at x = gs.xWin - 12*gi.nScale
  // (near right edge). Use a tight margin based on scale.
  int sidebarMargin = 14 * gi.nScale;
  int isSidebarGlyph = (x > gs.xWin - sidebarMargin);

  if (isSidebarGlyph) {
    fontSize = 9 * nScale / 100;
  } else {
    // For map modes (AstroGraph/WorldMap), clamp the nScale used for font
    // sizing to prevent excessively large glyphs when the map is expanded.
    int effectiveScale = gs.nScale;
    if (gi.nMode == gAstroGraph || gi.nMode == gWorldMap ||
        gi.nMode == gGlobe || gi.nMode == gPolar) {
      // Cap the effective scale for glyph calculation.
      // A scale of 180 gives ~28pt max font size, which is large enough but not
      // huge.
      if (effectiveScale > 180)
        effectiveScale = 180;
    }
    // Calculate base font size from scale
    fontSize = baseSize * effectiveScale * nScale / 10000;

    // Apply map-specific text scale factor if in map mode
    if (gi.nMode == gAstroGraph || gi.nMode == gWorldMap ||
        gi.nMode == gGlobe || gi.nMode == gPolar) {
      if (nMapGlyphScale != 100) {
        fontSize = fontSize * nMapGlyphScale / 100;
      }
    }
    // For non-map modes, or if explicitly desired, we could respect
    // gs.nScaleText but typically standard charts scale everything evenly via
    // gs.nScale.
  }

  if (fontSize < 6)
    fontSize = 6;

  // On HiDPI displays with FLTK, we use logical font sizes.
  // FLTK handles the high-resolution rasterization internally.
  // We do NOT need to manually scale up the font size for Retina,
  // as that would result in double-scaling (oversized text).

  fl_font(font, fontSize);

  // Handle Unicode characters for text fonts
  if (nFont >= fiCourier && ch > 127) {
    // Convert Unicode codepoint to UTF-8
    if (ch < 0x80) {
      sz[0] = (char)ch;
      sz[1] = '\0';
    } else if (ch < 0x800) {
      sz[0] = (char)(0xC0 | (ch >> 6));
      sz[1] = (char)(0x80 | (ch & 0x3F));
      sz[2] = '\0';
    } else {
      sz[0] = (char)(0xE0 | (ch >> 12));
      sz[1] = (char)(0x80 | ((ch >> 6) & 0x3F));
      sz[2] = (char)(0x80 | (ch & 0x3F));
      sz[3] = '\0';
    }
  } else {
    sz[0] = (char)ch;
    sz[1] = '\0';
  }

  // Measure text to center it
  fl_measure(sz, w, h, 0);
  fl_draw(sz, x - w / 2, y + h / 2 - fl_descent());

  return 1; // Successfully rendered
}

// Draw a text string using FLTK fonts
// Returns 1 if drawn, 0 to fall back to vector rendering
static int FltkPutText(const char *sz, int x, int y, int nFont, int nScale) {
  // Reset line style to default before text rendering to avoid interference
  fl_line_style(0);

  Fl_Font font;
  int fontSize;
  int w, h;

  font = FltkFontFromFI(nFont);
  if (font == (Fl_Font)-1)
    return 0; // Font not available, use vector fallback

  // Calculate font size based on scale
  fontSize = 6 * nScale;
  if (fontSize < 6)
    fontSize = 6;

  fl_font(font, fontSize);

  // Measure text to center it
  fl_measure(sz, w, h, 0);
  fl_draw(sz, x - w / 2, y + h / 2 - fl_descent());

  return 1; // Successfully rendered
}

static void FltkClearScreen(int ki) {
  fl_color(FltkColorFromKI(ki));
  fl_rectf(0, 0, gs.xWin, gs.yWin);
}

static void FltkFlush(void) { Fl::flush(); }

static GB gbFltk = {"FLTK",           FltkSetColor,  FltkSetColorAlpha,
                    FltkSetLineWidth, FltkDrawPixel, FltkDrawPixelThick,
                    FltkDrawLine,     FltkDrawLineF, FltkDrawLineThick,
                    FltkDrawRect,     FltkDrawArc,   FltkDrawEllipse,
                    FltkPutGlyph,     FltkPutText,   FltkClearScreen,
                    FltkFlush,        NULL};

void InitBackendFltk(void) {
#ifdef __APPLE__
  // Load bundled astrology fonts from app bundle
  LoadBundledFonts();
#endif
  gpBackend = &gbFltk;
}
#endif // FLTK

/*
******************************************************************************
** Cairo Backend Implementation
******************************************************************************
*/

#ifdef CAIRO
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <cairo/cairo.h>

static cairo_t *gi_cr = NULL;
static cairo_surface_t *gi_surface = NULL;
static GB *gpBackendPrev = NULL; // Previous backend to restore

static void CairoSetColor(int ki) {
  if (ki < 0 || ki >= cColor)
    ki = 0;
  KV kv = rgbbmp[ki];
  cairo_set_source_rgb(gi_cr, (double)RgbR(kv) / 255.0,
                       (double)RgbG(kv) / 255.0, (double)RgbB(kv) / 255.0);
}

static void CairoSetColorAlpha(int ki, int alpha) {
  if (ki < 0 || ki >= cColor)
    ki = 0;
  KV kv = rgbbmp[ki];
  cairo_set_source_rgba(gi_cr, (double)RgbR(kv) / 255.0,
                        (double)RgbG(kv) / 255.0, (double)RgbB(kv) / 255.0,
                        (double)alpha / 255.0);
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
  // Draw without offset for proper antialiasing
  // Cairo will antialias across pixel boundaries
  cairo_move_to(gi_cr, (double)x1, (double)y1);
  cairo_line_to(gi_cr, (double)x2, (double)y2);
  cairo_stroke(gi_cr);
}

static void CairoDrawLineF(double x1, double y1, double x2, double y2) {
  // Native floating-point coordinates for smooth vector output
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
  // Cairo uses radians, Astrolog uses degrees
  // Also need to handle ellipse (non-circular arc)
  double cx = x + w / 2.0;
  double cy = y + h / 2.0;
  double rx = w / 2.0;
  double ry = h / 2.0;

  cairo_save(gi_cr);
  cairo_translate(gi_cr, cx, cy);
  cairo_scale(gi_cr, rx, ry);
  // Cairo angles are in radians, counterclockwise from positive x-axis
  // Convert degrees to radians, negate for clockwise direction
  cairo_arc(gi_cr, 0, 0, 1.0, -deg2 * rPi / 180.0, -deg1 * rPi / 180.0);
  cairo_restore(gi_cr);
  cairo_stroke(gi_cr);
}

static void CairoDrawEllipse(int x, int y, int w, int h) {
  double cx = x + w / 2.0;
  double cy = y + h / 2.0;
  double rx = w / 2.0;
  double ry = h / 2.0;

  cairo_save(gi_cr);
  cairo_translate(gi_cr, cx, cy);
  cairo_scale(gi_cr, rx, ry);
  cairo_arc(gi_cr, 0, 0, 1.0, 0, 2 * rPi);
  cairo_restore(gi_cr);
  cairo_fill(gi_cr);
}

static void CairoClearScreen(int ki) {
  CairoSetColor(ki);
  cairo_paint(gi_cr);
}

static void CairoFlush(void) { cairo_surface_flush(gi_surface); }

// Get font name for Cairo from Astrolog font index
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
    return "Menlo";
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
  default:
    return NULL;
  }
}

// Draw a single glyph using Cairo fonts
// Returns 1 if drawn, 0 to fall back to vector rendering
static int CairoPutGlyph(int ch, int x, int y, int nFont, int nScale) {
  const char *fontName = CairoFontName(nFont);
  if (!fontName)
    return 0; // Unknown font, use vector fallback

  // Calculate font size with smooth scaling based on window size.
  // Detect sidebar: track gs.xWin changes. During chart drawing, gs.xWin is
  // reduced. When DrawSidebar runs, it increases gs.xWin back. Sidebar glyphs
  // have x >= the chart width (stored xWin before increase).
  int baseSize = (nFont >= fiAstro && nFont <= fiNakshatr) ? 16 : 12;
  double fontSize;

  // Sidebar detection: sidebar glyphs are drawn at x = gs.xWin - 12*gi.nScale
  // (near right edge). Chart glyphs are distributed across the chart area.
  // Use a tight margin based on scale to avoid false positives for chart
  // glyphs.
  int sidebarMargin = 14 * gi.nScale;
  int isSidebarGlyph = (x > gs.xWin - sidebarMargin);

  if (isSidebarGlyph) {
    // Sidebar - use small fixed size to fit 10-pixel line spacing
    fontSize = 9.0 * nScale / 100.0;
  } else {
    // Chart area - use gs.nScale for smooth proportional scaling
    // gs.nScale is percentage (100 = 1x, 150 = 1.5x, 200 = 2x)
    fontSize = (double)(baseSize * gs.nScale * nScale) / 10000.0;
  }

  if (fontSize < 6.0)
    fontSize = 6.0;

  // Select font
  cairo_select_font_face(gi_cr, fontName, CAIRO_FONT_SLANT_NORMAL,
                         gs.fThick ? CAIRO_FONT_WEIGHT_BOLD
                                   : CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(gi_cr, fontSize);

  // Build the character string - convert to UTF-8 for Cairo
  // The astrology fonts use characters 0x80-0xFF which need to be
  // encoded as UTF-8 (2 bytes for characters >= 0x80)
  char sz[8];
  if (ch < 0x80) {
    // ASCII - pass through
    sz[0] = (char)ch;
    sz[1] = '\0';
  } else if (ch < 0x100) {
    // High byte - encode as UTF-8 (Latin-1 to UTF-8 conversion)
    // Characters 0x80-0xFF map to Unicode U+0080-U+00FF
    sz[0] = (char)(0xC0 | ((ch >> 6) & 0x1F));
    sz[1] = (char)(0x80 | (ch & 0x3F));
    sz[2] = '\0';
  } else {
    // Unicode > 0xFF (shouldn't happen for these fonts)
    sz[0] = (char)(0xE0 | ((ch >> 12) & 0x0F));
    sz[1] = (char)(0x80 | ((ch >> 6) & 0x3F));
    sz[2] = (char)(0x80 | (ch & 0x3F));
    sz[3] = '\0';
  }

  // Measure text to center it
  cairo_text_extents_t extents;
  cairo_text_extents(gi_cr, sz, &extents);

  // Check if glyph exists (width > 0)
  if (extents.width < 0.5)
    return 0; // Glyph not found, fall back to vector

  // Draw centered at (x, y)
  double dx = x - (extents.width / 2.0 + extents.x_bearing);
  double dy = y + (extents.height / 2.0);
  cairo_move_to(gi_cr, dx, dy);
  cairo_show_text(gi_cr, sz);

  return 1; // Successfully rendered
}

// Draw a text string using Cairo fonts
// Returns 1 if drawn, 0 to fall back to vector rendering
static int CairoPutText(const char *sz, int x, int y, int nFont, int nScale) {
  const char *fontName = CairoFontName(nFont);
  if (!fontName)
    return 0; // Unknown font, use vector fallback

  // Calculate font size to match vector font character width.
  // Vector font uses xFont2 * nScale = 3 * nScale pixels per character.
  // For Menlo, char_width ≈ 0.6 * fontSize, so fontSize = 3*nScale/0.6 =
  // 5*nScale.
  double fontSize = 5.0 * nScale;
  if (fontSize < 8.0)
    fontSize = 8.0;

  // Select font
  cairo_select_font_face(gi_cr, fontName, CAIRO_FONT_SLANT_NORMAL,
                         gs.fThick ? CAIRO_FONT_WEIGHT_BOLD
                                   : CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(gi_cr, fontSize);

  // Measure text for vertical positioning
  cairo_text_extents_t extents;
  cairo_text_extents(gi_cr, sz, &extents);

  // Draw left-aligned at (x, y) where y is approximate baseline
  // x is the left edge, y is vertically centered
  double dx = x - extents.x_bearing;
  double dy = y - extents.y_bearing - extents.height / 2.0;
  cairo_move_to(gi_cr, dx, dy);
  cairo_show_text(gi_cr, sz);

  return 1; // Successfully rendered
}

static GB gbCairo = {"Cairo",
                     CairoSetColor,
                     CairoSetColorAlpha,
                     CairoSetLineWidth,
                     CairoDrawPixel,
                     CairoDrawPixelThick,
                     CairoDrawLine,
                     CairoDrawLineF,
                     CairoDrawLineThick,
                     CairoDrawRect,
                     CairoDrawArc,
                     CairoDrawEllipse,
                     CairoPutGlyph,
                     CairoPutText,
                     CairoClearScreen,
                     CairoFlush,
                     NULL};

void InitBackendCairo(cairo_surface_t *surface) {
  gpBackendPrev = gpBackend;
  gi_surface = surface;
  gi_cr = cairo_create(gi_surface);

  // Set default drawing properties for antialiased rendering
  cairo_set_line_width(gi_cr, 1.0);
  cairo_set_line_cap(gi_cr,
                     CAIRO_LINE_CAP_ROUND); // Round caps for smooth line ends
  cairo_set_line_join(gi_cr,
                      CAIRO_LINE_JOIN_ROUND); // Round joins for smooth corners
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

/*
******************************************************************************
** Backend Selection
******************************************************************************
*/

// Initialize the appropriate backend based on compile-time configuration
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
