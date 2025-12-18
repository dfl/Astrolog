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
#include "xevent.h"
#include "xbackend.h"

#ifdef FLTK
#include "fdriver.h"
#ifdef OPENGL
#include "fgl.h"
#endif
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_ask.H>

#ifdef CAIRO
#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <cairo/cairo-pdf.h>
#endif

// Global FLTK state
FI fi = {0};

// Flag to enable Cairo rendering (for antialiased graphics)
#ifdef CAIRO
static int fUseCairo = fTrue;  // Default to Cairo rendering when available

void FMenuViewCairo(Fl_Widget *w, void *data)
{
  fUseCairo = !fUseCairo;

  // Update menu checkmark
  if (fi.menubar) {
    Fl_Menu_Item *item = (Fl_Menu_Item *)fi.menubar->find_item(FMenuViewCairo);
    if (item) {
      if (fUseCairo)
        item->set();
      else
        item->clear();
    }
  }

  if (fi.chart)
    fi.chart->redraw();
}
#endif

// Convert Astrolog color index to FLTK color
// Astrolog stores colors as 0x00BBGGRR (Windows COLORREF format)
Fl_Color FltkColorFromKI(int ki)
{
  if (ki < 0 || ki >= cColor)
    ki = 0;
  KV kv = rgbbmp[ki];
  uchar r = RgbR(kv);
  uchar g = RgbG(kv);
  uchar b = RgbB(kv);
  return fl_rgb_color(r, g, b);
}

/*
******************************************************************************
** ChartWidget Implementation
******************************************************************************
*/

ChartWidget::ChartWidget(int x, int y, int w, int h)
  : Fl_Widget(x, y, w, h), mousex_(-1), mousey_(-1), buttonx_(-1), buttony_(-1)
{
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::draw()
{
  // Clear the entire widget area with background color first
  fl_color(FL_BLACK);
  fl_rectf(x(), y(), w(), h());

  // Set the chart size to match the widget
  gs.xWin = w();
  gs.yWin = h();

#ifdef CAIRO
  if (fUseCairo) {
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
    // GetDeviceCaps returns 96 for 100%, 120 for 125%, 144 for 150%, 192 for 200%
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
      if (!envScale) envScale = getenv("GDK_SCALE");
      if (envScale) {
        float envVal = (float)atof(envScale);
        if (envVal >= 1.0f) scale = envVal;
      }
    }
#endif

    // Debug: print scale factor once
    static int debugOnce = 0;
    if (!debugOnce) {
      printf("Cairo rendering: scale=%.2f (fltk_scale=%.2f), widget=%dx%d, surface=%dx%d\n",
             scale, Fl::screen_scale(0), w(), h(), (int)(w()*scale), (int)(h()*scale));
      debugOnce = 1;
    }

    int surfW = (int)(w() * scale);
    int surfH = (int)(h() * scale);

    // Cairo rendering path - antialiased, resolution-independent
    cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, surfW, surfH);

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
    img.scale(w(), h(), 1, 1);  // Scale with high quality
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
    gi.fFile = fFalse;  // Drawing to screen, not file
    DrawChartX();

    fl_pop_matrix();
    fl_pop_clip();
  }
}

