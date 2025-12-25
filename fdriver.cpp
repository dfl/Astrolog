/*
** Astrolog (Version 7.80) File: fdriver.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** FLTK GUI implementation for Astrolog
*/

#include "astrolog.h"
#include "xbackend.h"
#include "xevent.h"

#ifdef FLTK
#include "fdriver.h"
#ifdef OPENGL
#include "fgl.h"
#endif
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Terminal.H>
#include <FL/fl_ask.H>
#include <unistd.h> // For unlink()

#ifdef CAIRO
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <cairo/cairo.h>

#endif

// Global FLTK state
FI fi = {0};

// Cairo rendering is always enabled when available (no toggle)
#ifdef CAIRO
static int fUseCairo = fTrue;
#endif

// Manual character scale multiplier (100=1x, 200=2x, 300=3x, 400=4x)
static int nCharScaleManual = 100;

// Auto-square window on resize (for chart types that need it)
static flag fAutoSquare = fFalse;

// Flag to enable OpenGL 3D rendering
#ifdef OPENGL
static int fUseOpenGL = fTrue; // Default to OpenGL rendering when available

void FMenuViewOpenGL(Fl_Widget *w, void *data) {
  fUseOpenGL = !fUseOpenGL;

  // Update menu checkmark
  if (fi.menubar) {
    Fl_Menu_Item *item = (Fl_Menu_Item *)fi.menubar->find_item(FMenuViewOpenGL);
    if (item) {
      if (fUseOpenGL)
        item->set();
      else
        item->clear();
    }
  }

  // If currently in a 3D mode, switch widgets accordingly
  if (fi.window && FIs3DChartMode(gi.nMode)) {
    // Set fDoCast to ensure chart data is available
    fi.fDoCast = fTrue;
    fi.window->switchTo3D(fUseOpenGL);
  }
}

// Check if OpenGL rendering is enabled
int FUseOpenGL(void) { return fUseOpenGL; }
#endif

// Convert Astrolog color index to FLTK color
// Astrolog stores colors as 0x00BBGGRR (Windows COLORREF format)
Fl_Color FltkColorFromKI(int ki) {
  if (ki < 0 || ki >= cColor)
    ki = 0;
  KV kv = rgbbmp[ki];
  uchar r = RgbR(kv);
  uchar g = RgbG(kv);
  uchar b = RgbB(kv);
  return fl_rgb_color(r, g, b);
}

// Forward declarations for helper functions
static void UpdateMenuCheck(Fl_Callback *cb, flag f);
static void UpdateMenuRadio(Fl_Callback *cb);
static void UpdateMenuRadioByValue(Fl_Callback *cb, intptr_t value);
static flag HandleAltMenuShortcut(int key);

// On macOS, strip '&' mnemonic characters from menu labels since Alt+key
// shortcuts don't work. On Windows/Linux, keep them for keyboard navigation.
#ifdef __APPLE__
static const char *StripMnemonic(const char *label, char *buf, size_t bufSize) {
  size_t j = 0;
  for (size_t i = 0; label[i] && j < bufSize - 1; i++) {
    if (label[i] != '&')
      buf[j++] = label[i];
  }
  buf[j] = '\0';
  return buf;
}
#define MENU_LABEL(s) StripMnemonic(s, menuLabelBuf, sizeof(menuLabelBuf))
#else
#define MENU_LABEL(s) (s)
#endif

// Forward declarations for view menu callbacks (used by handleKey)
void FMenuViewWheel(Fl_Widget *w, void *data);
void FMenuViewGrid(Fl_Widget *w, void *data);
void FMenuViewHorizon(Fl_Widget *w, void *data);
void FMenuViewOrbit(Fl_Widget *w, void *data);
void FMenuViewAstroGraph(Fl_Widget *w, void *data);
void FMenuViewGlobe(Fl_Widget *w, void *data);
void FMenuViewSphere(Fl_Widget *w, void *data);
void FMenuViewTelescope(Fl_Widget *w, void *data);
void FMenuViewPolar(Fl_Widget *w, void *data);
void FMenuViewWorldMap(Fl_Widget *w, void *data);

// Forward declarations for include toggles (used by handleKey)
void FMenuIncludeMinors(Fl_Widget *w, void *data);
void FMenuIncludeCusps(Fl_Widget *w, void *data);
void FMenuIncludeUranians(Fl_Widget *w, void *data);
void FMenuIncludeDwarfs(Fl_Widget *w, void *data);
void FMenuIncludeMoons(Fl_Widget *w, void *data);
void FMenuIncludeCOB(Fl_Widget *w, void *data);
void FMenuIncludeStars(Fl_Widget *w, void *data);

// Forward declarations for other menu callbacks (used by handleKey)
void FMenuChartNow(Fl_Widget *w, void *data);
void FMenuSidereal(Fl_Widget *w, void *data);
void FMenuHeliocentric(Fl_Widget *w, void *data);
void FMenuGraphicsReverse(Fl_Widget *w, void *data);
void FMenuGraphicsMonochrome(Fl_Widget *w, void *data);
void FMenuReduceContrast(Fl_Widget *w, void *data);
void FMenuGraphicsBorder(Fl_Widget *w, void *data);
void FMenuGraphicsText(Fl_Widget *w, void *data);
void FMenuGraphicsThick(Fl_Widget *w, void *data);
void FMenuGraphicsLabel(Fl_Widget *w, void *data);
void FMenuGraphicsLabelAsp(Fl_Widget *w, void *data);
void FMenuGraphicsModify(Fl_Widget *w, void *data);
void FMenuGraphicsHouseExtra(Fl_Widget *w, void *data);
void FMenuGraphicsEquator(Fl_Widget *w, void *data);
void FMenuViewSector(Fl_Widget *w, void *data);
void FMenuViewCalendar(Fl_Widget *w, void *data);
void FMenuViewInfluence(Fl_Widget *w, void *data);
void FMenuViewEphemeris(Fl_Widget *w, void *data);
void FMenuViewRising(Fl_Widget *w, void *data);
void FMenuAnimToggle(Fl_Widget *w, void *data);
void FMenuAnimNow(Fl_Widget *w, void *data);
void FMenuAnimRate1(Fl_Widget *w, void *data);
void FMenuAnimRate2(Fl_Widget *w, void *data);
void FMenuAnimRate3(Fl_Widget *w, void *data);
void FMenuAnimRate4(Fl_Widget *w, void *data);
void FMenuAnimRate5(Fl_Widget *w, void *data);
void FMenuAnimRate6(Fl_Widget *w, void *data);
void FMenuAnimRate7(Fl_Widget *w, void *data);
void FMenuAnimRate8(Fl_Widget *w, void *data);
void FMenuAnimRate9(Fl_Widget *w, void *data);
void FMenuAnimFactor1(Fl_Widget *w, void *data);
void FMenuAnimFactor2(Fl_Widget *w, void *data);
void FMenuAnimFactor3(Fl_Widget *w, void *data);
void FMenuAnimFactor4(Fl_Widget *w, void *data);
void FMenuAnimFactor5(Fl_Widget *w, void *data);
void FMenuAnimFactor6(Fl_Widget *w, void *data);
void FMenuAnimFactor7(Fl_Widget *w, void *data);
void FMenuAnimFactor8(Fl_Widget *w, void *data);
void FMenuAnimFactor9(Fl_Widget *w, void *data);
void FMenuAnimTimedExposure(Fl_Widget *w, void *data);
void FMenuRelNo(Fl_Widget *w, void *data);
void FMenuRelSynastry(Fl_Widget *w, void *data);
void FMenuRelComposite(Fl_Widget *w, void *data);
void FMenuRelMidpoint(Fl_Widget *w, void *data);
void FMenuRelDate(Fl_Widget *w, void *data);
void FMenuRelBiorhythm(Fl_Widget *w, void *data);
void FMenuRelTransit(Fl_Widget *w, void *data);
void FMenuRelProgressed(Fl_Widget *w, void *data);
void FMenuHouseSystem(Fl_Widget *w, void *data);
void FMenuHouseSolar(Fl_Widget *w, void *data);
void FMenuHouse3D(Fl_Widget *w, void *data);
void FMenuHouseDecan(Fl_Widget *w, void *data);
void FMenuHouseFlip(Fl_Widget *w, void *data);
void FMenuMapConstel(Fl_Widget *w, void *data);
void FMenuMapAllStar(Fl_Widget *w, void *data);
void FMenuMapExo(Fl_Widget *w, void *data);
void FMenuMapCity(Fl_Widget *w, void *data);
void FMenuMapBmp(Fl_Widget *w, void *data);
void FMenuMapAxis(Fl_Widget *w, void *data);
void FMenuIndian(Fl_Widget *w, void *data);
void FMenuIndianS(Fl_Widget *w, void *data);
void FMenuIndianN(Fl_Widget *w, void *data);
void FMenuIndianE(Fl_Widget *w, void *data);
void FMenuGlyphFont(Fl_Widget *w, void *data);
void FMenuHelpSign(Fl_Widget *w, void *data);
void FMenuHelpObject(Fl_Widget *w, void *data);
void FMenuHelpAspect(Fl_Widget *w, void *data);
void FMenuHelpConstel(Fl_Widget *w, void *data);
void FMenuHelpPlanet(Fl_Widget *w, void *data);
void FMenuHelpRay(Fl_Widget *w, void *data);
void FMenuHelpMeaning(Fl_Widget *w, void *data);
void FMenuHelpSwitch(Fl_Widget *w, void *data);
void FMenuHelpObscure(Fl_Widget *w, void *data);
void FMenuHelpKeystroke(Fl_Widget *w, void *data);
void FMenuHelpCredit(Fl_Widget *w, void *data);

// Forward declarations for new chart view callbacks
void FMenuViewList(Fl_Widget *w, void *data);
void FMenuViewHouse(Fl_Widget *w, void *data);
void FMenuViewAspect(Fl_Widget *w, void *data);
void FMenuViewEsoteric(Fl_Widget *w, void *data);
#ifdef ARABIC
void FMenuViewArabic(Fl_Widget *w, void *data);
#endif
void FMenuViewNearestCity(Fl_Widget *w, void *data);
void FMenuViewMoons(Fl_Widget *w, void *data);
void FMenuViewExo(Fl_Widget *w, void *data);

// Forward declarations for new toggle callbacks
void FMenuGraphicsToggle(Fl_Widget *w, void *data);
void FMenuInterpret(Fl_Widget *w, void *data);
void FMenuSecond(Fl_Widget *w, void *data);
void FMenuParallel(Fl_Widget *w, void *data);
void FMenuApplying(Fl_Widget *w, void *data);
void FMenuRedraw(Fl_Widget *w, void *data);
void FMenuClear(Fl_Widget *w, void *data);

// Forward declarations for new settings callbacks
void FMenuHouseSetDwad(Fl_Widget *w, void *data);
void FMenuHouseSetGeodetic(Fl_Widget *w, void *data);
void FMenuHouseSetNavamsa(Fl_Widget *w, void *data);
void FMenuHouseSetIndian(Fl_Widget *w, void *data);

// Forward declarations for new graphics callbacks
void FMenuGraphicsSidebar(Fl_Widget *w, void *data);
void FMenuGraphicsAutoSquare(Fl_Widget *w, void *data);
void FMenuGraphicsAntialias(Fl_Widget *w, void *data);
void FMenuDashStyle(Fl_Widget *w, void *data);
void FMenuModifyChart(Fl_Widget *w, void *data);
void FMenuPenColor(Fl_Widget *w, void *data);
void FMenuWheelFill(Fl_Widget *w, void *data);
void FMenuScaleDecrease(Fl_Widget *w, void *data);
void FMenuScaleIncrease(Fl_Widget *w, void *data);
void FMenuScale1(Fl_Widget *w, void *data);
void FMenuScale2(Fl_Widget *w, void *data);
void FMenuScale3(Fl_Widget *w, void *data);
void FMenuScale4(Fl_Widget *w, void *data);
void FMenuTextDecrease(Fl_Widget *w, void *data);
void FMenuTextIncrease(Fl_Widget *w, void *data);
void FMenuTiltZero(Fl_Widget *w, void *data);
void FMenuTiltNorth(Fl_Widget *w, void *data);
void FMenuTiltSouth(Fl_Widget *w, void *data);
void FMenuRotateWest(Fl_Widget *w, void *data);
void FMenuRotateEast(Fl_Widget *w, void *data);
void FMenuZoomIn(Fl_Widget *w, void *data);
void FMenuZoomOut(Fl_Widget *w, void *data);

// Forward declarations for new animate callbacks
void FMenuStore(Fl_Widget *w, void *data);
void FMenuRecall(Fl_Widget *w, void *data);
void FMenuAnimRate_11(Fl_Widget *w, void *data);
void FMenuAnimRate_12(Fl_Widget *w, void *data);
void FMenuAnimRate_13(Fl_Widget *w, void *data);

// Forward declarations for dialog callbacks
void FMenuTransitDlg(Fl_Widget *w, void *data);
void FMenuProgressDlg(Fl_Widget *w, void *data);
void FMenuChartSettingsDlg(Fl_Widget *w, void *data);

// Forward declarations for info menu callbacks
void FMenuSwapCharts(Fl_Widget *w, void *data);
void FMenuDefaultInfo(Fl_Widget *w, void *data);
void FMenuInfoAll(Fl_Widget *w, void *data);
void FMenuListPrev(Fl_Widget *w, void *data);
void FMenuListNext(Fl_Widget *w, void *data);
void FMenuListFirst(Fl_Widget *w, void *data);
void FMenuListLast(Fl_Widget *w, void *data);
void FMenuListDlg(Fl_Widget *w, void *data);
void FMenuMoonRestrict(Fl_Widget *w, void *data);
void FMenuMoonObjSettings(Fl_Widget *w, void *data);
void FMenuObjCustom(Fl_Widget *w, void *data);
void FMenuStarCustom(Fl_Widget *w, void *data);

// Forward declarations for file menu callbacks
void FMenuFileOpenChart2(Fl_Widget *w, void *data);
void FMenuFileSaveSettings(Fl_Widget *w, void *data);
void FMenuFilePrint(Fl_Widget *w, void *data);

// Forward declarations for settings menu callbacks
void FMenuObjectSettings(Fl_Widget *w, void *data);
void FMenuObjectSettings2(Fl_Widget *w, void *data);
void FMenuStarRestrict(Fl_Widget *w, void *data);

// Forward declarations for help menu callbacks
void FMenuHelpWebsite(Fl_Widget *w, void *data);
void FMenuHelpChanges(Fl_Widget *w, void *data);
void FMenuHelpLicense(Fl_Widget *w, void *data);
void FMenuDocDefault(Fl_Widget *w, void *data);
void FMenuDocAtlas(Fl_Widget *w, void *data);
void FMenuDocTimezone(Fl_Widget *w, void *data);
void FMenuDocStar(Fl_Widget *w, void *data);
void FMenuDocOrbit(Fl_Widget *w, void *data);
void FMenuDocExo(Fl_Widget *w, void *data);

// Forward declarations for window settings callbacks
void FMenuFullScreen(Fl_Widget *w, void *data);

// Edit menu - Copy/Export Text
void FMenuExportText(Fl_Widget *w, void *data);
void FMenuCopyText(Fl_Widget *w, void *data);

// Text window
void FMenuShowTextWindow(Fl_Widget *w, void *data);

// Macro callbacks
void FMenuMacro(Fl_Widget *w, void *data);

// Other Formats submenu callbacks
void FMenuSaveChartList(Fl_Widget *w, void *data);
void FMenuSaveAAF(Fl_Widget *w, void *data);
void FMenuSaveQuick(Fl_Widget *w, void *data);

// Open Bitmap submenu callbacks
void FMenuOpenBackground(Fl_Widget *w, void *data);
void FMenuOpenWorldMap(Fl_Widget *w, void *data);

/*
******************************************************************************
** ChartWidget Implementation
******************************************************************************
*/

ChartWidget::ChartWidget(int x, int y, int w, int h)
    : Fl_Widget(x, y, w, h), mousex_(-1), mousey_(-1), buttonx_(-1),
      buttony_(-1) {}

ChartWidget::~ChartWidget() {}

