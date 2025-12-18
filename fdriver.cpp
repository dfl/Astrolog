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

#ifdef FLTK
#include "fdriver.h"
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>

// Global FLTK state
FI fi = {0};

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
  // Set up clipping region
  fl_push_clip(x(), y(), w(), h());

  // Set the chart size to match the widget
  gs.xWin = w();
  gs.yWin = h();

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

int ChartWidget::handle(int event)
{
  int mx = Fl::event_x() - x();
  int my = Fl::event_y() - y();

  switch (event) {
  case FL_PUSH:
    mousex_ = buttonx_ = mx;
    mousey_ = buttony_ = my;

    if (Fl::event_button() == FL_LEFT_MOUSE) {
      // Left click - draw point or interact with chart
      if (Fl::event_state() & FL_ALT) {
        // Alt+click: relocate chart on map
        if (fMap && !gs.fConstel && !gs.fMollewide) {
          Lon = rDegHalf - Mod((real)(mx - gi.xOffset) /
            (real)gs.xWin * rDegMax - gs.rRot);
          if (Lon < -rDegHalf)
            Lon = -rDegHalf;
          else if (Lon > rDegHalf)
            Lon = rDegHalf;
          Lat = rDegQuad - (real)(my - gi.yOffset) / (real)gs.yWin * rDegHalf;
          if (Lat < -rDegQuad)
            Lat = -rDegQuad;
          else if (Lat > rDegQuad)
            Lat = rDegQuad;
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
      if (us.fGraphics && (fMap || gi.nMode == gMidpoint ||
          gi.nMode == gLocal || gi.nMode == gSphere ||
          gi.nMode == gGlobe || gi.nMode == gPolar || gi.nMode == gTelescope)) {
        gs.rRot += (real)(mx - mousex_) * rDegHalf / (real)gs.xWin *
          (gi.nMode == gLocal || gi.nMode == gTelescope ? -gi.zViewRatio : 1.0);
        gs.rTilt += (real)(my - mousey_) * rDegHalf / (real)gs.yWin *
          (gi.nMode == gLocal || gi.nMode == gTelescope ? gi.zViewRatio :
          (gi.nMode == gGlobe ? -1.0 : 1.0));
        while (gs.rRot >= rDegMax)
          gs.rRot -= rDegMax;
        while (gs.rRot < 0.0)
          gs.rRot += rDegMax;
        while (gs.rTilt > rDegQuad)
          gs.rTilt = rDegQuad;
        while (gs.rTilt < -rDegQuad)
          gs.rTilt = -rDegQuad;
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
        // Zoom in/out
        int nScale = gs.nScale;
        if (dy < 0)
          nScale = nScale * 11 / 10;  // Zoom in
        else
          nScale = nScale * 10 / 11;  // Zoom out
        if (nScale < 100)
          nScale = 100;
        else if (nScale > MAXSCALE)
          nScale = MAXSCALE;
        if (nScale != gs.nScale) {
          gs.nScale = nScale;
          gi.nScale = gs.nScale / 100;
          redraw();
        }
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
#ifdef EXPRESS
  // Allow AstroExpression to adjust the key
  if (!us.fExpOff && FSzSet(us.szExpKey)) {
    ExpSetN(iLetterZ, key);
    ParseExpression(us.szExpKey);
    key = NExpGet(iLetterZ);
  }
#endif

  switch (key) {
  case ' ':
    redraw();
    return 1;

  case 'p':
    inv(gi.fPause);
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

  case 'B':
#ifdef PS
    // Toggle PostScript output mode
    if (gs.ft == ftPS)
      gs.ft = ftNone;
    else
      gs.ft = ftPS;
#endif
    return 1;

  case 'C':
    // Not implemented yet - would show chart info dialog
    return 1;

  case 'Q':
    // Quit
    if (fi.window)
      fi.window->hide();
    return 1;

  case FL_Escape:
    // Also quit on Escape
    if (fi.window)
      fi.window->hide();
    return 1;

  case 'V':
    if (gi.nMode == gWheel)
      gi.nMode = gHouse;
    else if (gi.nMode == gHouse)
      gi.nMode = gWheel;
    redraw();
    return 1;

  case 'v':
    gi.nMode = (gi.nMode == gGrid ? gWheel : gGrid);
    redraw();
    return 1;

  case 'g':
    gi.nMode = (gi.nMode == gGlobe ? gWheel : gGlobe);
    redraw();
    return 1;

  case 'a':
    gi.nMode = (gi.nMode == gAstroGraph ? gWheel : gAstroGraph);
    redraw();
    return 1;

  case 'z':
    gi.nMode = (gi.nMode == gHorizon ? gWheel : gHorizon);
    redraw();
    return 1;

  case 'w':
    gi.nMode = (gi.nMode == gWorldMap ? gWheel : gWorldMap);
    redraw();
    return 1;

  case '+':
  case '=':
    // Zoom in
    gs.nScale = gs.nScale * 11 / 10;
    if (gs.nScale > MAXSCALE)
      gs.nScale = MAXSCALE;
    gi.nScale = gs.nScale / 100;
    redraw();
    return 1;

  case '-':
  case '_':
    // Zoom out
    gs.nScale = gs.nScale * 10 / 11;
    if (gs.nScale < 100)
      gs.nScale = 100;
    gi.nScale = gs.nScale / 100;
    redraw();
    return 1;

  case FL_Left:
    // Navigate left
    gs.rRot = Mod(gs.rRot + (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0));
    redraw();
    return 1;

  case FL_Right:
    // Navigate right
    gs.rRot = Mod(gs.rRot - (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0));
    redraw();
    return 1;

  case FL_Up:
    // Navigate up
    gs.rTilt += (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0);
    if (gs.rTilt > rDegQuad)
      gs.rTilt = rDegQuad;
    redraw();
    return 1;

  case FL_Down:
    // Navigate down
    gs.rTilt -= (Fl::event_state() & FL_SHIFT ? 1.0 : 5.0);
    if (gs.rTilt < -rDegQuad)
      gs.rTilt = -rDegQuad;
    redraw();
    return 1;

  default:
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

/*
******************************************************************************
** AstrologWindow Implementation
******************************************************************************
*/

AstrologWindow::AstrologWindow(int w, int h, const char *title)
  : Fl_Double_Window(w, h + 25, title), animating_(false)
{
  // Create menu bar
  menubar_ = new Fl_Menu_Bar(0, 0, w, 25);
  createMenus();

  // Create chart widget below menu bar
  chart_ = new ChartWidget(0, 25, w, h);

  end();
  resizable(chart_);

  // Store in global state
  fi.window = this;
  fi.chart = chart_;
  fi.menubar = menubar_;
  fi.xClient = w;
  fi.yClient = h;
}

AstrologWindow::~AstrologWindow()
{
  stopAnimation();
  fi.window = NULL;
  fi.chart = NULL;
  fi.menubar = NULL;
}

void AstrologWindow::resize(int x, int y, int w, int h)
{
  Fl_Double_Window::resize(x, y, w, h);

  // Update global state
  fi.xClient = w;
  fi.yClient = h - 25;  // Subtract menu bar height
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
  menubar_->add("&File/&Open Chart...", FL_CTRL+'o', FMenuFileOpen);
  menubar_->add("&File/&Save Chart...", FL_CTRL+'s', FMenuFileSave);
  menubar_->add("&File/Save &As...", 0, FMenuFileSaveAs);
  menubar_->add("&File/Save &Image...", 0, (Fl_Callback*)NULL);
  menubar_->add("&File/E&xit", FL_CTRL+'q', FMenuFileExit);

  // Edit menu
  menubar_->add("&Edit/&Copy", FL_CTRL+'c', FMenuEditCopy);
  menubar_->add("&Edit/&Paste", FL_CTRL+'v', (Fl_Callback*)NULL);
  menubar_->add("&Edit/Command &Line...", FL_F+2, FMenuCommand);

  // Info menu
  menubar_->add("&Info/Set &Chart Info...", FL_CTRL+'i', FMenuInfoChart);
  menubar_->add("&Info/Set Chart #&2 Info...", 0, FMenuInfoChart2);

  // View menu
  menubar_->add("&View/&Wheel Chart", 'v', (Fl_Callback*)NULL);
  menubar_->add("&View/&Aspect Grid", 'g', (Fl_Callback*)NULL);
  menubar_->add("&View/&Midpoint Grid", 0, (Fl_Callback*)NULL);
  menubar_->add("&View/&Horizon Chart", 'z', (Fl_Callback*)NULL);
  menubar_->add("&View/&Orbit Chart", 0, (Fl_Callback*)NULL);
  menubar_->add("&View/&Astro-Graph", 'a', (Fl_Callback*)NULL);
  menubar_->add("&View/&Globe", 0, (Fl_Callback*)NULL);
  menubar_->add("&View/&World Map", 'w', (Fl_Callback*)NULL);

  // Settings menu
  menubar_->add("Se&ttings/&Calculation Settings...", 0, FMenuCalcSettings);
  menubar_->add("Se&ttings/&Display Settings...", 0, FMenuDisplaySettings);
  menubar_->add("Se&ttings/&Graphics Settings...", 0, FMenuGraphicsSettings);
  menubar_->add("Se&ttings/&Aspect Settings...", 0, FMenuAspectSettings);

  // Animate menu
  menubar_->add("&Animate/Animation &Settings...", 0, FMenuAnimSettings);
  menubar_->add("&Animate/&Pause", 'p', (Fl_Callback*)NULL);
  menubar_->add("&Animate/&Reverse", 'r', (Fl_Callback*)NULL);
  menubar_->add("&Animate/Jump &Forward", 0, (Fl_Callback*)NULL);
  menubar_->add("&Animate/Jump &Back", 0, (Fl_Callback*)NULL);

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