int ChartWidget::handle(int event)
{
  int mx = Fl::event_x() - x();
  int my = Fl::event_y() - y();

  switch (event) {
  case FL_PUSH:
    mousex_ = buttonx_ = mx;
    mousey_ = buttony_ = my;
    take_focus();  // Grab keyboard focus when clicked

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
    if (Fl::event_button() == FL_RIGHT_MOUSE) {
      // Right mouse drag: rotate/tilt globe views
      if (us.fGraphics && FSupportsRotation(gi.nMode)) {
        // Calculate rotation delta with mode-specific factor
        real rFactor = (gi.nMode == gLocal || gi.nMode == gTelescope) ?
          -gi.zViewRatio : 1.0;
        gs.rRot += (real)(mx - mousex_) * rDegHalf / (real)gs.xWin * rFactor;

        // Calculate tilt delta with mode-specific factor
        rFactor = (gi.nMode == gLocal || gi.nMode == gTelescope) ? gi.zViewRatio :
          (gi.nMode == gGlobe ? -1.0 : 1.0);
        gs.rTilt += (real)(my - mousey_) * rDegHalf / (real)gs.yWin * rFactor;

        // Clamp values using shared helpers
        ClampRotation();
        ClampTilt();

        if (gi.nMode == gMidpoint || gi.nMode == gTelescope) {
          if (gi.nMode == gMidpoint && gs.objTrack >= 0)
            gs.rRot = planet[gs.objTrack];
          gs.objTrack = -1;
        }
        mousex_ = mx;
        mousey_ = my;
        redraw();
      }
    } else if (Fl::event_button() == FL_LEFT_MOUSE) {
      // Left mouse drag: draw line
      if (Fl::event_state() & FL_SHIFT) {
        // Would draw line, but needs proper implementation
        mousex_ = mx;
        mousey_ = my;
      }
    }
    return 1;

  case FL_RELEASE:
    return 1;

  case FL_MOUSEWHEEL:
    // Mouse wheel for zoom
    {
      int dy = Fl::event_dy();
      if (dy != 0) {
        // Zoom in (dy < 0) or out (dy > 0)
        if (FAdjustZoom(dy < 0 ? 1 : -1))
          redraw();
      }
    }
    return 1;

  case FL_FOCUS:
  case FL_UNFOCUS:
    return 1;

  case FL_KEYDOWN:
    return handleKey(Fl::event_key());

  default:
    break;
  }

  return Fl_Widget::handle(event);
}