void ChartWidget::draw() {
  // Clear the entire widget area with background color first
  fl_color(FL_BLACK);
  fl_rectf(x(), y(), w(), h());

  // Set the chart size to match the widget
  gs.xWin = w();
  gs.yWin = h();

  // Auto-scale for map charts (World Map, AstroGraph) to fill window smoothly
  // These charts use a 360x180 degree coordinate system
  if (gi.nMode == gWorldMap || gi.nMode == gAstroGraph) {
    // Use floating-point scaling for smooth resize
    gi.rScaleX = (real)gs.xWin / 360.0;
    gi.rScaleY = (real)gs.yWin / 180.0;
    // Keep integer scale for compatibility with other drawing functions
    int scaleX = (gs.xWin * 100) / 360;
    int scaleY = (gs.yWin * 100) / 180;
    gs.nScale = Min(scaleX, scaleY);
    gs.nScale = Max(gs.nScale, 100);
    gi.nScale = gs.nScale / 100;
  } else if (gi.nMode == gGrid) {
    // Auto-scale for aspect grid to fill window smoothly
    // First calculate gi.nGridCell (number of planets shown) if not already set
    if (gs.nGridCell > 0)
      gi.nGridCell = gs.nGridCell;
    else {
      gi.nGridCell = 0;
      for (int i = 0; i <= is.nObj; i++)
        gi.nGridCell += FProper(i);
    }
    // Calculate floating-point scale to fill window smoothly
    int nCells = gi.nGridCell + (us.nRel <= rcDual);
    if (nCells > 0) {
      int winSize = Min(gs.xWin, gs.yWin);
      // Use floating-point scaling for smooth resize (like map charts)
      gi.rScaleX = gi.rScaleY = (real)winSize / (real)(nCells * CELLSIZE);
      // Keep integer scale for compatibility, but use floating-point for
      // drawing
      gi.nScale = (int)gi.rScaleX;
      if (gi.nScale < 1)
        gi.nScale = 1;
      // Use fractional scale for smooth glyph sizing
      gs.nScale = Max((int)(gi.rScaleX * 100.0), 100);
    }
  } else {
    // Non-map charts: auto-scale based on window size relative to default
    // Use both dimensions to ensure scaling responds to any resize direction
    int minDim = Min(gs.xWin, gs.yWin);
    // Use a smaller reference (440) so glyphs scale up even at default window
    // size This accounts for the fact that the default 600px matches typical
    // minimum dimension
    int defDim = 440;
    real baseScale = (real)minDim / (real)defDim;
    // Apply manual scale multiplier (nCharScaleManual) to base scale
    real finalScale = baseScale * (nCharScaleManual / 100.0);
    gi.rScaleX = gi.rScaleY = finalScale;
    gi.nScale = Max((int)finalScale, 1);
    gs.nScale = Max((int)(finalScale * 100.0), 100);
  }

#ifdef CAIRO
  if (fUseCairo && gi.nMode != gAstroGraph && gi.nMode != gWorldMap &&
      gi.nMode != gGlobe && gi.nMode != gPolar) {
    // Get HiDPI scale factor - platform-specific detection
    float scale = Fl::screen_scale(0);

#ifdef __APPLE__
    // On macOS, FLTK reports scale=1.0 but handles Retina internally
    // However for Cairo, we render our own surface so we need the actual scale
    if (scale < 1.5f) {
      // Use macOS default of 2.0 for Retina displays
      // This works on all Macs from ~2013 onwards
      scale = 2.0f;
    }
#elif defined(_WIN32)
    // On Windows, get DPI scale from system
    // GetDeviceCaps returns 96 for 100%, 120 for 125%, 144 for 150%, 192 for
    // 200%
    if (scale < 1.1f) {
      HDC hdc = GetDC(NULL);
      if (hdc) {
        int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        scale = (float)dpi / 96.0f;
        ReleaseDC(NULL, hdc);
      }
    }
#else
    // On Linux/Unix, FLTK 1.4 should report correct scale on Wayland
    // For X11, check FLTK_SCALING_FACTOR or GDK_SCALE environment
    if (scale < 1.1f) {
      const char *envScale = getenv("FLTK_SCALING_FACTOR");
      if (!envScale)
        envScale = getenv("GDK_SCALE");
      if (envScale) {
        float envVal = (float)atof(envScale);
        if (envVal >= 1.0f)
          scale = envVal;
      }
    }
#endif

    // Debug: print scale factor once
    static int debugOnce = 0;
    if (!debugOnce) {
      printf("Cairo rendering: scale=%.2f (fltk_scale=%.2f), widget=%dx%d, "
             "surface=%dx%d\n",
             scale, Fl::screen_scale(0), w(), h(), (int)(w() * scale),
             (int)(h() * scale));
      debugOnce = 1;
    }

    int surfW = (int)(w() * scale);
    int surfH = (int)(h() * scale);

    // Cairo rendering path - antialiased, resolution-independent
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surfW, surfH);

    // Switch to Cairo backend
    InitBackendCairo(surface);

    // Scale the context for HiDPI
    cairo_t *cr = CairoContext();
    if (scale != 1.0f) {
      cairo_scale(cr, scale, scale);
    }

    // Clear background
    GBClearScreen(gi.kiOff);

    // Draw the chart using Cairo backend
    gi.fFile = fFalse;
    DrawChartX();

    // Blit Cairo surface to FLTK widget
    cairo_surface_flush(surface);
    unsigned char *data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);

    // Convert BGRA to RGBA for fl_draw_image
    // Cairo on little-endian is BGRA in memory
    for (int row = 0; row < surfH; row++) {
      unsigned char *p = data + row * stride;
      for (int col = 0; col < surfW; col++) {
        unsigned char b = p[0];
        unsigned char g = p[1];
        unsigned char r = p[2];
        unsigned char a = p[3];
        p[0] = r;
        p[1] = g;
        p[2] = b;
        p[3] = a;
        p += 4;
      }
    }

    // Draw scaled image to widget
    // Use Fl_RGB_Image for proper scaling on HiDPI
    Fl_RGB_Image img(data, surfW, surfH, 4, stride);
    img.scale(w(), h(), 1, 1); // Scale with high quality
    img.draw(x(), y());

    // Clean up
    EndBackendCairo();
    cairo_surface_destroy(surface);
  } else
#endif
  {
    // FLTK rendering path (original)
    fl_push_clip(x(), y(), w(), h());

    // Clear background
    fl_color(FltkColorFromKI(gi.kiOff));
    fl_rectf(x(), y(), w(), h());

    // Set translation so drawing coordinates start at 0,0
    fl_push_matrix();
    fl_translate(x(), y());

    // Call Astrolog's chart drawing function
    gi.fFile = fFalse; // Drawing to screen, not file
    DrawChartX();

    fl_pop_matrix();
    fl_pop_clip();
  }
}

int ChartWidget::handle(int event) {
  int mx = Fl::event_x() - x();
  int my = Fl::event_y() - y();

  switch (event) {
  case FL_PUSH:
    mousex_ = buttonx_ = mx;
    mousey_ = buttony_ = my;
    take_focus(); // Grab keyboard focus when clicked

    if (Fl::event_button() == FL_LEFT_MOUSE) {
      // Left click - draw point or interact with chart
      if (Fl::event_state() & FL_ALT) {
        // Alt+click: relocate chart on map
        if (fMap && !gs.fConstel && !gs.fMollewide) {
          CalculateMapCoords(mx, my, &Lon, &Lat);
          fi.xMouse = -1;
          ciCore = ciMain;
          fi.fDoCast = fTrue;
          redraw();
        }
      } else if (gi.nMode == gMidpoint && !gs.fEquator) {
        // Click on midpoint dial: orient arrow to clicked position
        // Match center calculation from XChartMidpoint
        int chartWidth = gs.xWin;
        if (gs.fText && gs.fDoSidebar)
          chartWidth -= xSideT;
        int cx = chartWidth / 2 - 1;
        int cy = gs.yWin / 2 - 1;
        real dx = (real)(mx - cx);
        real dy = (real)(my - cy);
        if (dx != 0.0 || dy != 0.0) {
          // Convert screen position to angle (270 is top, clockwise positive)
          real screenAngle = RAngleD(dx, dy);
          real rxi = !us.fIndian ? 1.0 : -1.0;
          gs.rRot = Mod((270.0 - screenAngle) * rxi);
          gs.objTrack = -1; // Clear any tracked object
          redraw();
        }
      }
      return 1;
    } else if (Fl::event_button() == FL_RIGHT_MOUSE) {
      // Right click - could show context menu
      return 1;
    } else if (Fl::event_button() == FL_MIDDLE_MOUSE) {
      // Middle click - show coordinates on map
      if ((gi.nMode == gAstroGraph || gi.nMode == gWorldMap) &&
          gs.rRot == 0.0) {
        real lon = rDegHalf - (real)(mx - 1) / (real)(gs.xWin - 2) * rDegMax;
        real lat = rDegQuad - (real)(my - 1) / (real)(gs.yWin - 2) * 181.0;
        char sz[cchSzMax];
        sprintf(sz, "Mouse at %s", SzLocation(lon, lat));
        PrintProgress(sz);
      }
      return 1;
    }
    break;

  case FL_DRAG:
    // Both left and right mouse drag can rotate globe/map views
    if (us.fGraphics && FSupportsRotation(gi.nMode) &&
        !(Fl::event_state() & (FL_SHIFT | FL_ALT))) {
      if (gi.nMode == gMidpoint && !gs.fEquator) {
        // Midpoint dial: make arrow follow mouse position directly
        int chartWidth = gs.xWin;
        if (gs.fText && gs.fDoSidebar)
          chartWidth -= xSideT;
        int cx = chartWidth / 2 - 1;
        int cy = gs.yWin / 2 - 1;
        real dx = (real)(mx - cx);
        real dy = (real)(my - cy);
        if (dx != 0.0 || dy != 0.0) {
          real screenAngle = RAngleD(dx, dy);
          real rxi = !us.fIndian ? 1.0 : -1.0;
          gs.rRot = Mod((270.0 - screenAngle) * rxi);
          gs.objTrack = -1;
        }
      } else {
        // Calculate rotation delta with mode-specific factor
        real rFactor = (gi.nMode == gLocal || gi.nMode == gTelescope)
                           ? -gi.zViewRatio
                           : 1.0;
        gs.rRot += (real)(mx - mousex_) * rDegHalf / (real)gs.xWin * rFactor;

        // Calculate tilt delta with mode-specific factor
        rFactor = (gi.nMode == gLocal || gi.nMode == gTelescope)
                      ? gi.zViewRatio
                      : (gi.nMode == gGlobe ? -1.0 : 1.0);
        gs.rTilt += (real)(my - mousey_) * rDegHalf / (real)gs.yWin * rFactor;

        // Clamp values using shared helpers
        ClampRotation();
        ClampTilt();

        if (gi.nMode == gTelescope)
          gs.objTrack = -1;
      }
      mousex_ = mx;
      mousey_ = my;
      redraw();
    } else if (Fl::event_button() == FL_LEFT_MOUSE &&
               (Fl::event_state() & FL_SHIFT)) {
      // Shift+left drag: draw line
      mousex_ = mx;
      mousey_ = my;
    }
    return 1;

  case FL_RELEASE:
    return 1;

  case FL_MOUSEWHEEL:
    // Mouse wheel for zoom
    {
      int dy = Fl::event_dy();
      if (dy != 0) {
        flag fChanged = fFalse;
        // Local Horizon, Telescope, and Orbit views use gs.rspace for zoom
        if (gi.nMode == gLocal || gi.nMode == gTelescope ||
            gi.nMode == gOrbit) {
          real r = gs.rspace;
          if (r < rSmall)
            r = (real)(1 << (4 - gi.nScale / gi.nScaleT));
          if (dy < 0)
            r /= 1.25; // Zoom in (scroll up)
          else
            r *= 1.25; // Zoom out (scroll down)
          if (FValidZoom(r)) {
            gs.rspace = r;
            fChanged = fTrue;
          }
        } else {
          // Other views use gs.nScale for zoom
          fChanged = FAdjustZoom(dy < 0 ? 1 : -1);
        }
        if (fChanged)
          redraw();
      }
    }
    return 1;

#ifdef __APPLE__
  case FL_ZOOM_GESTURE:
    // Trackpad pinch-to-zoom gesture (macOS)
    // event_dy() contains the magnification delta (positive = zoom in)
    {
      float mag = (float)Fl::event_dy();
      if (mag != 0.0f) {
        flag fChanged = fFalse;
        // Local Horizon, Telescope, and Orbit views use gs.rspace for zoom
        if (gi.nMode == gLocal || gi.nMode == gTelescope ||
            gi.nMode == gOrbit) {
          real r = gs.rspace;
          if (r < rSmall)
            r = (real)(1 << (4 - gi.nScale / gi.nScaleT));
          // Use same zoom factor as mousewheel but scaled by magnification
          // mag is typically -1.0 to 1.0 range for full pinch gesture
          if (mag > 0)
            r /= (1.0 + mag * 0.02); // Zoom in (pinch out)
          else
            r *= (1.0 - mag * 0.02); // Zoom out (pinch in)
          if (FValidZoom(r)) {
            gs.rspace = r;
            fChanged = fTrue;
          }
        } else {
          // Other views use gs.nScale for zoom
          // Accumulate magnification for discrete zoom steps
          static float accumMag = 0.0f;
          accumMag += mag;
          if (accumMag > 0.3f) {
            fChanged = FAdjustZoom(1); // Zoom in
            accumMag = 0.0f;
          } else if (accumMag < -0.3f) {
            fChanged = FAdjustZoom(-1); // Zoom out
            accumMag = 0.0f;
          }
        }
        if (fChanged)
          redraw();
      }
    }
    return 1;
#endif

  case FL_FOCUS:
  case FL_UNFOCUS:
    return 1;

  case FL_KEYDOWN:
    // Use event_text() to get the actual character typed (with Shift applied)
    // Fall back to event_key() for special keys (arrows, function keys, etc.)
    {
      const char *text = Fl::event_text();
      int key = (text && text[0] && !text[1]) ? (unsigned char)text[0]
                                              : Fl::event_key();
      // Alt+letter menu navigation - Windows/Linux style, disabled on macOS
      // where it doesn't work correctly with FLTK
#ifndef __APPLE__
      if (HandleAltMenuShortcut(key))
        return 1;
#endif
      return handleKey(key);
    }

  default:
    break;
  }

  return Fl_Widget::handle(event);
}

int ChartWidget::handleKey(int key) {
  // Let Cmd/Ctrl modified keys pass through to menu shortcuts
  if (Fl::event_state() & FL_COMMAND)
    return 0;

#ifdef EXPRESS
  // Allow AstroExpression to adjust the key
  if (!us.fExpOff && FSzSet(us.szExpKey)) {
    ExpSetN(iLetterZ, key);
    ParseExpression(us.szExpKey);
    key = NExpGet(iLetterZ);
  }
#endif

  switch (key) {
  // Basic controls
  case ' ':
  case 'p':
    inv(gi.fPause); // Spacebar or 'p' toggles pause/play
    return 1;

  case 'r':
    neg(gi.nDir);
    return 1;

  // Graphics toggles - delegate to menu callbacks
  case 'x':
    FMenuGraphicsReverse(NULL, NULL);
    return 1;
  case 'm':
    FMenuGraphicsMonochrome(NULL, NULL);
    return 1;
  case 't':
    FMenuGraphicsText(NULL, NULL);
    return 1;
  case 'i':
    FMenuGraphicsModify(NULL, NULL);
    return 1;
  case 'b':
    FMenuGraphicsBorder(NULL, NULL);
    return 1;
  case 'q':
    FMenuDashStyle(NULL, NULL);
    return 1;
  case 'l':
    FMenuGraphicsLabel(NULL, NULL);
    return 1;
  case 'k':
    FMenuGraphicsLabelAsp(NULL, NULL);
    return 1;
  case 'j':
    inv(gs.fJetTrail);
    return 1; // Timed exposure (no menu item)
  case 'd':
    FMenuGraphicsHouseExtra(NULL, NULL);
    return 1;
  case 'e':
    FMenuGraphicsEquator(NULL, NULL);
    return 1;

  case '=':
    inv(gs.fIndianWheel);
    redraw();
    return 1;

  case '0':
    inv(us.fPrimeVert);
    inv(us.fCalendarYear);
    inv(us.nEphemYears);
    inv(gs.fMollewide);
    gi.nMode = (gi.nMode == gWheel ? gHouse
                                   : (gi.nMode == gHouse ? gWheel : gi.nMode));
    redraw();
    return 1;

#ifdef CONSTEL
  case 'F':
    if (gi.nMode != gHorizon && gi.nMode != gSphere && gi.nMode != gGlobe &&
        gi.nMode != gPolar && gi.nMode != gTelescope)
      gi.nMode = gWorldMap;
    inv(gs.fConstel);
    redraw();
    return 1;
#endif

  // Calculation toggles
  case 'c':
    us.nRel = us.nRel ? rcNone : rcDual;
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 's':
    FMenuSidereal(NULL, NULL);
    return 1;
  case 'h':
    FMenuHeliocentric(NULL, NULL);
    return 1;

  case 'a':
    inv(us.fHouse3D);
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'g':
    inv(us.fDecan);
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'f':
    inv(us.fFlip);
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'z':
    inv(us.fIndian);
    redraw();
    return 1;

  // Object restrictions - delegate to menu callbacks
  case 'R':
    FMenuIncludeMinors(NULL, NULL);
    return 1;
  case 'C':
    FMenuIncludeCusps(NULL, NULL);
    return 1;
  case 'u':
    FMenuIncludeUranians(NULL, NULL);
    return 1;
  case 'y':
    FMenuIncludeDwarfs(NULL, NULL);
    return 1;
  case '`':
    FMenuIncludeMoons(NULL, NULL);
    return 1;
  case '~':
    FMenuIncludeCOB(NULL, NULL);
    return 1;
  case 'U':
    FMenuIncludeStars(NULL, NULL);
    return 1;

  // Scale adjustments - delegate to menu callbacks
  case '<':
    FMenuScaleDecrease(NULL, NULL);
    return 1;
  case '>':
    FMenuScaleIncrease(NULL, NULL);
    return 1;

  // Tilt controls
  case '[':
    gs.rTilt =
        gs.rTilt > -rDegQuad ? gs.rTilt - (real)NAbs(gi.nDir) : -rDegQuad;
    if (gi.nMode == gTelescope)
      gs.objTrack = -1;
    redraw();
    return 1;

  case ']':
    gs.rTilt = gs.rTilt < rDegQuad ? gs.rTilt + (real)NAbs(gi.nDir) : rDegQuad;
    if (gi.nMode == gTelescope)
      gs.objTrack = -1;
    redraw();
    return 1;

  // Rotation controls
  case '{':
    if (gi.nMode == gMidpoint || gi.nMode == gTelescope) {
      if (gi.nMode == gMidpoint && gs.objTrack >= 0)
        gs.rRot = planet[gs.objTrack];
      gs.objTrack = -1;
    }
    gs.rRot += (real)NAbs(gi.nDir);
    if (gs.rRot >= rDegMax)
      gs.rRot -= rDegMax;
    redraw();
    return 1;

  case '}':
    if (gi.nMode == gMidpoint || gi.nMode == gTelescope) {
      if (gi.nMode == gMidpoint && gs.objTrack >= 0)
        gs.rRot = planet[gs.objTrack];
      gs.objTrack = -1;
    }
    gs.rRot -= (real)NAbs(gi.nDir);
    if (gs.rRot < 0.0)
      gs.rRot += rDegMax;
    redraw();
    return 1;

  // Chart save/restore
  case 'o':
    ciSave = ciMain;
    return 1;

  case 'O':
    ciMain = ciSave;
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  // Animation step controls
  case '+':
    Animate(gs.nAnim, NAbs(gi.nDir));
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case '-':
    Animate(gs.nAnim, -NAbs(gi.nDir));
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  // Set current time
  case 'n':
    FMenuChartNow(NULL, NULL);
    return 1;

  // Toggle continuous animation
  case 'N':
    gs.nAnim = gs.nAnim ? 0 : -iAnimNow;
    return 1;

  // Animation modes (time units)
  case '!':
    gs.nAnim = -1;
    return 1; // Seconds
  case '@':
    gs.nAnim = -2;
    return 1; // Minutes
  case '#':
    gs.nAnim = -3;
    return 1; // Hours
  case '$':
    gs.nAnim = -4;
    return 1; // Days
  case '%':
    gs.nAnim = -5;
    return 1; // Months
  case '^':
    gs.nAnim = -6;
    return 1; // Years
  case '&':
    gs.nAnim = -7;
    return 1; // Decades
  case '*':
    gs.nAnim = -8;
    return 1; // Centuries
  case '(':
    gs.nAnim = -9;
    return 1; // Millennia

  // Chart modes (uppercase letters) - use menu functions for proper widget
  // switching
  case 'V':
    FMenuViewWheel(NULL, NULL);
    return 1;
  case 'A':
    FMenuViewGrid(NULL, NULL);
    return 1;
  case 'Z':
    FMenuViewHorizon(NULL, NULL);
    return 1;
  case 'S':
    FMenuViewOrbit(NULL, NULL);
    return 1;
  case 'H':
    gi.nMode = gSector;
    redraw();
    return 1;
  case 'K':
    gi.nMode = gCalendar;
    redraw();
    return 1;
  case 'J':
    gi.nMode = gDisposit;
    redraw();
    return 1;
  case 'L':
    FMenuViewAstroGraph(NULL, NULL);
    return 1;
  case 'E':
    gi.nMode = gEphemeris;
    redraw();
    return 1;
  case 'I':
    gi.nMode = gRising;
    redraw();
    return 1;
  case 'M':
    gi.nMode = gMoons;
    redraw();
    return 1;
  case 'X':
    FMenuViewSphere(NULL, NULL);
    return 1;
  case 'W':
    FMenuViewWorldMap(NULL, NULL);
    return 1;
  case 'G':
    FMenuViewGlobe(NULL, NULL);
    return 1;
  case 'P':
    FMenuViewPolar(NULL, NULL);
    return 1;
  case 'T':
    FMenuViewTelescope(NULL, NULL);
    return 1;
#ifdef BIORHYTHM
  case 'Y':
    us.nRel = rcBiorhythm;
    gi.nMode = gBiorhythm;
    fi.fDoCast = fTrue;
    redraw();
    return 1;
#endif

  case FL_Escape:
    if (fi.window)
      fi.window->hide();
    return 1;

  // Arrow key navigation
  case FL_Left:
    gs.rRot = Mod(gs.rRot + (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0));
    redraw();
    return 1;

  case FL_Right:
    gs.rRot = Mod(gs.rRot - (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0));
    redraw();
    return 1;

  case FL_Up:
    gs.rTilt += (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0);
    if (gs.rTilt > rDegQuad)
      gs.rTilt = rDegQuad;
    redraw();
    return 1;

  case FL_Down:
    gs.rTilt -= (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0);
    if (gs.rTilt < -rDegQuad)
      gs.rTilt = -rDegQuad;
    redraw();
    return 1;

  default:
    // Animation speed (1-9)
    if (key >= '1' && key <= '9') {
      gi.nDir = (gi.nDir > 0 ? 1 : -1) * (key - '0');
      return 1;
    }
    break;
  }

  return 0;
}

void ChartWidget::requestResize(int w, int h) {
  // Request parent to resize
  if (parent()) {
    // Calculate new window size accounting for menu bar
    int newW = w;
    int newH = h + y(); // y() is the offset from menu bar
    window()->resize(window()->x(), window()->y(), newW, newH);
  }
}

// Forward declarations for menu callbacks
void FMenuRestrict(Fl_Widget *w, void *data);
void FMenuRestrictTransit(Fl_Widget *w, void *data);
void FMenuShowTextWindow(Fl_Widget *w, void *data);
void FMenuColoredText(Fl_Widget *w, void *data);
void FMenuColorSettings(Fl_Widget *w, void *data);
void FMenuChartType(Fl_Widget *w, void *data);
void FMenuViewWheel(Fl_Widget *w, void *data);
void FMenuViewGrid(Fl_Widget *w, void *data);
void FMenuViewMidpoint(Fl_Widget *w, void *data);
void FMenuViewHorizon(Fl_Widget *w, void *data);
void FMenuViewOrbit(Fl_Widget *w, void *data);
void FMenuViewAstroGraph(Fl_Widget *w, void *data);
void FMenuViewGlobe(Fl_Widget *w, void *data);
void FMenuViewSphere(Fl_Widget *w, void *data);
void FMenuViewLocal(Fl_Widget *w, void *data);
void FMenuViewTelescope(Fl_Widget *w, void *data);
void FMenuViewPolar(Fl_Widget *w, void *data);
void FMenuViewWorldMap(Fl_Widget *w, void *data);
void FMenuAnimPause(Fl_Widget *w, void *data);
void FMenuAnimReverse(Fl_Widget *w, void *data);
void FMenuAnimForward(Fl_Widget *w, void *data);
void FMenuAnimBack(Fl_Widget *w, void *data);

/*
******************************************************************************
** AstrologWindow Implementation
******************************************************************************
*/

AstrologWindow::AstrologWindow(int w, int h, const char *title)
    : Fl_Double_Window(w, h + 25, title),
#ifdef OPENGL
      chart3D_(NULL),
#endif
      animating_(false) {
  // Set background to black to avoid white gaps during resize
  color(FL_BLACK);

  // Create menu bar
  menubar_ = new Fl_Menu_Bar(0, 0, w, 25);
  fi.menubar =
      menubar_; // Set global before createMenus so UpdateMenuCheck works
  // Create chart widget below menu bar (2D)
  chart_ = new ChartWidget(0, 25, w, h);
  fi.chart = chart_;

#ifdef OPENGL
  // Create OpenGL 3D widget (initially hidden)
  chart3D_ = new Globe3DWidget(0, 25, w, h);
  chart3D_->hide();
  fi.chart3D = chart3D_;
#endif

  createMenus();

  end();
  resizable(chart_);

  // Store in global state
  fi.window = this;
  // fi.chart already set
#ifdef OPENGL
  // fi.chart3D already set
#endif
  // fi.menubar already set before createMenus()
  fi.xClient = w;
  fi.yClient = h;

  // Start animation timer (runs continuously like Windows version)
  startAnimation();
}

AstrologWindow::~AstrologWindow() {
  stopAnimation();
  fi.window = NULL;
  fi.chart = NULL;
#ifdef OPENGL
  fi.chart3D = NULL;
#endif
  fi.menubar = NULL;
  if (fi.textWindow) {
    delete fi.textWindow;
    fi.textWindow = NULL;
  }
}

#ifdef OPENGL
void AstrologWindow::switchTo3D(bool use3D) {
  // Safety checks
  if (!chart_ || !chart3D_)
    return;

  // Ensure chart data is calculated before rendering
  if (fi.fDoCast) {
    fi.fDoCast = fFalse;
    ciCore = ciMain;
    if (us.nRel)
      CastRelation();
    else
      CastChart(0);
  }

  if (use3D) {
    // Switch to OpenGL 3D widget
    if (chart_->visible())
      chart_->hide();
    chart3D_->setChartMode(gi.nMode);
    if (!chart3D_->visible())
      chart3D_->show();
    chart3D_->redraw();
    resizable(chart3D_);
  } else {
    // Switch to 2D FLTK widget
    // Ensure OpenGL context is properly flushed before hiding
    if (chart3D_->visible()) {
      if (chart3D_->valid()) {
        chart3D_->make_current();
        glFinish();
      }
      chart3D_->hide();
    }
    if (!chart_->visible())
      chart_->show();
    chart_->redraw();
    resizable(chart_);
  }
}
#endif

// Deferred callback to square the window after resize completes
static void SquareWindowCallback(void *data) {
  if (!fSquare || !fi.window)
    return;

  int oldW = gs.xWin, oldH = gs.yWin;
  int menuH = 25;

  // Get screen work area (excludes menu bar, dock, etc.)
  int screenX, screenY, screenW, screenH;
  Fl::screen_work_area(screenX, screenY, screenW, screenH);

  // Calculate sidebar width if visible
  int sidebarW = (gs.fText && gs.fDoSidebar) ? xSideT : 0;

  // Calculate chart area (excluding sidebar)
  int chartW = gs.xWin - sidebarW;
  int chartH = gs.yWin;

  // Square the chart area to maximum dimension
  int maxChartDim = Max(chartW, chartH);

  // Limit to screen bounds (accounting for menu bar and sidebar)
  int maxAllowedW = screenW - sidebarW;
  int maxAllowedH = screenH - menuH;
  maxChartDim = Min(maxChartDim, Min(maxAllowedW, maxAllowedH));

  // Set new dimensions (add sidebar back to width)
  gs.xWin = maxChartDim + sidebarW;
  gs.yWin = maxChartDim;

  if (gs.xWin != oldW || gs.yWin != oldH) {
    fi.window->size(gs.xWin, gs.yWin + menuH);
  }
}

void AstrologWindow::resize(int x, int y, int w, int h) {
  Fl_Double_Window::resize(x, y, w, h);

  int menuH = 25;
  int clientW = w;
  int clientH = h - menuH;

  // Update global state
  fi.xClient = clientW;
  fi.yClient = clientH;
  gs.xWin = fi.xClient;
  gs.yWin = fi.yClient;
  gi.xWinResize = gs.xWin;
  gi.yWinResize = gs.yWin;

  // Schedule deferred squaring if auto-square is enabled
  if (fAutoSquare && fSquare) {
    // Cancel any pending callback and schedule a new one
    Fl::remove_timeout(SquareWindowCallback, this);
    Fl::add_timeout(0.0, SquareWindowCallback, this);
  }

  // Trigger redraw of chart widgets after resize
  if (chart_)
    chart_->redraw();
#ifdef OPENGL
  if (chart3D_ && chart3D_->visible())
    chart3D_->redraw();
#endif
  if (fi.textWindow && fi.textWindow->shown()) {
    RefreshTextWindow();
  }
}

int AstrologWindow::handle(int event) {
#ifdef __APPLE__
  // Handle trackpad pinch-to-zoom gesture (macOS sends this to window, not
  // widget)
  if (event == FL_ZOOM_GESTURE) {
    // Forward to chart widget's handle method
    if (chart_ && chart_->visible())
      return chart_->handle(event);
#ifdef OPENGL
    if (chart3D_ && chart3D_->visible())
      return chart3D_->handle(event);
#endif
  }
#endif
  return Fl_Double_Window::handle(event);
}

void AstrologWindow::timer_callback(void *data) {
  AstrologWindow *win = (AstrologWindow *)data;
  if (!win)
    return;

  // Only animate if animation mode is enabled and not paused
  if (gs.nAnim && !gi.fPause) {
    Animate(gs.nAnim, gi.nDir);

    // Redraw the appropriate chart widget
    if (win->chart_ && win->chart_->visible())
      win->chart_->redraw();
#ifdef OPENGL
    if (win->chart3D_ && win->chart3D_->visible())
      win->chart3D_->redraw();
#endif
    if (fi.textWindow && fi.textWindow->shown()) {
      RefreshTextWindow();
    }
  }

  // Always reschedule - timer runs continuously like Windows version
  Fl::repeat_timeout(1.0 / 30.0, timer_callback, data);
}

void AstrologWindow::startAnimation() {
  if (!animating_) {
    animating_ = true;
    Fl::add_timeout(1.0 / 30.0, timer_callback, this);
  }
}

void AstrologWindow::stopAnimation() {
  animating_ = false;
  Fl::remove_timeout(timer_callback, this);
}