int ChartWidget::handleKey(int key)
{
  int i;

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
    inv(gi.fPause);  // Spacebar or 'p' toggles pause/play
    return 1;

  case 'r':
    neg(gi.nDir);
    return 1;

  case 'x':
    inv(gs.fInverse);
    InitColorPalette(gs.fInverse);
    redraw();
    return 1;

  case 'm':
    inv(gs.fColor);
    redraw();
    return 1;

  // Display toggles
  case 't':
    inv(gs.fText);
    redraw();
    return 1;

  case 'i':
    inv(gs.fAlt);
    redraw();
    return 1;

  case 'b':
    inv(gs.fBorder);
    redraw();
    return 1;

  case 'q':
    inv(gs.fThick);
    redraw();
    return 1;

  case 'l':
    inv(gs.fLabel);
    redraw();
    return 1;

  case 'k':
    inv(gs.fLabelAsp);
    redraw();
    return 1;

  case 'j':
    inv(gs.fJetTrail);
    return 1;

  case 'd':
    inv(gs.fHouseExtra);
    redraw();
    return 1;

  case 'e':
    inv(gs.fEquator);
    redraw();
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
    gi.nMode = (gi.nMode == gWheel ? gHouse :
      (gi.nMode == gHouse ? gWheel : gi.nMode));
    redraw();
    return 1;

#ifdef CONSTEL
  case 'F':
    if (gi.nMode != gHorizon && gi.nMode != gSphere &&
        gi.nMode != gGlobe && gi.nMode != gPolar && gi.nMode != gTelescope)
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
    inv(us.fSidereal);
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'h':
    inv(us.objCenter);
    fi.fDoCast = fTrue;
    redraw();
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

  // Object restrictions
  case 'R':
    for (i = oChi; i <= oVes; i++)
      inv(ignore[i]);
    for (i = oSou; i <= oEP; i++)
      inv(ignore[i]);
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'C':
    inv(us.fCusp);
    for (i = cuspLo; i <= cuspHi; i++)
      ignore[i] = !us.fCusp || !ignore[i];
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'u':
    inv(us.fUranian);
    for (i = uranLo; i <= uranHi; i++)
      ignore[i] = !us.fUranian || !ignore[i];
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'y':
    inv(us.fDwarf);
    for (i = dwarfLo; i <= dwarfHi; i++)
      ignore[i] = !us.fDwarf || !ignore[i];
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case '`':
    inv(us.fMoons);
    for (i = moonsLo; i <= moonsHi; i++)
      ignore[i] = !us.fMoons || !ignore[i];
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case '~':
    inv(us.fCOB);
    for (i = cobLo; i <= cobHi; i++)
      ignore[i] = !us.fCOB || !ignore[i];
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  case 'U':
    inv(us.fStar);
    for (i = starLo; i <= starHi; i++)
      ignore[i] = !us.fStar || !ignore[i];
    AdjustRestrictions();
    fi.fDoCast = fTrue;
    redraw();
    return 1;

  // Scale adjustments
  case '<':
    if (gs.nScale > 100) {
      gs.nScale -= 100;
      gi.nScale = gs.nScale / 100;
      redraw();
    }
    return 1;

  case '>':
    if (gs.nScale < MAXSCALE) {
      gs.nScale += 100;
      gi.nScale = gs.nScale / 100;
      redraw();
    }
    return 1;

  // Tilt controls
  case '[':
    gs.rTilt = gs.rTilt > -rDegQuad ?
      gs.rTilt - (real)NAbs(gi.nDir) : -rDegQuad;
    if (gi.nMode == gTelescope)
      gs.objTrack = -1;
    redraw();
    return 1;

  case ']':
    gs.rTilt = gs.rTilt < rDegQuad ?
      gs.rTilt + (real)NAbs(gi.nDir) : rDegQuad;
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
#ifdef TIME
  case 'n':
    Animate(10, 0);
    ciMain = ciCore;
    fi.fDoCast = fTrue;
    redraw();
    return 1;
#endif

  // Toggle continuous animation
  case 'N':
    gs.nAnim = gs.nAnim ? 0 : -iAnimNow;
    return 1;

  // Animation modes (time units)
  case '!': gs.nAnim = -1; return 1;  // Seconds
  case '@': gs.nAnim = -2; return 1;  // Minutes
  case '#': gs.nAnim = -3; return 1;  // Hours
  case '$': gs.nAnim = -4; return 1;  // Days
  case '%': gs.nAnim = -5; return 1;  // Months
  case '^': gs.nAnim = -6; return 1;  // Years
  case '&': gs.nAnim = -7; return 1;  // Decades
  case '*': gs.nAnim = -8; return 1;  // Centuries
  case '(': gs.nAnim = -9; return 1;  // Millennia

  // Chart modes (uppercase letters)
  case 'V': gi.nMode = gWheel;      redraw(); return 1;
  case 'A': gi.nMode = gGrid;       redraw(); return 1;
  case 'Z': gi.nMode = gHorizon;    redraw(); return 1;
  case 'S': gi.nMode = gOrbit;      redraw(); return 1;
  case 'H': gi.nMode = gSector;     redraw(); return 1;
  case 'K': gi.nMode = gCalendar;   redraw(); return 1;
  case 'J': gi.nMode = gDisposit;   redraw(); return 1;
  case 'L': gi.nMode = gAstroGraph; redraw(); return 1;
  case 'E': gi.nMode = gEphemeris;  redraw(); return 1;
  case 'I': gi.nMode = gRising;     redraw(); return 1;
  case 'M': gi.nMode = gMoons;      redraw(); return 1;
  case 'X': gi.nMode = gSphere;     redraw(); return 1;
  case 'W': gi.nMode = gWorldMap;   redraw(); return 1;
  case 'G': gi.nMode = gGlobe;      redraw(); return 1;
  case 'P': gi.nMode = gPolar;      redraw(); return 1;
  case 'T': gi.nMode = gTelescope;  redraw(); return 1;
#ifdef BIORHYTHM
  case 'Y':
    us.nRel = rcBiorhythm;
    gi.nMode = gBiorhythm;
    fi.fDoCast = fTrue;
    redraw();
    return 1;
#endif

  // Quit
  case 'Q':
    SquareX(&gs.xWin, &gs.yWin, fTrue);
    redraw();
    return 1;

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

void ChartWidget::requestResize(int w, int h)
{
  // Request parent to resize
  if (parent()) {
    // Calculate new window size accounting for menu bar
    int newW = w;
    int newH = h + y();  // y() is the offset from menu bar
    window()->resize(window()->x(), window()->y(), newW, newH);
  }
}

// Forward declarations for menu callbacks
void FMenuRestrict(Fl_Widget *w, void *data);
void FMenuRestrictTransit(Fl_Widget *w, void *data);
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
    animating_(false),
    aspectRatio_((double)w / (double)h)
{
  // Set background to black to avoid white gaps during resize
  color(FL_BLACK);

  // Create menu bar
  menubar_ = new Fl_Menu_Bar(0, 0, w, 25);
  createMenus();

  // Create chart widget below menu bar (2D)
  chart_ = new ChartWidget(0, 25, w, h);

#ifdef OPENGL
  // Create OpenGL 3D widget (initially hidden)
  chart3D_ = new Globe3DWidget(0, 25, w, h);
  chart3D_->hide();
#endif

  end();
  resizable(chart_);

  // Store in global state
  fi.window = this;
  fi.chart = chart_;
#ifdef OPENGL
  fi.chart3D = chart3D_;
#endif
  fi.menubar = menubar_;
  fi.xClient = w;
  fi.yClient = h;
}

AstrologWindow::~AstrologWindow()
{
  stopAnimation();
  fi.window = NULL;
  fi.chart = NULL;
#ifdef OPENGL
  fi.chart3D = NULL;
#endif
  fi.menubar = NULL;
}

#ifdef OPENGL
void AstrologWindow::switchTo3D(bool use3D)
{
  if (use3D) {
    // Switch to OpenGL 3D widget
    chart_->hide();
    chart3D_->setChartMode(gi.nMode);
    chart3D_->show();
    chart3D_->redraw();
    resizable(chart3D_);
  } else {
    // Switch to 2D FLTK widget
    chart3D_->hide();
    chart_->show();
    chart_->redraw();
    resizable(chart_);
  }
}
#endif

void AstrologWindow::resize(int x, int y, int w, int h)
{
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
}

void AstrologWindow::timer_callback(void *data)
{
  AstrologWindow *win = (AstrologWindow *)data;
  if (win && win->animating_ && gs.nAnim && !gi.fPause) {
    // Advance animation
    Animate(gs.nAnim, gi.nDir);

    // Redraw
    if (win->chart_)
      win->chart_->redraw();

    // Schedule next frame (30 FPS)
    Fl::repeat_timeout(1.0/30.0, timer_callback, data);
  }
}

void AstrologWindow::startAnimation()
{
  if (!animating_) {
    animating_ = true;
    Fl::add_timeout(1.0/30.0, timer_callback, this);
  }
}

void AstrologWindow::stopAnimation()
{
  animating_ = false;
  Fl::remove_timeout(timer_callback, this);
}

void AstrologWindow::createMenus()
{
  // File menu
  // FL_COMMAND maps to Cmd on macOS, Ctrl on Windows/Linux
  menubar_->add("&File/&Open Chart...", FL_COMMAND+'o', FMenuFileOpen);
  menubar_->add("&File/&Save Chart...", FL_COMMAND+'s', FMenuFileSave);
  menubar_->add("&File/Save &As...", 0, FMenuFileSaveAs);
#ifdef CAIRO
  menubar_->add("&File/Export/&SVG...", 0, FMenuExportSVG);
  menubar_->add("&File/Export/&PDF...", 0, FMenuExportPDF);
#endif
  menubar_->add("&File/Export/&Bitmap...", 0, FMenuExportBitmap);
  menubar_->add("&File/E&xit", FL_COMMAND+'q', FMenuFileExit);

  // Edit menu
  menubar_->add("&Edit/&Copy", FL_COMMAND+'c', FMenuEditCopy);
  menubar_->add("&Edit/&Paste", FL_COMMAND+'v', (Fl_Callback*)NULL);
  menubar_->add("&Edit/Command &Line...", FL_F+2, FMenuCommand);

  // Info menu
  menubar_->add("&Info/Set &Chart Info...", FL_COMMAND+'i', FMenuInfoChart);
  menubar_->add("&Info/Set Chart #&2 Info...", 0, FMenuInfoChart2);

  // View menu - using legacy uppercase key mappings
  menubar_->add("&View/&Wheel Chart", 'V', FMenuViewWheel);
  menubar_->add("&View/&Aspect Grid", 'A', FMenuViewGrid);
  menubar_->add("&View/&Midpoint Grid", 0, FMenuViewMidpoint);
  menubar_->add("&View/&Horizon Chart", 'Z', FMenuViewHorizon);
  menubar_->add("&View/&Orbit Chart", 'S', FMenuViewOrbit);
  menubar_->add("&View/Astro-Graph", 'L', FMenuViewAstroGraph);
  menubar_->add("&View/&Globe", 'G', FMenuViewGlobe);
  menubar_->add("&View/&Sphere", 'X', FMenuViewSphere);
  menubar_->add("&View/&Local Horizon", 0, FMenuViewLocal);
  menubar_->add("&View/&Telescope", 'T', FMenuViewTelescope);
  menubar_->add("&View/&Polar", 'P', FMenuViewPolar);
  menubar_->add("&View/&World Map", 'W', FMenuViewWorldMap);
#ifdef CAIRO
  menubar_->add("&View/Antialiased (&Cairo)", 0, FMenuViewCairo, 0, FL_MENU_TOGGLE|FL_MENU_VALUE);
#endif

  // Settings menu
  menubar_->add("Se&ttings/&Calculation Settings...", 0, FMenuCalcSettings);
  menubar_->add("Se&ttings/&Display Settings...", 0, FMenuDisplaySettings);
  menubar_->add("Se&ttings/&Graphics Settings...", 0, FMenuGraphicsSettings);
  menubar_->add("Se&ttings/&Aspect Settings...", 0, FMenuAspectSettings);
  menubar_->add("Se&ttings/&Object Restrictions...", 0, FMenuRestrict);
  menubar_->add("Se&ttings/&Transit Restrictions...", 0, FMenuRestrictTransit);
  menubar_->add("Se&ttings/Co&lor Settings...", 0, FMenuColorSettings);

  // View menu - add Chart Type
  menubar_->add("&View/Chart &Type...", 0, FMenuChartType);

  // Animate menu
  menubar_->add("&Animate/Animation &Settings...", 0, FMenuAnimSettings);
  menubar_->add("&Animate/&Pause/Play", ' ', FMenuAnimPause);
  menubar_->add("&Animate/&Reverse", 'r', FMenuAnimReverse);
  menubar_->add("&Animate/Jump &Forward", 0, FMenuAnimForward);
  menubar_->add("&Animate/Jump &Back", 0, FMenuAnimBack);

  // Help menu
  menubar_->add("&Help/&About Astrolog...", 0, FMenuHelpAbout);
  menubar_->add("&Help/&Documentation", 0, (Fl_Callback*)NULL);
}

/*
******************************************************************************
** Menu Callbacks
******************************************************************************
*/

void FMenuFileOpen(Fl_Widget *w, void *data)
{
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

void FMenuFileSave(Fl_Widget *w, void *data)
{
  // Would implement save functionality
}

void FMenuFileSaveAs(Fl_Widget *w, void *data)
{
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
void FMenuExportSVG(Fl_Widget *w, void *data)
{
  Fl_File_Chooser chooser(".", "SVG Files (*.svg)",
    Fl_File_Chooser::CREATE, "Export as SVG");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Create SVG surface
    cairo_surface_t *surface = cairo_svg_surface_create(
      chooser.value(), gs.xWin, gs.yWin);

    if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
      // Switch to Cairo backend and render
      InitBackendCairo(surface);
      GBClearScreen(gi.kiOff);
      gi.fFile = fFalse;  // Use screen rendering path
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

void FMenuExportPDF(Fl_Widget *w, void *data)
{
  Fl_File_Chooser chooser(".", "PDF Files (*.pdf)",
    Fl_File_Chooser::CREATE, "Export as PDF");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Create PDF surface
    cairo_surface_t *surface = cairo_pdf_surface_create(
      chooser.value(), gs.xWin, gs.yWin);

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

void FMenuExportBitmap(Fl_Widget *w, void *data)
{
  Fl_File_Chooser chooser(".", "Bitmap Files (*.bmp)",
    Fl_File_Chooser::CREATE, "Export as Bitmap");
  chooser.show();
  while (chooser.shown())
    Fl::wait();

  if (chooser.value()) {
    // Use existing BMP export mechanism
    // TODO: Implement using gs.ft = ftBmp
    fl_message("Bitmap export not yet implemented");
  }
}

void FMenuFileExit(Fl_Widget *w, void *data)
{
  if (fi.window)
    fi.window->hide();
}

void FMenuEditCopy(Fl_Widget *w, void *data)
{
  // Would implement copy to clipboard
}

void FMenuHelpAbout(Fl_Widget *w, void *data)
{
  FShowDlgAbout();
}

void FMenuInfoChart(Fl_Widget *w, void *data)
{
  FShowDlgInfo(1);
}

void FMenuInfoChart2(Fl_Widget *w, void *data)
{
  FShowDlgInfo(2);
}

void FMenuCommand(Fl_Widget *w, void *data)
{
  char szCommand[cchSzMax] = "";
  if (FShowDlgCommand(szCommand, cchSzMax) && szCommand[0]) {
    FProcessCommandLine(szCommand);
    if (fi.chart)
      fi.chart->redraw();
  }
}

void FMenuGraphicsSettings(Fl_Widget *w, void *data)
{
  FShowDlgGraphics();
}

void FMenuCalcSettings(Fl_Widget *w, void *data)
{
  FShowDlgCalc();
}

void FMenuDisplaySettings(Fl_Widget *w, void *data)
{
  FShowDlgDisplay();
}

void FMenuAnimSettings(Fl_Widget *w, void *data)
{
  FShowDlgAnim();
}

void FMenuAspectSettings(Fl_Widget *w, void *data)
{
  FShowDlgAspect();
}

void FMenuRestrict(Fl_Widget *w, void *data)
{
  FShowDlgRestrict(fFalse);
}

void FMenuRestrictTransit(Fl_Widget *w, void *data)
{
  FShowDlgRestrict(fTrue);
}

void FMenuColorSettings(Fl_Widget *w, void *data)
{
  FShowDlgColor();
}

void FMenuChartType(Fl_Widget *w, void *data)
{
  FShowDlgChartType();
}

// View menu callbacks for chart type switching
void FMenuViewWheel(Fl_Widget *w, void *data)
{
  gi.nMode = gWheel;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewGrid(Fl_Widget *w, void *data)
{
  gi.nMode = gGrid;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewMidpoint(Fl_Widget *w, void *data)
{
  gi.nMode = gMidpoint;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewHorizon(Fl_Widget *w, void *data)
{
  gi.nMode = gHorizon;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewOrbit(Fl_Widget *w, void *data)
{
  gi.nMode = gOrbit;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewAstroGraph(Fl_Widget *w, void *data)
{
  gi.nMode = gAstroGraph;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewGlobe(Fl_Widget *w, void *data)
{
  gi.nMode = gGlobe;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(true);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewSphere(Fl_Widget *w, void *data)
{
  gi.nMode = gSphere;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(true);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewLocal(Fl_Widget *w, void *data)
{
  gi.nMode = gLocal;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(true);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewTelescope(Fl_Widget *w, void *data)
{
  gi.nMode = gTelescope;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(true);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewPolar(Fl_Widget *w, void *data)
{
  gi.nMode = gPolar;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(true);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

void FMenuViewWorldMap(Fl_Widget *w, void *data)
{
  gi.nMode = gWorldMap;
  fi.fDoCast = fTrue;
#ifdef OPENGL
  if (fi.window) fi.window->switchTo3D(false);
#else
  if (fi.chart) fi.chart->redraw();
#endif
}

// Animate menu callbacks
void FMenuAnimPause(Fl_Widget *w, void *data)
{
  inv(gi.fPause);
}

void FMenuAnimReverse(Fl_Widget *w, void *data)
{
  neg(gi.nDir);
}

void FMenuAnimForward(Fl_Widget *w, void *data)
{
  Animate(gs.nAnim, gi.nDir);
  fi.fDoCast = fTrue;
  if (fi.chart) fi.chart->redraw();
}

void FMenuAnimBack(Fl_Widget *w, void *data)
{
  Animate(gs.nAnim, -gi.nDir);
  fi.fDoCast = fTrue;
  if (fi.chart) fi.chart->redraw();
}

/*
******************************************************************************
** FLTK Graphics Entry Points
******************************************************************************
*/

void BeginFltk()
{
  // Initialize FLTK scheme for modern look
  Fl::scheme("gtk+");

  // Create the main window
  fi.window = new AstrologWindow(gs.xWin, gs.yWin,
    szAppNameCore " " szVersionCore);

  // Show the window
  fi.window->show();

  // Initialize colors
  InitColorsX();
}

void InteractFltk()
{
  // Main event loop
  if (!fi.window)
    return;

  // Start animation if needed
  if (gs.nAnim)
    fi.window->startAnimation();

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
    }
    if (fi.fDoRedraw) {
      fi.fDoRedraw = fFalse;
      if (fi.chart)
        fi.chart->redraw();
    }

    // Process events
    Fl::wait(0.01);  // 10ms timeout for responsive animation
  }
}

void EndFltk()
{
  // Clean up
  if (fi.window) {
    fi.window->stopAnimation();
    delete fi.window;
    fi.window = NULL;
  }
}

#endif // FLTK