void AstrologWindow::createMenus() {
#ifdef __APPLE__
  char menuLabelBuf[256]; // Buffer for stripping mnemonic characters
#endif
  // File menu
  // FL_COMMAND maps to Cmd on macOS, Ctrl on Windows/Linux
  menubar_->add(MENU_LABEL("&File/&Open Chart..."), FL_COMMAND + 'o',
                FMenuFileOpen);
  menubar_->add(MENU_LABEL("&File/Open Chart #&2..."), 0, FMenuFileOpenChart2);
  menubar_->add(MENU_LABEL("&File/&Save Chart..."), FL_COMMAND + 's',
                FMenuFileSave);
  menubar_->add(MENU_LABEL("&File/Save &As..."), 0, FMenuFileSaveAs);
  menubar_->add(MENU_LABEL("&File/Save Program Settin&gs..."), 0,
                FMenuFileSaveSettings);
  menubar_->add(MENU_LABEL("&File/Other Formats/Save Chart &List..."), 0,
                FMenuSaveChartList);
  menubar_->add(MENU_LABEL("&File/Other Formats/Save Chart &Exchange..."), 0,
                FMenuSaveAAF);
  menubar_->add(MENU_LABEL("&File/Other Formats/Save Chart &Quick*Chart..."), 0,
                FMenuSaveQuick);
#ifdef CAIRO
  menubar_->add(MENU_LABEL("&File/Export/&SVG..."), 0, FMenuExportSVG);
  menubar_->add(MENU_LABEL("&File/Export/&PDF..."), 0, FMenuExportPDF);
#endif
  menubar_->add(MENU_LABEL("&File/Export/&Bitmap..."), 0, FMenuExportBitmap);
  menubar_->add(MENU_LABEL("&File/Export/Chart &Text Output..."), 0,
                FMenuExportText);
  menubar_->add(MENU_LABEL("&File/Open Bitmap/Open Chart &Background..."), 0,
                FMenuOpenBackground);
  menubar_->add(MENU_LABEL("&File/Open Bitmap/Open &World Map..."), 0,
                FMenuOpenWorldMap, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&File/&Print..."), FL_COMMAND + 'p', FMenuFilePrint,
                0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&File/E&xit"), FL_COMMAND + 'q', FMenuFileExit);

  // Edit menu
  menubar_->add(MENU_LABEL("&Edit/&Copy Chart Text Output"), FL_COMMAND + 'c',
                FMenuCopyText);
  menubar_->add(MENU_LABEL("&Edit/&Paste"), FL_COMMAND + 'v',
                (Fl_Callback *)NULL);
  menubar_->add(MENU_LABEL("&Edit/Command &Line..."), FL_F + 2, FMenuCommand, 0,
                FL_MENU_DIVIDER);

  // Run Macro submenus (F1-F12 with modifiers)
  // Normal set (F1-F12)
  for (int i = 1; i <= 12; i++) {
    char szMenu[64];
    sprintf(szMenu, "&Edit/Run Macro (&Normal Set)/Macro &%d", i);
    menubar_->add(szMenu, FL_F + i, FMenuMacro, (void *)(long)i);
  }
  // Shift set (Shift+F1-F12 = macros 13-24)
  for (int i = 1; i <= 12; i++) {
    char szMenu[64];
    sprintf(szMenu, "&Edit/Run Macro (&Shift Set)/Macro &%d", i + 12);
    menubar_->add(szMenu, FL_SHIFT + FL_F + i, FMenuMacro,
                  (void *)(long)(i + 12));
  }
  // Command/Ctrl set (Cmd+F1-F12 = macros 25-36)
  for (int i = 1; i <= 12; i++) {
    char szMenu[64];
    sprintf(szMenu, "&Edit/Run Macro (&Command Set)/Macro &%d", i + 24);
    menubar_->add(szMenu, FL_COMMAND + FL_F + i, FMenuMacro,
                  (void *)(long)(i + 24));
  }
  // Alt/Option set (Alt+F1-F12 = macros 37-48)
  for (int i = 1; i <= 12; i++) {
    char szMenu[64];
    sprintf(szMenu, "&Edit/Run Macro (&Alt Set)/Macro &%d", i + 36);
    menubar_->add(szMenu, FL_ALT + FL_F + i, FMenuMacro,
                  (void *)(long)(i + 36));
  }

  // Info menu
  menubar_->add(MENU_LABEL("&Info/Set &Chart Info..."), FL_COMMAND + 'i',
                FMenuInfoChart);
  menubar_->add(MENU_LABEL("&Info/Chart for &Now"), 'n', FMenuChartNow);
  menubar_->add(MENU_LABEL("&Info/Set Chart #&2 Info..."), 0, FMenuInfoChart2);
  menubar_->add(MENU_LABEL("&Info/Charts #&3 Through #6..."), 0, FMenuInfoAll);
  menubar_->add(MENU_LABEL("&Info/&Default Chart Info..."), 0,
                FMenuDefaultInfo);
  menubar_->add(MENU_LABEL("&Info/Swap Chart #1 and #&2"), 'x', FMenuSwapCharts,
                0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Info/Chart List/Chart &List..."), 0, FMenuListDlg);
  menubar_->add(MENU_LABEL("&Info/Chart List/&Previous Chart"),
                FL_SHIFT + FL_Up, FMenuListPrev);
  menubar_->add(MENU_LABEL("&Info/Chart List/&Next Chart"), FL_SHIFT + FL_Down,
                FMenuListNext);
  menubar_->add(MENU_LABEL("&Info/Chart List/&First Chart"), FL_COMMAND + FL_Up,
                FMenuListFirst);
  menubar_->add(MENU_LABEL("&Info/Chart List/&Last Chart"),
                FL_COMMAND + FL_Down, FMenuListLast, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Info/Relationship/No &Relationship Chart"), 'c',
                FMenuRelNo);
  menubar_->add(MENU_LABEL("&Info/Relationship/Com&parison Chart"), 0,
                FMenuRelNo);
  menubar_->add(MENU_LABEL("&Info/Relationship/&Synastry Chart"), 0,
                FMenuRelSynastry);
  menubar_->add(MENU_LABEL("&Info/Relationship/&Composite Chart"), 0,
                FMenuRelComposite);
  menubar_->add(MENU_LABEL("&Info/Relationship/Time Space &Midpoint Chart"), 0,
                FMenuRelMidpoint, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Info/Relationship/Date &Difference Chart"), 'D',
                FMenuRelDate);
#ifdef BIORHYTHM
  menubar_->add(MENU_LABEL("&Info/Relationship/&Biorhythm Chart"), 'Y',
                FMenuRelBiorhythm);
#endif
  menubar_->add(MENU_LABEL("&Info/Relationship/&Transit and Natal"), 0,
                FMenuRelTransit);
  menubar_->add(MENU_LABEL("&Info/Relationship/&Progressed and Natal"), 0,
                FMenuRelProgressed);

  // View menu - display settings only (matching Windows)
  menubar_->add(MENU_LABEL("&View/Show &Graphics"), 'v', FMenuGraphicsToggle, 0,
                FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&View/Show &Text Window"), FL_COMMAND + 't',
                FMenuShowTextWindow, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&View/Window Settings/&Redraw Screen"), ' ',
                FMenuRedraw);
  menubar_->add(MENU_LABEL("&View/Window Settings/&Clear Screen"), FL_Delete,
                FMenuClear, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&View/Window Settings/&Full Screen"), FL_F + 11,
                FMenuFullScreen, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&View/&Colored Text"), FL_ALT + 'k',
                FMenuColoredText, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&View/&Set Colors..."), FL_ALT + FL_SHIFT + 'k',
                FMenuColorSettings, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&View/Show &Interpretations"), 0, FMenuInterpret, 0,
                FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&View/Print &Nearest Second"), 0, FMenuSecond, 0,
                FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&View/&Parallel Aspects"), 0, FMenuParallel, 0,
                FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&View/&Applying Aspects"), 0, FMenuApplying, 0,
                FL_MENU_TOGGLE);

  // Chart menu - chart types and dialogs (matching Windows)
  menubar_->add(MENU_LABEL("C&hart/Standard Radi&x"), 'V', FMenuViewList);
  menubar_->add(MENU_LABEL("C&hart/House &Wheel"), 0, FMenuViewHouse);
  menubar_->add(MENU_LABEL("C&hart/&Aspect Midpoint Grid"), 'A', FMenuViewGrid);
  menubar_->add(MENU_LABEL("C&hart/Aspec&t List"), 0, FMenuViewAspect);
  menubar_->add(MENU_LABEL("C&hart/&Midpoint List"), 0, FMenuViewMidpoint);
  menubar_->add(MENU_LABEL("C&hart/Local Hori&zon"), 'Z', FMenuViewHorizon);
  menubar_->add(MENU_LABEL("C&hart/Solar System &Orbit"), 'S', FMenuViewOrbit);
  menubar_->add(MENU_LABEL("C&hart/Ga&uquelin Sectors"), 'H', FMenuViewSector);
  menubar_->add(MENU_LABEL("C&hart/&Calendar"), 'K', FMenuViewCalendar);
  menubar_->add(MENU_LABEL("C&hart/Inf&luence"), 'J', FMenuViewInfluence);
  menubar_->add(MENU_LABEL("C&hart/Esoter&ic"), 0, FMenuViewEsoteric);
  menubar_->add(MENU_LABEL("C&hart/Astro-Grap&h"), 'L', FMenuViewAstroGraph);
  menubar_->add(MENU_LABEL("C&hart/&Ephemeris"), 'E', FMenuViewEphemeris);
#ifdef ARABIC
  menubar_->add(MENU_LABEL("C&hart/A&rabic Parts"), 0, FMenuViewArabic);
#endif
  menubar_->add(MENU_LABEL("C&hart/R&ising and Setting"), 'I', FMenuViewRising);
  menubar_->add(MENU_LABEL("C&hart/&Nearest Cities"), 0, FMenuViewNearestCity);
  menubar_->add(MENU_LABEL("C&hart/&Moons Chart"), 'M', FMenuViewMoons, 0,
                FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("C&hart/&Transits..."), 0, FMenuTransitDlg);
  menubar_->add(MENU_LABEL("C&hart/Pro&gressions..."), 0, FMenuProgressDlg, 0,
                FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("C&hart/Chart Sett&ings..."), 0,
                FMenuChartSettingsDlg);
  menubar_->add(MENU_LABEL("C&hart/Chart &Type..."), 0, FMenuChartType);

  // Settings menu
  menubar_->add(MENU_LABEL("Se&ttings/&Sidereal Zodiac"), 's', FMenuSidereal, 0,
                FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/&Heliocentric"), 'h', FMenuHeliocentric,
                0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  // House System submenu
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Placidus"),
                FL_COMMAND + 'p', FMenuHouseSystem, (void *)hsPlacidus);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Koch"), FL_COMMAND + 'k',
                FMenuHouseSystem, (void *)hsKoch);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Campanus"), 0,
                FMenuHouseSystem, (void *)hsCampanus);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Regiomontanus"),
                FL_COMMAND + 'r', FMenuHouseSystem, (void *)hsRegiomontanus);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Topocentric"),
                FL_COMMAND + 't', FMenuHouseSystem, (void *)hsTopocentric);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Alca&bitius"), 0,
                FMenuHouseSystem, (void *)hsAlcabitius);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Kr&usinski"), 0,
                FMenuHouseSystem, (void *)hsKrusinski);
  menubar_->add(MENU_LABEL("Se&ttings/House System/A&.P.C."), 0,
                FMenuHouseSystem, (void *)hsAPC, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Porph&yry"),
                FL_COMMAND + 'y', FMenuHouseSystem, (void *)hsPorphyry);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Pullen (S.Rati&o)"),
                FL_COMMAND + 'o', FMenuHouseSystem, (void *)hsSineRatio);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Pullen (S.&Delta)"),
                FL_COMMAND + 'd', FMenuHouseSystem, (void *)hsSineDelta,
                FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Meridian"),
                FL_COMMAND + 'm', FMenuHouseSystem, (void *)hsMeridian);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Morinu&s"), FL_COMMAND + 'u',
                FMenuHouseSystem, (void *)hsMorinus);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Hori&zon"), FL_COMMAND + 'h',
                FMenuHouseSystem, (void *)hsHorizon);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Carter& P.Equat."),
                FL_COMMAND + 'g', FMenuHouseSystem, (void *)hsCarter);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Suns&hine"),
                FL_COMMAND + 'j', FMenuHouseSystem, (void *)hsSunshine);
  menubar_->add(MENU_LABEL("Se&ttings/House System/Sr&ipati"), 0,
                FMenuHouseSystem, (void *)hsSripati, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Equal"), FL_COMMAND + 'e',
                FMenuHouseSystem, (void *)hsEqual);
  menubar_->add(MENU_LABEL("Se&ttings/House System/E&qual (MC)"),
                FL_COMMAND + 'q', FMenuHouseSystem, (void *)hsEqualMC);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Whole"), FL_COMMAND + 'w',
                FMenuHouseSystem, (void *)hsWhole);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Vedic"), FL_COMMAND + 'v',
                FMenuHouseSystem, (void *)hsVedic);
  menubar_->add(MENU_LABEL("Se&ttings/House System/&Null"), FL_COMMAND + 'n',
                FMenuHouseSystem, (void *)hsNull, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/&Solar Chart"), 0,
                FMenuHouseSolar, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/&3D Houses"), 'a',
                FMenuHouse3D, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/Show &Decans"), 'g',
                FMenuHouseDecan, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/Show D&wads"), 0,
                FMenuHouseSetDwad, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/&Flip Signs with Houses"),
                'f', FMenuHouseFlip, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/&Geodetic Houses"), 0,
                FMenuHouseSetGeodetic, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/&Indian Wheel Order"), 'z',
                FMenuHouseSetIndian, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/House Settings/Show &Navamsas"), 0,
                FMenuHouseSetNavamsa, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/&Calculation Settings..."), 0,
                FMenuCalcSettings);
  menubar_->add(MENU_LABEL("Se&ttings/&Display Settings..."), 0,
                FMenuDisplaySettings);
  menubar_->add(MENU_LABEL("Se&ttings/&Graphics Settings..."), 0,
                FMenuGraphicsSettings);
  menubar_->add(MENU_LABEL("Se&ttings/&Aspect Settings..."), 0,
                FMenuAspectSettings);
  menubar_->add(MENU_LABEL("Se&ttings/&Object Restrictions..."), 0,
                FMenuRestrict);
  menubar_->add(MENU_LABEL("Se&ttings/&Transit Restrictions..."), 0,
                FMenuRestrictTransit);
  menubar_->add(MENU_LABEL("Se&ttings/Star Restric&tions..."), 0,
                FMenuStarRestrict);
  menubar_->add(MENU_LABEL("Se&ttings/Object Sett&ings..."), 0,
                FMenuObjectSettings);
  menubar_->add(MENU_LABEL("Se&ttings/&More Object Settings..."), 0,
                FMenuObjectSettings2);
  menubar_->add(MENU_LABEL("Se&ttings/Co&lor Settings..."), 0,
                FMenuColorSettings, 0, FL_MENU_DIVIDER);
  // Glyph Fonts submenu
  menubar_->add(MENU_LABEL("Se&ttings/Glyph &Fonts/Astrono&micon"), 0,
                FMenuGlyphFont, (void *)5, FL_MENU_RADIO | FL_MENU_VALUE);
  menubar_->add(MENU_LABEL("Se&ttings/Glyph &Fonts/&Astro"), 0, FMenuGlyphFont,
                (void *)2, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("Se&ttings/Glyph &Fonts/&Enigma"), 0, FMenuGlyphFont,
                (void *)3, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("Se&ttings/Glyph &Fonts/&Hamburg"), 0,
                FMenuGlyphFont, (void *)4, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("Se&ttings/Glyph &Fonts/&Built-in"), 0,
                FMenuGlyphFont, (void *)0, FL_MENU_RADIO | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/Include &Minors"), 'R',
                FMenuIncludeMinors, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/Include &Cusps"), 'C', FMenuIncludeCusps,
                0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/Include &Uranians"), 'u',
                FMenuIncludeUranians, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/Include &Dwarfs"), 'y',
                FMenuIncludeDwarfs, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/Include Fixed &Stars"), 'U',
                FMenuIncludeStars, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("Se&ttings/Planetary Moons/&Moons Chart"), 'M',
                FMenuViewMoons);
  menubar_->add(MENU_LABEL("Se&ttings/Planetary Moons/&Exoplanets Chart"), 0,
                FMenuViewExo, 0, FL_MENU_DIVIDER);
  menubar_->add(
      MENU_LABEL("Se&ttings/Planetary Moons/Moon &Object Settings..."), 0,
      FMenuMoonObjSettings);
  menubar_->add(MENU_LABEL("Se&ttings/Planetary Moons/Moon &Restrictions..."),
                0, FMenuMoonRestrict);
  menubar_->add(
      MENU_LABEL("Se&ttings/Planetary Moons/Object &Customization..."), 0,
      FMenuObjCustom);
  menubar_->add(MENU_LABEL("Se&ttings/Planetary Moons/Star Custo&mization..."),
                0, FMenuStarCustom, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("Se&ttings/Planetary Moons/Include Moo&ns"), '`',
                FMenuIncludeMoons, 0, FL_MENU_TOGGLE);
  menubar_->add(
      MENU_LABEL("Se&ttings/Planetary Moons/Include &Body Centers (COB)"), '~',
      FMenuIncludeCOB, 0, FL_MENU_TOGGLE);

  // Graphics menu - 3D modes first (matching Windows)
  menubar_->add(MENU_LABEL("&Graphics/Draw Chart S&phere"), 'X',
                FMenuViewSphere);
  menubar_->add(MENU_LABEL("&Graphics/Draw &World Map"), 'W',
                FMenuViewWorldMap);
  menubar_->add(MENU_LABEL("&Graphics/Draw &Globe"), 'G', FMenuViewGlobe);
  menubar_->add(MENU_LABEL("&Graphics/Draw &Polar Globe"), 'P', FMenuViewPolar);
  menubar_->add(MENU_LABEL("&Graphics/Draw &Telescope"), 'T',
                FMenuViewTelescope);
  menubar_->add(MENU_LABEL("&Graphics/Draw Local &3D Horizon"), 0,
                FMenuViewLocal, 0, FL_MENU_DIVIDER);
#ifdef OPENGL
  menubar_->add(MENU_LABEL("&Graphics/3D with &OpenGL"), 0, FMenuViewOpenGL, 0,
                FL_MENU_TOGGLE | FL_MENU_VALUE | FL_MENU_DIVIDER);
#endif
  menubar_->add(MENU_LABEL("&Graphics/&Reverse Background"), 'x',
                FMenuGraphicsReverse, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/&Monochrome"), 'm',
                FMenuGraphicsMonochrome, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Reduce Contrast/&None (0%)"), 0,
                FMenuReduceContrast, (void *)0, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Reduce Contrast/&Light (25%)"), 0,
                FMenuReduceContrast, (void *)25, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Reduce Contrast/&Medium (50%)"), 0,
                FMenuReduceContrast, (void *)50, FL_MENU_RADIO | FL_MENU_VALUE);
  menubar_->add(MENU_LABEL("&Graphics/Reduce Contrast/&Heavy (75%)"), 0,
                FMenuReduceContrast, (void *)75, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Auto S&quare Window"), 0,
                FMenuGraphicsAutoSquare, 0, FL_MENU_TOGGLE);
  // Character Scale submenu
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/&Small"), 0, FMenuScale1,
                0, FL_MENU_RADIO | FL_MENU_VALUE);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/&Medium"), 0, FMenuScale2,
                0, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/&Large"), 0, FMenuScale3,
                0, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/&Huge"), 0, FMenuScale4,
                0, FL_MENU_RADIO | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/&Decrease"), '<',
                FMenuScaleDecrease);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/&Increase"), '>',
                FMenuScaleIncrease, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/Decrease &Text"), 0,
                FMenuTextDecrease);
  menubar_->add(MENU_LABEL("&Graphics/Character Scale/Increase Te&xt"), 0,
                FMenuTextIncrease);
  // Chart Effects submenu
  menubar_->add(MENU_LABEL("&Graphics/Chart Effects/Show &Border"), 'b',
                FMenuGraphicsBorder, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Chart Effects/Show Chart &Info"), 't',
                FMenuGraphicsText, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Chart Effects/Show Info &Sidebar"), 0,
                FMenuGraphicsSidebar, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Chart Effects/&Thicker Lines"), 0,
                FMenuGraphicsThick, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Chart Effects/&Transparent Aspect Lines"),
                'q', FMenuDashStyle, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Chart Effects/Show Glyph &Labels"), 'l',
                FMenuGraphicsLabel, 0, FL_MENU_TOGGLE);
  menubar_->add(
      MENU_LABEL("&Graphics/Chart Effects/Show &Glyphs on Aspect Lines"), 'k',
      FMenuGraphicsLabelAsp, 0, FL_MENU_TOGGLE);
  // Wheel Fill submenu
  menubar_->add(MENU_LABEL("&Graphics/Wheel Fill/&None"), 0, FMenuWheelFill,
                (void *)0, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Wheel Fill/&Solid Color"), 0,
                FMenuWheelFill, (void *)1, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Wheel Fill/&Rainbow Gradient"), 0,
                FMenuWheelFill, (void *)2, FL_MENU_RADIO | FL_MENU_VALUE);
  menubar_->add(MENU_LABEL("&Graphics/Wheel Fill/Rainbow Gradient &2"), 0,
                FMenuWheelFill, (void *)3, FL_MENU_RADIO | FL_MENU_DIVIDER);
  // Map Effects submenu
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Show &Constellations"), 'F',
                FMenuMapConstel, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Show Full &Star List"), 0,
                FMenuMapAllStar, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Show E&xoplanets"), 0,
                FMenuMapExo, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Show &House Details"), 'd',
                FMenuGraphicsHouseExtra, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Show &Equator"), 'e',
                FMenuGraphicsEquator, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Show C&ities"), 0,
                FMenuMapCity, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Use Detailed World &Map"),
                'w', FMenuMapBmp, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Map Effects/Use Ecliptic &Axis"), 0,
                FMenuMapAxis, 0, FL_MENU_TOGGLE);
  // Map Orientation submenu
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Set Tilt to &Zero"), 0,
                FMenuTiltZero, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Tilt &North"), '[',
                FMenuTiltNorth);
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Tilt &South"), ']',
                FMenuTiltSouth, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Rotate &West"), '{',
                FMenuRotateWest);
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Rotate &East"), '}',
                FMenuRotateEast, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Zoom &Out"), 0,
                FMenuZoomOut);
  menubar_->add(MENU_LABEL("&Graphics/Map Orientation/Zoom &In"), 0,
                FMenuZoomIn);
  // Indian Style Charts submenu
  menubar_->add(MENU_LABEL("&Graphics/Indian Style/Show &Indian Wheels"), '=',
                FMenuIndian, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Indian Style/&South Indian"), 0,
                FMenuIndianS, 0, FL_MENU_RADIO | FL_MENU_VALUE);
  menubar_->add(MENU_LABEL("&Graphics/Indian Style/&North Indian"), 0,
                FMenuIndianN, 0, FL_MENU_RADIO);
  menubar_->add(MENU_LABEL("&Graphics/Indian Style/&East Indian"), 0,
                FMenuIndianE, 0, FL_MENU_RADIO | FL_MENU_DIVIDER);

  // Pen/Scribble Color submenu
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/Blac&k"), FL_COMMAND + 'z',
                FMenuPenColor, (void *)kBlack);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&White"), FL_COMMAND + 'a',
                FMenuPenColor, (void *)kWhite);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Red"), FL_COMMAND + 'r',
                FMenuPenColor, (void *)kRed);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Green"), FL_COMMAND + 'g',
                FMenuPenColor, (void *)kGreen);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Blue"), FL_COMMAND + 'b',
                FMenuPenColor, (void *)kBlue);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Yellow"), FL_COMMAND + 'y',
                FMenuPenColor, (void *)kYellow);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Magenta"), FL_COMMAND + 'm',
                FMenuPenColor, (void *)kMagenta);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Cyan"), FL_COMMAND + 'j',
                FMenuPenColor, (void *)kCyan, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/Gr&ay"), FL_COMMAND + 'd',
                FMenuPenColor, (void *)kDkGray);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Lt. Gray"), FL_COMMAND + 'l',
                FMenuPenColor, (void *)kLtGray);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/Maroo&n"), FL_COMMAND + 'e',
                FMenuPenColor, (void *)kMaroon);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/Dk. Gr&een"), FL_COMMAND + 'f',
                FMenuPenColor, (void *)kDkGreen);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/Dk. Bl&ue"), FL_COMMAND + 'n',
                FMenuPenColor, (void *)kDkBlue);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Orange"), FL_COMMAND + 'o',
                FMenuPenColor, (void *)kOrange);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Purple"), FL_COMMAND + 'u',
                FMenuPenColor, (void *)kPurple);
  menubar_->add(MENU_LABEL("&Graphics/Pen Color/&Dk. Cyan"), FL_COMMAND + 'k',
                FMenuPenColor, (void *)kDkCyan);

  menubar_->add(MENU_LABEL("&Graphics/Modify &Display"), 'i',
                FMenuGraphicsModify, 0, FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/Modif&y Chart"), '0', FMenuModifyChart, 0,
                FL_MENU_TOGGLE);
  menubar_->add(MENU_LABEL("&Graphics/&Graphics Settings..."), 0,
                FMenuGraphicsSettings);

  // Animate menu
  menubar_->add(MENU_LABEL("&Animate/Do &Animation"), 'N', FMenuAnimToggle);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/Update to &Now"), 0,
                FMenuAnimNow, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Seconds"), '!', FMenuAnimRate1);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Minutes"), '@', FMenuAnimRate2);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Hours"), '#', FMenuAnimRate3);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Days"), '$', FMenuAnimRate4);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/M&onths"), '%', FMenuAnimRate5);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Years"), '^', FMenuAnimRate6);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Decades"), '&', FMenuAnimRate7);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/&Centuries"), '*',
                FMenuAnimRate8);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/Mi&llennia"), '(',
                FMenuAnimRate9, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/1\\/&10th Seconds"), 0,
                FMenuAnimRate_11);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/1\\/1&00th Seconds"), 0,
                FMenuAnimRate_12);
  menubar_->add(MENU_LABEL("&Animate/Jump Rate/1\\/100&0th Seconds"), 0,
                FMenuAnimRate_13);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&1 Unit"), '1',
                FMenuAnimFactor1);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&2 Units"), '2',
                FMenuAnimFactor2);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&3 Units"), '3',
                FMenuAnimFactor3);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&4 Units"), '4',
                FMenuAnimFactor4);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&5 Units"), '5',
                FMenuAnimFactor5);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&6 Units"), '6',
                FMenuAnimFactor6);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&7 Units"), '7',
                FMenuAnimFactor7);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&8 Units"), '8',
                FMenuAnimFactor8);
  menubar_->add(MENU_LABEL("&Animate/Jump Factor/&9 Units"), '9',
                FMenuAnimFactor9);
  menubar_->add(MENU_LABEL("&Animate/&Reverse Direction"), 'r',
                FMenuAnimReverse);
  menubar_->add(MENU_LABEL("&Animate/&Pause Animation"), 'p', FMenuAnimPause);
  menubar_->add(MENU_LABEL("&Animate/&Timed Exposure"), 'j',
                FMenuAnimTimedExposure, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Animate/Step &Forward"), '+', FMenuAnimForward);
  menubar_->add(MENU_LABEL("&Animate/Step &Backward"), '-', FMenuAnimBack);
  menubar_->add(MENU_LABEL("&Animate/&Store Chart Info"), 'o', FMenuStore);
  menubar_->add(MENU_LABEL("&Animate/Re&call Chart Info"), 'O', FMenuRecall, 0,
                FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Animate/Animation &Settings..."), 0,
                FMenuAnimSettings);

  // Help menu
  menubar_->add(MENU_LABEL("&Help/&About Astrolog..."), 0, FMenuHelpAbout);
  menubar_->add(MENU_LABEL("&Help/Open &Website"), 0, FMenuHelpWebsite);
  menubar_->add(MENU_LABEL("&Help/Open &Changes Log"), 0, FMenuHelpChanges);
  menubar_->add(MENU_LABEL("&Help/Show &License"), 0, FMenuHelpLicense, 0,
                FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Help/Open Data Files/Open &Default Settings"), 0,
                FMenuDocDefault);
#ifdef ATLAS
  menubar_->add(MENU_LABEL("&Help/Open Data Files/Open &Atlas"), 0,
                FMenuDocAtlas);
  menubar_->add(MENU_LABEL("&Help/Open Data Files/Open &Time Zone Changes"), 0,
                FMenuDocTimezone);
#endif
  menubar_->add(MENU_LABEL("&Help/Open Data Files/Open &Star List"), 0,
                FMenuDocStar);
  menubar_->add(MENU_LABEL("&Help/Open Data Files/Open &Orbital Elements"), 0,
                FMenuDocOrbit);
  menubar_->add(MENU_LABEL("&Help/Open Data Files/Open &Exoplanet List"), 0,
                FMenuDocExo, 0, FL_MENU_DIVIDER);
  menubar_->add(MENU_LABEL("&Help/List Si&gns"), 0, FMenuHelpSign);
  menubar_->add(MENU_LABEL("&Help/List &Objects"), 0, FMenuHelpObject);
  menubar_->add(MENU_LABEL("&Help/List Aspec&ts"), 0, FMenuHelpAspect);
#ifdef CONSTEL
  menubar_->add(MENU_LABEL("&Help/List &Constellations"), 0, FMenuHelpConstel);
#endif
  menubar_->add(MENU_LABEL("&Help/List &Planet Info"), 0, FMenuHelpPlanet);
  menubar_->add(MENU_LABEL("&Help/List &Rays"), 0, FMenuHelpRay);
#ifdef INTERPRET
  menubar_->add(MENU_LABEL("&Help/List &General Meanings"), 0,
                FMenuHelpMeaning);
#endif
  menubar_->add(MENU_LABEL("&Help/List S&witches"), 0, FMenuHelpSwitch);
  menubar_->add(MENU_LABEL("&Help/List O&bscure Switches"), 0,
                FMenuHelpObscure);
  menubar_->add(MENU_LABEL("&Help/List &Keystrokes"), '?', FMenuHelpKeystroke);
  menubar_->add(MENU_LABEL("&Help/List Cr&edits"), 0, FMenuHelpCredit);

  // Initialize menu checkbox states based on current settings
  flag fMinors = fFalse;
  for (int i = oChi; i <= oVes && !fMinors; i++)
    fMinors = !ignore[i];
  UpdateMenuCheck(FMenuIncludeMinors, fMinors);
  UpdateMenuCheck(FMenuIncludeCusps, us.fCusp);
  UpdateMenuCheck(FMenuIncludeUranians, us.fUranian);
  UpdateMenuCheck(FMenuIncludeDwarfs, us.fDwarf);
  UpdateMenuCheck(FMenuIncludeMoons, us.fMoons);
  UpdateMenuCheck(FMenuIncludeCOB, us.fCOB);
  UpdateMenuCheck(FMenuIncludeStars, us.fStar);
  UpdateMenuCheck(FMenuGraphicsReverse, gs.fInverse);
  UpdateMenuCheck(FMenuGraphicsMonochrome, !gs.fColor);
  UpdateMenuCheck(FMenuGraphicsBorder, gs.fBorder);
  UpdateMenuCheck(FMenuGraphicsText, gs.fText);
  UpdateMenuCheck(FMenuGraphicsThick, gs.fThick);
  UpdateMenuCheck(FMenuGraphicsLabel, gs.fLabel);
  UpdateMenuCheck(FMenuGraphicsLabelAsp, gs.fLabelAsp);
  UpdateMenuCheck(FMenuGraphicsModify, gs.fAlt);
  UpdateMenuCheck(FMenuGraphicsHouseExtra, gs.fHouseExtra);
  UpdateMenuCheck(FMenuGraphicsEquator, gs.fEquator);
  UpdateMenuCheck(FMenuColoredText, us.fAnsiColor);
  UpdateMenuCheck(FMenuShowTextWindow, fi.textWindow && fi.textWindow->shown());
  UpdateMenuCheck(FMenuInterpret, us.fInterpret);
  UpdateMenuCheck(FMenuSecond, us.fSeconds);
  UpdateMenuCheck(FMenuParallel, us.fParallel);
  UpdateMenuCheck(FMenuApplying, us.nAppSep == 1);
  UpdateMenuCheck(FMenuSidereal, us.fSidereal);
  UpdateMenuCheck(FMenuHeliocentric, us.objCenter != oEar);
  UpdateMenuCheck(FMenuHouseSolar, us.objOnAsc != 0);
  UpdateMenuCheck(FMenuHouse3D, us.fHouse3D);
  UpdateMenuCheck(FMenuHouseDecan, us.fDecan);
  UpdateMenuCheck(FMenuHouseSetDwad, us.nDwad > 0);
  UpdateMenuCheck(FMenuHouseFlip, us.fFlip);
  UpdateMenuCheck(FMenuHouseSetGeodetic, us.fGeodetic);
  UpdateMenuCheck(FMenuHouseSetIndian, us.fIndian);
  UpdateMenuCheck(FMenuHouseSetNavamsa, us.fNavamsa);
  UpdateMenuCheck(FMenuGraphicsSidebar, gs.fDoSidebar);
  UpdateMenuCheck(FMenuAnimTimedExposure, gs.fJetTrail);
  UpdateMenuCheck(FMenuIndian, gs.fIndianWheel);
  UpdateMenuCheck(FMenuMapConstel, gs.fConstel);
  UpdateMenuCheck(FMenuMapAllStar, gs.fAllStar);
  UpdateMenuCheck(FMenuMapExo, gs.fAllExo);
  UpdateMenuCheck(FMenuMapCity, gs.fLabelCity);
  UpdateMenuCheck(FMenuMapBmp, gi.fBmp);
  UpdateMenuCheck(FMenuMapAxis, gs.fEcliptic);

  // Initialize radio button states
  UpdateMenuRadioByValue(FMenuReduceContrast, gs.nReduceContrast);
  // Initialize glyph font menu
  // For glyph font, use nFontSig as representative (all are set to same value)
  UpdateMenuRadioByValue(FMenuGlyphFont, gs.nFontSig);
  // Initialize wheel fill menu
  UpdateMenuRadioByValue(FMenuWheelFill, gs.nDecaFill);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuColoredText(Fl_Widget *w, void *data) {
  us.fAnsiColor = !us.fAnsiColor;
  UpdateMenuCheck(FMenuColoredText, us.fAnsiColor);
  if (fi.textWindow && fi.textWindow->shown()) {
    RefreshTextWindow();
  }
}
/*
******************************************************************************
** Menu Callbacks
******************************************************************************
*/

void FMenuFileOpen(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Chart Files (*.as)\tAll Files (*)",
                          Fl_File_Chooser::SINGLE, "Open Chart");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Load the chart file
    FInputData(chooser.value());
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuFileSave(Fl_Widget *w, void *data) {
  // Would implement save functionality
}

void FMenuFileSaveAs(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Chart Files (*.as)\tAll Files (*)",
                          Fl_File_Chooser::CREATE, "Save Chart As");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Save the chart file
    // FOutputData() or similar
  }
}

#ifdef CAIRO
void FMenuExportSVG(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "SVG Files (*.svg)", Fl_File_Chooser::CREATE,
                          "Export as SVG");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Create SVG surface
    cairo_surface_t *surface =
        cairo_svg_surface_create(chooser.value(), gs.xWin, gs.yWin);

    if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
      // Switch to Cairo backend and render
      InitBackendCairo(surface);
      GBClearScreen(gi.kiOff);
      gi.fFile = fFalse; // Use screen rendering path
      DrawChartX();
      EndBackendCairo();

      cairo_surface_destroy(surface);

      fl_message("Chart exported to %s", chooser.value());
    } else {
      fl_alert("Failed to create SVG file: %s",
               cairo_status_to_string(cairo_surface_status(surface)));
      cairo_surface_destroy(surface);
    }
  }
}

void FMenuExportPDF(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "PDF Files (*.pdf)", Fl_File_Chooser::CREATE,
                          "Export as PDF");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Create PDF surface
    cairo_surface_t *surface =
        cairo_pdf_surface_create(chooser.value(), gs.xWin, gs.yWin);

    if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
      // Switch to Cairo backend and render
      InitBackendCairo(surface);
      GBClearScreen(gi.kiOff);
      gi.fFile = fFalse;
      DrawChartX();
      EndBackendCairo();

      cairo_surface_destroy(surface);

      fl_message("Chart exported to %s", chooser.value());
    } else {
      fl_alert("Failed to create PDF file: %s",
               cairo_status_to_string(cairo_surface_status(surface)));
      cairo_surface_destroy(surface);
    }
  }
}
#endif // CAIRO

void FMenuExportBitmap(Fl_Widget *w, void *data) {
#ifdef CAIRO
  Fl_File_Chooser chooser(".", "PNG Files (*.png)", Fl_File_Chooser::CREATE,
                          "Export as PNG");
  chooser.preview(0);
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    char szFile[cchSzMax];
    strncpy(szFile, chooser.value(), sizeof(szFile) - 1);
    szFile[sizeof(szFile) - 1] = '\0';

    // Add .png extension if not present
    int len = strlen(szFile);
    if (len <= 4 || NCompareSzI(&szFile[len - 4], ".png") != 0) {
      strcat(szFile, ".png");
    }

    // Create PNG using Cairo
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gs.xWin, gs.yWin);

    if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
      InitBackendCairo(surface);
      GBClearScreen(gi.kiOff);
      gi.fFile = fFalse;
      DrawChartX();
      EndBackendCairo();

      cairo_status_t status = cairo_surface_write_to_png(surface, szFile);
      cairo_surface_destroy(surface);

      if (status == CAIRO_STATUS_SUCCESS)
        fl_message("Chart exported to %s", szFile);
      else
        fl_alert("Failed to write PNG: %s", cairo_status_to_string(status));
    } else {
      fl_alert("Failed to create image surface");
      cairo_surface_destroy(surface);
    }
  }
#else
  fl_alert("PNG export requires Cairo support.");
#endif
}

void FMenuFileExit(Fl_Widget *w, void *data) {
  if (fi.window)
    fi.window->hide();
}

void FMenuEditCopy(Fl_Widget *w, void *data) {
  // Placeholder - see FMenuCopyText for actual text copy implementation
}

// Export chart text output to a file
void FMenuExportText(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Text Files (*.txt)\tAll Files (*)",
                          Fl_File_Chooser::CREATE, "Export Chart Text Output");
  chooser.preview(0);
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value() == NULL)
    return;

  char szFile[cchSzMax];
  strncpy(szFile, chooser.value(), sizeof(szFile) - 1);
  szFile[sizeof(szFile) - 1] = '\0';

  // Save current graphics state
  flag fGraphicsSave = us.fGraphics;

  // Set up file output
  FCloneSz(szFile, &is.szFileScreen);
  us.fGraphics = fFalse;

  // Generate the chart text output
  Action();

  // Clean up
  FCloneSz(NULL, &is.szFileScreen);
  us.fGraphics = fGraphicsSave;

  fl_message("Chart text exported to:\n%s", szFile);
}

// Copy chart text output to clipboard
void FMenuCopyText(Fl_Widget *w, void *data) {
  // Create a temp file path
  char szTempFile[cchSzMax];
#ifdef __APPLE__
  const char *tmpDir = getenv("TMPDIR");
  if (tmpDir == NULL)
    tmpDir = "/tmp";
  snprintf(szTempFile, sizeof(szTempFile), "%s/astrolog_text.tmp", tmpDir);
#else
  snprintf(szTempFile, sizeof(szTempFile), "/tmp/astrolog_text.tmp");
#endif

  // Save current graphics state
  flag fGraphicsSave = us.fGraphics;

  // Set up file output
  FCloneSz(szTempFile, &is.szFileScreen);
  us.fGraphics = fFalse;

  // Generate the chart text output
  Action();

  // Clean up output redirection
  FCloneSz(NULL, &is.szFileScreen);
  us.fGraphics = fGraphicsSave;

  // Read the temp file
  FILE *fp = fopen(szTempFile, "r");
  if (fp == NULL) {
    fl_alert("Failed to generate chart text.");
    return;
  }

  // Get file size
  fseek(fp, 0, SEEK_END);
  long fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (fileSize <= 0) {
    fclose(fp);
    unlink(szTempFile);
    fl_alert("No chart text was generated.");
    return;
  }

  // Allocate buffer and read file
  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    fclose(fp);
    unlink(szTempFile);
    fl_alert("Out of memory.");
    return;
  }

  size_t bytesRead = fread(buffer, 1, fileSize, fp);
  buffer[bytesRead] = '\0';
  fclose(fp);

  // Copy to clipboard using FLTK
  Fl::copy(buffer, (int)bytesRead, 1); // 1 = system clipboard

  // Clean up
  free(buffer);
  unlink(szTempFile);
}

// Update text window colors based on inverse setting
static void UpdateTextWindowColors() {
  if (!fi.textDisplay)
    return;
  if (gs.fInverse) {
    // White background, black text
    fi.textDisplay->color(FL_WHITE);
    fi.textDisplay->textfgcolor(FL_BLACK);
    fi.textDisplay->textbgcolor(FL_WHITE);
    fi.textDisplay->cursorbgcolor(FL_WHITE);
    fi.textDisplay->cursorfgcolor(FL_WHITE);
  } else {
    // Black background, white text
    fi.textDisplay->color(FL_BLACK);
    fi.textDisplay->textfgcolor(FL_WHITE);
    fi.textDisplay->textbgcolor(FL_BLACK);
    fi.textDisplay->cursorbgcolor(FL_BLACK);
    fi.textDisplay->cursorfgcolor(FL_BLACK);
  }
  fi.textDisplay->redraw();
}

// Text window close callback - update menu state when window is closed via X
// button
static void TextWindowCloseCallback(Fl_Widget *w, void *data) {
  if (fi.textWindow) {
    fi.textWindow->hide();
    // Also disable interpretations when window is closed
    us.fInterpret = fFalse;
    UpdateMenuCheck(FMenuInterpret, us.fInterpret);
  }
}

// Create the text output window
static void CreateTextWindow() {
  if (fi.textWindow)
    return; // Already exists

  fi.textWindow = new Fl_Window(600, 500, "Chart Text Output");
  fi.textDisplay = new Fl_Terminal(0, 0, 600, 500);
  fi.textDisplay->textfont(FL_COURIER);
  fi.textDisplay->textsize(12);
  // Set terminal colors based on current inverse setting
  UpdateTextWindowColors();
  // Enable ANSI parsing
  fi.textDisplay->ansi(true);
  fi.textWindow->resizable(fi.textDisplay);
  fi.textWindow->callback(TextWindowCloseCallback);
  fi.textWindow->end();
}

// Refresh the text window content with current chart text output
void RefreshTextWindow() {
  if (!fi.textWindow || !fi.textWindow->shown())
    return;

  // Capture text output to temp file (reuse existing pattern from
  // FMenuCopyText)
  char szTempFile[cchSzMax];
#ifdef __APPLE__
  const char *tmpDir = getenv("TMPDIR");
  if (tmpDir == NULL)
    tmpDir = "/tmp";
  snprintf(szTempFile, sizeof(szTempFile), "%s/astrolog_textwin.tmp", tmpDir);
#else
  snprintf(szTempFile, sizeof(szTempFile), "/tmp/astrolog_textwin.tmp");
#endif

  flag fGraphicsSave = us.fGraphics;
  flag fAnsiColorSave = us.fAnsiColor;
  FCloneSz(szTempFile, &is.szFileScreen);
  us.fGraphics = fFalse;
  us.fAnsiColor = fTrue;  // Enable ANSI colors for terminal output
  Action();
  FCloneSz(NULL, &is.szFileScreen);
  us.fGraphics = fGraphicsSave;
  us.fAnsiColor = fAnsiColorSave;

  // Load into terminal
  fi.textDisplay->reset_terminal();
  fi.textDisplay->clear_history();

  // Read file and append to terminal (Fl_Terminal::append parses ANSI)
  FILE *fp = fopen(szTempFile, "r");
  if (fp) {
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
      fi.textDisplay->append(line);
    }
    fclose(fp);
  }
  unlink(szTempFile);
}

// Toggle the text window visibility
void FMenuShowTextWindow(Fl_Widget *w, void *data) {
  if (!fi.textWindow) {
    CreateTextWindow();
  }

  if (fi.textWindow->shown()) {
    fi.textWindow->hide();
    UpdateMenuCheck(FMenuShowTextWindow, false);
  } else {
    fi.textWindow->show();
    RefreshTextWindow(); // Generate text after showing window
    UpdateMenuCheck(FMenuShowTextWindow, true);
  }
}

// Execute a macro command
void FMenuMacro(Fl_Widget *w, void *data) {
  int i = (int)(long)data;
  if (is.rgszMacro != NULL && i <= is.cszMacro && is.rgszMacro[i]) {
    FProcessCommandLine(is.rgszMacro[i]);
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  } else {
    // For F1, show help instead of error if macro not defined
    if (i == 1) {
      FMenuHelpKeystroke(w, NULL);
      return;
    }
    char sz[80];
    sprintf(sz, "Macro number %d is not defined.", i);
    fl_message("%s", sz);
  }
}

void FMenuHelpAbout(Fl_Widget *w, void *data) { FShowDlgAbout(); }

void FMenuInfoChart(Fl_Widget *w, void *data) { FShowDlgInfo(1); }

void FMenuInfoChart2(Fl_Widget *w, void *data) { FShowDlgInfo(2); }

void FMenuChartNow(Fl_Widget *w, void *data) {
#ifdef TIME
  Animate(10, 0);
  ciMain = ciCore;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
#endif
}

// Relationship chart callbacks
static void FSetRel(int rc) {
  CI ciT;
  if (us.nRel == rcMidpoint) { // Restore chart when leaving midpoint mode
    ciT = ciMain;
    ciCore = ciMain = ciSave;
    ciSave = ciT;
  }
  if (rc == rcMidpoint) // Remember chart so can restore it later
    ciSave = ciMain;
  us.nRel = rc;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuRelNo(Fl_Widget *w, void *data) {
  FSetRel(us.nRel ? rcNone : rcDual);
}

void FMenuRelSynastry(Fl_Widget *w, void *data) { FSetRel(rcSynastry); }

void FMenuRelComposite(Fl_Widget *w, void *data) { FSetRel(rcComposite); }

void FMenuRelMidpoint(Fl_Widget *w, void *data) { FSetRel(rcMidpoint); }

void FMenuRelDate(Fl_Widget *w, void *data) {
  FSetRel(rcDifference);
  gi.nMode = gWheel;
  us.fGraphics = fFalse;
}

#ifdef BIORHYTHM
void FMenuRelBiorhythm(Fl_Widget *w, void *data) {
  FSetRel(rcBiorhythm);
  gi.nMode = gBiorhythm;
}
#endif

void FMenuRelTransit(Fl_Widget *w, void *data) { FSetRel(rcTransit); }

void FMenuRelProgressed(Fl_Widget *w, void *data) { FSetRel(rcProgress); }

// House system callbacks
void FMenuHouseSystem(Fl_Widget *w, void *data) {
  us.nHouseSystem = (int)(long)data;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouseSolar(Fl_Widget *w, void *data) {
  us.objOnAsc = us.objOnAsc ? 0 : oSun + 1;
  UpdateMenuCheck(FMenuHouseSolar, us.objOnAsc);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouse3D(Fl_Widget *w, void *data) {
  inv(us.fHouse3D);
  UpdateMenuCheck(FMenuHouse3D, us.fHouse3D);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouseDecan(Fl_Widget *w, void *data) {
  inv(us.fDecan);
  UpdateMenuCheck(FMenuHouseDecan, us.fDecan);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouseFlip(Fl_Widget *w, void *data) {
  inv(us.fFlip);
  UpdateMenuCheck(FMenuHouseFlip, us.fFlip);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

// Map effects callbacks
void FMenuMapConstel(Fl_Widget *w, void *data) {
  inv(gs.fConstel);
  UpdateMenuCheck(FMenuMapConstel, gs.fConstel);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuMapAllStar(Fl_Widget *w, void *data) {
  inv(gs.fAllStar);
  UpdateMenuCheck(FMenuMapAllStar, gs.fAllStar);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuMapExo(Fl_Widget *w, void *data) {
  inv(gs.fAllExo);
  UpdateMenuCheck(FMenuMapExo, gs.fAllExo);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuMapCity(Fl_Widget *w, void *data) {
  inv(gs.fLabelCity);
  UpdateMenuCheck(FMenuMapCity, gs.fLabelCity);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuMapBmp(Fl_Widget *w, void *data) {
  inv(gi.fBmp);
  UpdateMenuCheck(FMenuMapBmp, gi.fBmp);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuMapAxis(Fl_Widget *w, void *data) {
  inv(gs.fEcliptic);
  UpdateMenuCheck(FMenuMapAxis, gs.fEcliptic);
  if (fi.chart)
    fi.chart->redraw();
}

// Wheel fill callback
void FMenuWheelFill(Fl_Widget *w, void *data) {
  gs.nDecaFill = (int)(intptr_t)data;
  UpdateMenuRadioByValue(FMenuWheelFill, (intptr_t)data);
  if (fi.chart)
    fi.chart->redraw();
}

// Indian style chart callbacks
void FMenuIndian(Fl_Widget *w, void *data) {
  inv(gs.fIndianWheel);
  UpdateMenuCheck(FMenuIndian, gs.fIndianWheel);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIndianS(Fl_Widget *w, void *data) {
  gi.nMode = gWheel;
  gs.fIndianWheel = fTrue;
  gs.fHouseExtra = fFalse;
  UpdateMenuCheck(FMenuIndian, fTrue);
  UpdateMenuRadio(FMenuIndianS);
  us.fGraphics = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIndianN(Fl_Widget *w, void *data) {
  gi.nMode = gHouse;
  gs.fIndianWheel = fTrue;
  UpdateMenuCheck(FMenuIndian, fTrue);
  UpdateMenuRadio(FMenuIndianN);
  us.fGraphics = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIndianE(Fl_Widget *w, void *data) {
  gi.nMode = gWheel;
  gs.fIndianWheel = gs.fHouseExtra = fTrue;
  UpdateMenuCheck(FMenuIndian, fTrue);
  UpdateMenuRadio(FMenuIndianE);
  us.fGraphics = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

// Help list callbacks - these switch to text mode
void FMenuHelpSign(Fl_Widget *w, void *data) {
  gi.nMode = gSign;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHelpObject(Fl_Widget *w, void *data) {
  gi.nMode = gObject;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHelpAspect(Fl_Widget *w, void *data) {
  gi.nMode = gHelpAsp;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

#ifdef CONSTEL
void FMenuHelpConstel(Fl_Widget *w, void *data) {
  gi.nMode = gConstel;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}
#endif

void FMenuHelpPlanet(Fl_Widget *w, void *data) {
  gi.nMode = gPlanet;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHelpRay(Fl_Widget *w, void *data) {
  gi.nMode = gRay;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

#ifdef INTERPRET
void FMenuHelpMeaning(Fl_Widget *w, void *data) {
  gi.nMode = gMeaning;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}
#endif

void FMenuHelpSwitch(Fl_Widget *w, void *data) {
  gi.nMode = gSwitch;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHelpObscure(Fl_Widget *w, void *data) {
  gi.nMode = gObscure;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHelpKeystroke(Fl_Widget *w, void *data) {
  gi.nMode = gKeystroke;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHelpCredit(Fl_Widget *w, void *data) {
  gi.nMode = gCredit;
  us.fGraphics = fFalse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuCommand(Fl_Widget *w, void *data) {
  char szCommand[cchSzMax] = "";
  if (FShowDlgCommand(szCommand, cchSzMax) && szCommand[0]) {
    FProcessCommandLine(szCommand);
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuGraphicsSettings(Fl_Widget *w, void *data) { FShowDlgGraphics(); }

void FMenuCalcSettings(Fl_Widget *w, void *data) { FShowDlgCalc(); }

void FMenuDisplaySettings(Fl_Widget *w, void *data) { FShowDlgDisplay(); }

void FMenuAnimSettings(Fl_Widget *w, void *data) { FShowDlgAnim(); }

void FMenuAspectSettings(Fl_Widget *w, void *data) { FShowDlgAspect(); }

void FMenuRestrict(Fl_Widget *w, void *data) { FShowDlgRestrict(fFalse); }

void FMenuRestrictTransit(Fl_Widget *w, void *data) { FShowDlgRestrict(fTrue); }

void FMenuColorSettings(Fl_Widget *w, void *data) { FShowDlgColor(); }

void FMenuTransitDlg(Fl_Widget *w, void *data) { FShowDlgTransit(); }

void FMenuProgressDlg(Fl_Widget *w, void *data) { FShowDlgProgress(); }

void FMenuChartSettingsDlg(Fl_Widget *w, void *data) {
  FShowDlgChartSettings();
}

void FMenuSwapCharts(Fl_Widget *w, void *data) {
  CI ci;
  SwapTemp(ciCore, ciTwin, ci);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuDefaultInfo(Fl_Widget *w, void *data) { FShowDlgDefaultInfo(); }

void FMenuInfoAll(Fl_Widget *w, void *data) { FShowDlgInfoAll(); }

void FMenuListPrev(Fl_Widget *w, void *data) {
  if (is.cci <= 0) {
    fl_message("There is no chart list in memory.");
    return;
  }
  int i = is.iciCur - 1;
  if (i < 0)
    i = 0;
  if (i != is.iciCur) {
    is.iciCur = i;
    ciCore = is.rgci[i];
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuListNext(Fl_Widget *w, void *data) {
  if (is.cci <= 0) {
    fl_message("There is no chart list in memory.");
    return;
  }
  int i = is.iciCur + 1;
  if (i >= is.cci)
    i = is.cci - 1;
  if (i != is.iciCur) {
    is.iciCur = i;
    ciCore = is.rgci[i];
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuListFirst(Fl_Widget *w, void *data) {
  if (is.cci <= 0) {
    fl_message("There is no chart list in memory.");
    return;
  }
  int i = 0;
  if (i != is.iciCur) {
    is.iciCur = i;
    ciCore = is.rgci[i];
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuListLast(Fl_Widget *w, void *data) {
  if (is.cci <= 0) {
    fl_message("There is no chart list in memory.");
    return;
  }
  int i = is.cci - 1;
  if (i != is.iciCur) {
    is.iciCur = i;
    ciCore = is.rgci[i];
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuListDlg(Fl_Widget *w, void *data) { FShowDlgList(); }

void FMenuMoonRestrict(Fl_Widget *w, void *data) { FShowDlgMoons(); }

void FMenuMoonObjSettings(Fl_Widget *w, void *data) { FShowDlgMoonObj(); }

void FMenuObjCustom(Fl_Widget *w, void *data) { FShowDlgCustom(); }

void FMenuStarCustom(Fl_Widget *w, void *data) { FShowDlgCustomS(); }

void FMenuFileOpenChart2(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Chart Files (*.as)\tAll Files (*)",
                          Fl_File_Chooser::SINGLE, "Open Chart #2");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Save current chart, load new one, then copy to chart #2
    CI ciSav = ciCore;
    if (FInputData(chooser.value())) {
      ciTwin = ciCore;
      ciCore = ciSav; // Restore original chart #1
    }
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuFileSaveSettings(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Chart Files (*.as)", Fl_File_Chooser::CREATE,
                          "Save Program Settings");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Write settings to file using FOutputSettings
    FOutputSettings();
  }
}

void FMenuFilePrint(Fl_Widget *w, void *data) {
  if (!fi.chart) {
    fl_alert("No chart to print.");
    return;
  }

  Fl_Printer printer;

  // Show native print dialog
  if (printer.start_job(1) != 0) {
    return; // User cancelled or error
  }

  if (printer.start_page() != 0) {
    printer.end_job();
    return;
  }

  // Get printable area
  int pw, ph;
  printer.printable_rect(&pw, &ph);

  // Get current chart size
  int chartW = fi.chart->w();
  int chartH = fi.chart->h();

  // Calculate scale to fit on page while maintaining aspect ratio
  double scaleX = (double)pw / chartW;
  double scaleY = (double)ph / chartH;
  double scale = (scaleX < scaleY) ? scaleX : scaleY;

  // Center the chart on the page
  int scaledW = (int)(chartW * scale);
  int scaledH = (int)(chartH * scale);
  int offsetX = (pw - scaledW) / 2;
  int offsetY = (ph - scaledH) / 2;

  // Set origin and scale for printing
  printer.origin(offsetX, offsetY);
  printer.scale((float)scale, (float)scale);

  // Print the chart widget
  printer.print_widget(fi.chart, 0, 0);

  printer.end_page();
  printer.end_job();
}

void FMenuSaveChartList(Fl_Widget *w, void *data) {
  if (is.cci <= 0) {
    fl_message("There is no chart list in memory to save.");
    return;
  }

  Fl_File_Chooser chooser(".", "Chart List (*.as)", Fl_File_Chooser::CREATE,
                          "Save Chart List");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    char szCmd[cchSzMax];
    sprintf(szCmd, "-ol \"%s\"", chooser.value());
    FProcessCommandLine(szCmd);
  }
}

void FMenuSaveAAF(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "AAF Files (*.aaf)", Fl_File_Chooser::CREATE,
                          "Save Chart Exchange (AAF)");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    char szCmd[cchSzMax];
    sprintf(szCmd, "-oa \"%s\"", chooser.value());
    FProcessCommandLine(szCmd);
  }
}

void FMenuSaveQuick(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Quick*Chart Files (*.qck)",
                          Fl_File_Chooser::CREATE, "Save Chart Quick*Chart");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    char szCmd[cchSzMax];
    sprintf(szCmd, "-oq \"%s\"", chooser.value());
    FProcessCommandLine(szCmd);
  }
}

void FMenuOpenBackground(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Bitmap Files (*.bmp)\tAll Files (*)",
                          Fl_File_Chooser::SINGLE, "Open Chart Background");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Load the bitmap as background
    char szCmd[cchSzMax];
    sprintf(szCmd, "-XI \"%s\"", chooser.value());
    FProcessCommandLine(szCmd);
    gi.fBmp = fTrue;
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuOpenWorldMap(Fl_Widget *w, void *data) {
  Fl_File_Chooser chooser(".", "Bitmap Files (*.bmp)\tAll Files (*)",
                          Fl_File_Chooser::SINGLE, "Open World Map");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Load the bitmap as world map
    char szCmd[cchSzMax];
    sprintf(szCmd, "-XB \"%s\"", chooser.value());
    FProcessCommandLine(szCmd);
    fi.fDoCast = fTrue;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuObjectSettings(Fl_Widget *w, void *data) { FShowDlgObject(); }

void FMenuObjectSettings2(Fl_Widget *w, void *data) { FShowDlgObject2(); }

void FMenuStarRestrict(Fl_Widget *w, void *data) { FShowDlgStar(); }

void FMenuHelpWebsite(Fl_Widget *w, void *data) {
#ifdef __APPLE__
  system("open http://www.astrolog.org/astrolog.htm");
#elif defined(__linux__)
  system("xdg-open http://www.astrolog.org/astrolog.htm");
#elif defined(_WIN32)
  system("start http://www.astrolog.org/astrolog.htm");
#endif
}

void FMenuHelpChanges(Fl_Widget *w, void *data) {
#ifdef __APPLE__
  system("open http://www.astrolog.org/astrolog/astrolog.htm");
#elif defined(__linux__)
  system("xdg-open http://www.astrolog.org/astrolog/astrolog.htm");
#elif defined(_WIN32)
  system("start http://www.astrolog.org/astrolog/astrolog.htm");
#endif
}

void FMenuHelpLicense(Fl_Widget *w, void *data) {
  // Show the license in a message box
  fl_message(
      "Astrolog (Version 7.80)\n\n"
      "Copyright (C) 1991-2025 by Walter D. Pullen\n"
      "(Astara@msn.com, http://www.astrolog.org/astrolog.htm)\n\n"
      "Permission is granted to freely use, modify, and distribute\n"
      "these routines provided these credits and notices remain\n"
      "unmodified with any altered or distributed versions of the program.");
}

// Helper function to open a file with the default text editor
static void OpenFileInEditor(const char *szFile) {
  char szCmd[cchSzMax];
#ifdef __APPLE__
  sprintf(szCmd, "open -t \"%s\"", szFile);
#elif defined(__linux__)
  sprintf(szCmd, "xdg-open \"%s\"", szFile);
#elif defined(_WIN32)
  sprintf(szCmd, "notepad \"%s\"", szFile);
#endif
  system(szCmd);
}

void FMenuDocDefault(Fl_Widget *w, void *data) {
  OpenFileInEditor(DEFAULT_INFOFILE);
}

void FMenuDocAtlas(Fl_Widget *w, void *data) {
  OpenFileInEditor(DEFAULT_ATLASFILE);
}

void FMenuDocTimezone(Fl_Widget *w, void *data) {
  OpenFileInEditor(DEFAULT_TIMECHANGE);
}

void FMenuDocStar(Fl_Widget *w, void *data) {
  OpenFileInEditor("sefstars.txt");
}

void FMenuDocOrbit(Fl_Widget *w, void *data) {
  OpenFileInEditor("seorbel.txt");
}

void FMenuDocExo(Fl_Widget *w, void *data) { OpenFileInEditor(szFileExoCore); }

void FMenuFullScreen(Fl_Widget *w, void *data) {
  if (fi.window) {
    static int savedX = 0, savedY = 0, savedW = 0, savedH = 0;
    static bool isFullScreen = false;

    if (!isFullScreen) {
      // Save current position and size
      savedX = fi.window->x();
      savedY = fi.window->y();
      savedW = fi.window->w();
      savedH = fi.window->h();
      // Go fullscreen
      fi.window->fullscreen();
      isFullScreen = true;
    } else {
      // Restore previous size
      fi.window->fullscreen_off(savedX, savedY, savedW, savedH);
      isFullScreen = false;
    }
  }
}

void FMenuGlyphFont(Fl_Widget *w, void *data) {
  int font = (int)(intptr_t)data;
  // Set all glyph fonts (signs, houses, objects, aspects, nakshatras) to
  // selected font Format: 0SSSSS where each S is a font digit
  gs.nFontSig = font;
  gs.nFontHou = font;
  gs.nFontObj = font;
  gs.nFontAsp = font;
  gs.nFontNak = font;
  gs.nFontAll = font * 11111; // Pack into single value
  UpdateMenuRadio(FMenuGlyphFont);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuChartType(Fl_Widget *w, void *data) { FShowDlgChartType(); }

// View menu callbacks for chart type switching
void FMenuViewWheel(Fl_Widget *w, void *data) {
  gi.nMode = gWheel;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewGrid(Fl_Widget *w, void *data) {
  gi.nMode = gGrid;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewMidpoint(Fl_Widget *w, void *data) {
  gi.nMode = gMidpoint;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewHorizon(Fl_Widget *w, void *data) {
  gi.nMode = gHorizon;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewOrbit(Fl_Widget *w, void *data) {
  gi.nMode = gOrbit;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewAstroGraph(Fl_Widget *w, void *data) {
  gi.nMode = gAstroGraph;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewGlobe(Fl_Widget *w, void *data) {
  gi.nMode = gGlobe;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(fUseOpenGL);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewSphere(Fl_Widget *w, void *data) {
  gi.nMode = gSphere;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(fUseOpenGL);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewLocal(Fl_Widget *w, void *data) {
  gi.nMode = gLocal;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(fUseOpenGL);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewTelescope(Fl_Widget *w, void *data) {
  gi.nMode = gTelescope;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(fUseOpenGL);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewPolar(Fl_Widget *w, void *data) {
  gi.nMode = gPolar;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(fUseOpenGL);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

void FMenuViewWorldMap(Fl_Widget *w, void *data) {
  gi.nMode = gWorldMap;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window)
    fi.window->switchTo3D(false);
#else
  if (fi.chart)
    fi.chart->redraw();
#endif
}

// Additional chart type callbacks
void FMenuViewSector(Fl_Widget *w, void *data) {
  gi.nMode = gSector;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewCalendar(Fl_Widget *w, void *data) {
  gi.nMode = gCalendar;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewInfluence(Fl_Widget *w, void *data) {
  gi.nMode = gDisposit;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewEphemeris(Fl_Widget *w, void *data) {
  gi.nMode = gEphemeris;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewRising(Fl_Widget *w, void *data) {
  gi.nMode = gRising;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewList(Fl_Widget *w, void *data) {
  gi.nMode = gWheel;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewHouse(Fl_Widget *w, void *data) {
  gi.nMode = gHouse;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewAspect(Fl_Widget *w, void *data) {
  gi.nMode = gAspect;
  us.fAspList = fTrue;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewEsoteric(Fl_Widget *w, void *data) {
  gi.nMode = gEsoteric;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

#ifdef ARABIC
void FMenuViewArabic(Fl_Widget *w, void *data) {
  gi.nMode = gArabic;
  us.fArabic = fTrue;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}
#endif

void FMenuViewNearestCity(Fl_Widget *w, void *data) {
  gi.nMode = gLocal;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewMoons(Fl_Widget *w, void *data) {
  gi.nMode = gMoons;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuViewExo(Fl_Widget *w, void *data) {
  gi.nMode = gExo;
  us.fExoTransit = fTrue;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

// Animate menu callbacks
void FMenuAnimPause(Fl_Widget *w, void *data) { inv(gi.fPause); }

void FMenuAnimReverse(Fl_Widget *w, void *data) { neg(gi.nDir); }

void FMenuAnimForward(Fl_Widget *w, void *data) {
  Animate(gs.nAnim, gi.nDir);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuAnimBack(Fl_Widget *w, void *data) {
  Animate(gs.nAnim, -gi.nDir);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuAnimToggle(Fl_Widget *w, void *data) {
  gs.nAnim = gs.nAnim ? 0 : -4; // Toggle animation (default to days)
}

void FMenuAnimNow(Fl_Widget *w, void *data) {
#ifdef TIME
  Animate(10, 0);
  ciMain = ciCore;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
#endif
}

// Jump Rate callbacks
void FMenuAnimRate1(Fl_Widget *w, void *data) { gs.nAnim = -1; } // Seconds
void FMenuAnimRate2(Fl_Widget *w, void *data) { gs.nAnim = -2; } // Minutes
void FMenuAnimRate3(Fl_Widget *w, void *data) { gs.nAnim = -3; } // Hours
void FMenuAnimRate4(Fl_Widget *w, void *data) { gs.nAnim = -4; } // Days
void FMenuAnimRate5(Fl_Widget *w, void *data) { gs.nAnim = -5; } // Months
void FMenuAnimRate6(Fl_Widget *w, void *data) { gs.nAnim = -6; } // Years
void FMenuAnimRate7(Fl_Widget *w, void *data) { gs.nAnim = -7; } // Decades
void FMenuAnimRate8(Fl_Widget *w, void *data) { gs.nAnim = -8; } // Centuries
void FMenuAnimRate9(Fl_Widget *w, void *data) { gs.nAnim = -9; } // Millennia

// Jump Factor callbacks - magnitude stored in |gi.nDir|, sign is direction
void FMenuAnimFactor1(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 1;
}
void FMenuAnimFactor2(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 2;
}
void FMenuAnimFactor3(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 3;
}
void FMenuAnimFactor4(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 4;
}
void FMenuAnimFactor5(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 5;
}
void FMenuAnimFactor6(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 6;
}
void FMenuAnimFactor7(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 7;
}
void FMenuAnimFactor8(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 8;
}
void FMenuAnimFactor9(Fl_Widget *w, void *data) {
  gi.nDir = (gi.nDir > 0 ? 1 : -1) * 9;
}

void FMenuAnimTimedExposure(Fl_Widget *w, void *data) {
  inv(gs.fJetTrail);
  UpdateMenuCheck(FMenuAnimTimedExposure, gs.fJetTrail);
  if (fi.chart)
    fi.chart->redraw();
}

// Helper to update menu checkbox state
static void UpdateMenuCheck(Fl_Callback *cb, flag f) {
  if (fi.menubar) {
    Fl_Menu_Item *item = (Fl_Menu_Item *)fi.menubar->find_item(cb);
    if (item) {
      if (f)
        item->set();
      else
        item->clear();
    }
  }
}

// Helper to update radio button selection
static void UpdateMenuRadio(Fl_Callback *cb) {
  if (fi.menubar) {
    Fl_Menu_Item *item = (Fl_Menu_Item *)fi.menubar->find_item(cb);
    if (item) {
      item->setonly();
    }
  }
}

// Helper to update radio button selection by matching data value
static void UpdateMenuRadioByValue(Fl_Callback *cb, intptr_t value) {
  if (fi.menubar) {
    const Fl_Menu_Item *menu = fi.menubar->menu();
    int n = fi.menubar->size();
    for (int i = 0; i < n; i++) {
      if (menu[i].callback() == cb && (intptr_t)menu[i].user_data() == value) {
        ((Fl_Menu_Item *)&menu[i])->setonly();
        break;
      }
    }
  }
}

// Helper to handle Alt+letter menu shortcuts (for macOS parity with Windows)
static flag HandleAltMenuShortcut(int key) {
  if (!fi.menubar || !(Fl::event_state() & FL_ALT))
    return fFalse;

  // Convert key to lowercase for matching
  int ch = key;
  if (ch >= 'A' && ch <= 'Z')
    ch = ch - 'A' + 'a';
  if (ch < 'a' || ch > 'z')
    return fFalse;

  // Search top-level menu items for matching mnemonic
  const Fl_Menu_Item *menu = fi.menubar->menu();
  int n = fi.menubar->size();
  int xPos = fi.menubar->x(); // Start at menubar left edge

  for (int i = 0; i < n; i++) {
    const char *label = menu[i].label();
    if (!label)
      continue;

    // Only process top-level menu items (those with submenus)
    if (!menu[i].submenu()) {
      continue;
    }

    // Find mnemonic character (after '&')
    const char *amp = strchr(label, '&');
    if (amp && amp[1]) {
      int mnemonic = amp[1];
      if (mnemonic >= 'A' && mnemonic <= 'Z')
        mnemonic = mnemonic - 'A' + 'a';
      if (mnemonic == ch) {
        // Found matching menu - pulldown at the correct x position
        const Fl_Menu_Item *picked =
            menu[i].pulldown(xPos, fi.menubar->y() + fi.menubar->h(), 0, 0,
                             &menu[i], fi.menubar);
        if (picked && picked->callback())
          picked->do_callback(fi.menubar);
        return fTrue;
      }
    }

    // Calculate width of this menu item to advance x position
    // Use FLTK's text measurement for the label (without the '&')
    fl_font(fi.menubar->textfont(), fi.menubar->textsize());
    int labelWidth = 0, labelHeight = 0;
    // Strip '&' from label for measurement
    char cleanLabel[256];
    int j = 0;
    for (const char *p = label; *p && j < 254; p++) {
      if (*p != '&')
        cleanLabel[j++] = *p;
    }
    cleanLabel[j] = '\0';
    fl_measure(cleanLabel, labelWidth, labelHeight, 0);
    xPos += labelWidth + 20; // Add padding between menu items
  }
  return fFalse;
}

// Include menu callbacks
void FMenuIncludeMinors(Fl_Widget *w, void *data) {
  int i;
  for (i = oChi; i <= oVes; i++)
    inv(ignore[i]);
  for (i = oSou; i <= oEP; i++)
    inv(ignore[i]);
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  // Check if any minor is shown to set the menu checkmark
  flag fMinors = fFalse;
  for (i = oChi; i <= oVes && !fMinors; i++)
    fMinors = !ignore[i];
  UpdateMenuCheck(FMenuIncludeMinors, fMinors);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIncludeCusps(Fl_Widget *w, void *data) {
  int i;
  inv(us.fCusp);
  for (i = cuspLo; i <= cuspHi; i++)
    ignore[i] = !us.fCusp || !ignore[i];
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuIncludeCusps, us.fCusp);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIncludeUranians(Fl_Widget *w, void *data) {
  int i;
  inv(us.fUranian);
  for (i = uranLo; i <= uranHi; i++)
    ignore[i] = !us.fUranian || !ignore[i];
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuIncludeUranians, us.fUranian);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIncludeDwarfs(Fl_Widget *w, void *data) {
  int i;
  inv(us.fDwarf);
  for (i = dwarfLo; i <= dwarfHi; i++)
    ignore[i] = !us.fDwarf || !ignore[i];
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuIncludeDwarfs, us.fDwarf);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIncludeMoons(Fl_Widget *w, void *data) {
  int i;
  inv(us.fMoons);
  for (i = moonsLo; i <= moonsHi; i++)
    ignore[i] = !us.fMoons || !ignore[i];
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuIncludeMoons, us.fMoons);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIncludeCOB(Fl_Widget *w, void *data) {
  int i;
  inv(us.fCOB);
  for (i = cobLo; i <= cobHi; i++)
    ignore[i] = !us.fCOB || !ignore[i];
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuIncludeCOB, us.fCOB);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuIncludeStars(Fl_Widget *w, void *data) {
  int i;
  inv(us.fStar);
  for (i = starLo; i <= starHi; i++)
    ignore[i] = !us.fStar || !ignore[i];
  AdjustRestrictions();
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuIncludeStars, us.fStar);
  if (fi.chart)
    fi.chart->redraw();
}

// Settings toggle callbacks
void FMenuSidereal(Fl_Widget *w, void *data) {
  inv(us.fSidereal);
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuSidereal, us.fSidereal);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHeliocentric(Fl_Widget *w, void *data) {
  inv(us.objCenter);
  fi.fDoCast = fTrue;
  UpdateMenuCheck(FMenuHeliocentric, us.objCenter);
  if (fi.chart)
    fi.chart->redraw();
}

// Graphics toggle callbacks
void FMenuGraphicsReverse(Fl_Widget *w, void *data) {
  inv(gs.fInverse);
  InitColorPalette(gs.fInverse);
  InitColorsX();
  UpdateTextWindowColors();
  RefreshTextWindow();
  UpdateMenuCheck(FMenuGraphicsReverse, gs.fInverse);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsMonochrome(Fl_Widget *w, void *data) {
  inv(gs.fColor);
  InitColorsX();
  UpdateMenuCheck(FMenuGraphicsMonochrome, !gs.fColor);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuReduceContrast(Fl_Widget *w, void *data) {
  gs.nReduceContrast = (int)(long)data;
  UpdateMenuRadioByValue(FMenuReduceContrast, (intptr_t)data);
  InitColorsX();
  RefreshTextWindow();
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsBorder(Fl_Widget *w, void *data) {
  inv(gs.fBorder);
  UpdateMenuCheck(FMenuGraphicsBorder, gs.fBorder);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsText(Fl_Widget *w, void *data) {
  inv(gs.fText);
  UpdateMenuCheck(FMenuGraphicsText, gs.fText);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsThick(Fl_Widget *w, void *data) {
  inv(gs.fThick);
  UpdateMenuCheck(FMenuGraphicsThick, gs.fThick);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsLabel(Fl_Widget *w, void *data) {
  inv(gs.fLabel);
  UpdateMenuCheck(FMenuGraphicsLabel, gs.fLabel);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsLabelAsp(Fl_Widget *w, void *data) {
  inv(gs.fLabelAsp);
  UpdateMenuCheck(FMenuGraphicsLabelAsp, gs.fLabelAsp);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsModify(Fl_Widget *w, void *data) {
  inv(gs.fAlt);
  UpdateMenuCheck(FMenuGraphicsModify, gs.fAlt);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsHouseExtra(Fl_Widget *w, void *data) {
  inv(gs.fHouseExtra);
  UpdateMenuCheck(FMenuGraphicsHouseExtra, gs.fHouseExtra);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsEquator(Fl_Widget *w, void *data) {
  inv(gs.fEquator);
  UpdateMenuCheck(FMenuGraphicsEquator, gs.fEquator);
  if (fi.chart)
    fi.chart->redraw();
}

// Additional View menu toggle callbacks
void FMenuInterpret(Fl_Widget *w, void *data) {
  inv(us.fInterpret);
  UpdateMenuCheck(FMenuInterpret, us.fInterpret);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  // Show/hide text window based on interpretation state
  if (us.fInterpret) {
    // Open text window when interpretations enabled
    if (!fi.textWindow || !fi.textWindow->shown()) {
      FMenuShowTextWindow(NULL, NULL);
    } else {
      RefreshTextWindow();
    }
  } else {
    // Close text window when interpretations disabled
    if (fi.textWindow && fi.textWindow->shown()) {
      fi.textWindow->hide();
      UpdateMenuCheck(FMenuShowTextWindow, false);
    }
  }
}

void FMenuSecond(Fl_Widget *w, void *data) {
  inv(us.fSeconds);
  UpdateMenuCheck(FMenuSecond, us.fSeconds);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuParallel(Fl_Widget *w, void *data) {
  inv(us.fParallel);
  UpdateMenuCheck(FMenuParallel, us.fParallel);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuApplying(Fl_Widget *w, void *data) {
  inv(us.nAppSep);
  UpdateMenuCheck(FMenuApplying, us.nAppSep);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

// Additional Graphics menu callbacks
void FMenuGraphicsSidebar(Fl_Widget *w, void *data) {
  inv(gs.fDoSidebar);
  UpdateMenuCheck(FMenuGraphicsSidebar, gs.fDoSidebar);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuGraphicsAutoSquare(Fl_Widget *w, void *data) {
  inv(fAutoSquare);
  UpdateMenuCheck(FMenuGraphicsAutoSquare, fAutoSquare);
  // If enabling, square the window immediately
  if (fAutoSquare && fSquare && fi.window) {
    // Trigger the same logic as the resize callback
    SquareWindowCallback(fi.window);
  }
}

void FMenuGraphicsAntialias(Fl_Widget *w, void *data) {
  inv(gs.fAntialias);
  UpdateMenuCheck(FMenuGraphicsAntialias, gs.fAntialias);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuDashStyle(Fl_Widget *w, void *data) {
  // Toggle between 0 (stippled) and 1 (alpha)
  gs.nDashStyle = !gs.nDashStyle;
  UpdateMenuCheck(FMenuDashStyle, gs.nDashStyle == 1);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuModifyChart(Fl_Widget *w, void *data) {
  inv(us.fGridMidpoint);
  UpdateMenuCheck(FMenuModifyChart, us.fGridMidpoint);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuPenColor(Fl_Widget *w, void *data) {
  gi.kiPen = (int)(long)data;
  if (fi.chart)
    fi.chart->redraw();
}

// Scale callbacks - update nCharScaleManual and radio buttons
void FMenuScale1(Fl_Widget *w, void *data) {
  nCharScaleManual = 100;
  UpdateMenuRadio(FMenuScale1);
  if (fi.chart)
    fi.chart->redraw();
}
void FMenuScale2(Fl_Widget *w, void *data) {
  nCharScaleManual = 200;
  UpdateMenuRadio(FMenuScale2);
  if (fi.chart)
    fi.chart->redraw();
}
void FMenuScale3(Fl_Widget *w, void *data) {
  nCharScaleManual = 300;
  UpdateMenuRadio(FMenuScale3);
  if (fi.chart)
    fi.chart->redraw();
}
void FMenuScale4(Fl_Widget *w, void *data) {
  nCharScaleManual = 400;
  UpdateMenuRadio(FMenuScale4);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuScaleDecrease(Fl_Widget *w, void *data) {
  // In map modes (AstroGraph/WorldMap), scaling keys should affect text size
  // since the map scale itself is fixed to the window size.
  // Use map-specific scalar to avoid affecting global UI text.
  if (gi.nMode == gAstroGraph || gi.nMode == gWorldMap || gi.nMode == gGlobe ||
      gi.nMode == gPolar) {
    FltkAdjustMapGlyphScale(-50);
    if (fi.chart)
      fi.chart->redraw();
    return;
  }

  if (nCharScaleManual > 100) {
    nCharScaleManual -= 50;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuScaleIncrease(Fl_Widget *w, void *data) {
  // In map modes, scaling keys should affect text size.
  if (gi.nMode == gAstroGraph || gi.nMode == gWorldMap || gi.nMode == gGlobe ||
      gi.nMode == gPolar) {
    FltkAdjustMapGlyphScale(50);
    if (fi.chart)
      fi.chart->redraw();
    return;
  }

  if (nCharScaleManual < 400) {
    nCharScaleManual += 50;
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuTextDecrease(Fl_Widget *w, void *data) {
  if (gs.nScaleText > 50)
    gs.nScaleText -= 50;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuTextIncrease(Fl_Widget *w, void *data) {
  if (gs.nScaleText < 400)
    gs.nScaleText += 50;
  if (fi.chart)
    fi.chart->redraw();
}

// Map Orientation callbacks
void FMenuTiltZero(Fl_Widget *w, void *data) {
  gs.rTilt = 0.0;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuTiltNorth(Fl_Widget *w, void *data) {
  gs.rTilt = gs.rTilt + 5.0;
  if (gs.rTilt > rDegQuad)
    gs.rTilt = rDegQuad;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuTiltSouth(Fl_Widget *w, void *data) {
  gs.rTilt = gs.rTilt - 5.0;
  if (gs.rTilt < -rDegQuad)
    gs.rTilt = -rDegQuad;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuRotateWest(Fl_Widget *w, void *data) {
  gs.rRot = Mod(gs.rRot + 5.0);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuRotateEast(Fl_Widget *w, void *data) {
  gs.rRot = Mod(gs.rRot - 5.0);
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuZoomIn(Fl_Widget *w, void *data) {
  gs.nScale += 100;
  if (gs.nScale > 400)
    gs.nScale = 400;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuZoomOut(Fl_Widget *w, void *data) {
  gs.nScale -= 100;
  if (gs.nScale < 100)
    gs.nScale = 100;
  if (fi.chart)
    fi.chart->redraw();
}

// Animate Store/Recall callbacks
void FMenuStore(Fl_Widget *w, void *data) { ciSave = ciMain; }

void FMenuRecall(Fl_Widget *w, void *data) {
  ciMain = ciSave;
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

// Sub-second animation rates
void FMenuAnimRate_11(Fl_Widget *w, void *data) {
  gs.nAnim = 11;
} // 1/10th Seconds
void FMenuAnimRate_12(Fl_Widget *w, void *data) {
  gs.nAnim = 12;
} // 1/100th Seconds
void FMenuAnimRate_13(Fl_Widget *w, void *data) {
  gs.nAnim = 13;
} // 1/1000th Seconds

// Additional Settings callbacks
void FMenuHouseSetDwad(Fl_Widget *w, void *data) {
  us.nDwad = us.nDwad ? 0 : 1;
  UpdateMenuCheck(FMenuHouseSetDwad, us.nDwad > 0);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouseSetGeodetic(Fl_Widget *w, void *data) {
  inv(us.fGeodetic);
  UpdateMenuCheck(FMenuHouseSetGeodetic, us.fGeodetic);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouseSetNavamsa(Fl_Widget *w, void *data) {
  inv(us.fNavamsa);
  UpdateMenuCheck(FMenuHouseSetNavamsa, us.fNavamsa);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuHouseSetIndian(Fl_Widget *w, void *data) {
  inv(us.fIndian);
  UpdateMenuCheck(FMenuHouseSetIndian, us.fIndian);
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

// Graphics toggle (fGraphics) - show graphics mode
void FMenuGraphicsToggle(Fl_Widget *w, void *data) {
  inv(us.fGraphics);
  UpdateMenuCheck(FMenuGraphicsToggle, us.fGraphics);
  if (fi.chart)
    fi.chart->redraw();
}

// Redraw/Clear callbacks
void FMenuRedraw(Fl_Widget *w, void *data) {
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();
}

void FMenuClear(Fl_Widget *w, void *data) {
  if (fi.chart)
    fi.chart->redraw();
}

/*
******************************************************************************
** FLTK Graphics Entry Points
******************************************************************************
*/

void BeginFltk() {
  // Initialize graphics backend (loads bundled fonts on macOS)
  InitGraphicsBackend();

  // Use 24 bit color bitmaps for globe/map rendering.
  gi.fBmp = fTrue;

  // Initialize FLTK scheme for modern look
  Fl::scheme("gtk+");

  // Create the main window
  fi.window =
      new AstrologWindow(gs.xWin, gs.yWin, szAppNameCore " " szVersionCore);

  // Show the window
  fi.window->show();

  // Initialize colors
  InitColorsX();
}

void InteractFltk() {
  // Main event loop
  if (!fi.window)
    return;

  // Run the FLTK event loop
  while (fi.window && fi.window->visible()) {
    // Handle any pending flags
    if (fi.fDoResize) {
      fi.fDoResize = fFalse;
      gs.xWin = fi.xClient;
      gs.yWin = fi.yClient;
    }
    if (fi.fDoCast) {
      fi.fDoCast = fFalse;
      ciCore = ciMain;
      if (us.nRel)
        CastRelation();
      else
        CastChart(0);
      if (fi.chart)
        fi.chart->redraw();
      RefreshTextWindow(); // Update text window if visible
    }
    if (fi.fDoRedraw) {
      fi.fDoRedraw = fFalse;
      if (fi.chart)
        fi.chart->redraw();
      RefreshTextWindow(); // Update text window if visible
    }

    // Process events
    Fl::wait(0.01); // 10ms timeout for responsive animation
  }
}

void EndFltk() {
  // Clean up
  if (fi.window) {
    fi.window->stopAnimation();
    delete fi.window;
    fi.window = NULL;
  }
}

#endif // FLTK
