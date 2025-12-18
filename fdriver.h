/*
** Astrolog (Version 7.80) File: fdriver.h
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

#ifndef _FDRIVER_H
#define _FDRIVER_H

#ifdef FLTK

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_draw.H>

// Forward declarations
class ChartWidget;
class AstrologWindow;

// FLTK-specific global state
// Note: Using int instead of flag since this header is included early
typedef struct _FltkInfo {
  AstrologWindow *window;    // Main application window
  ChartWidget *chart;        // Chart drawing widget
  Fl_Menu_Bar *menubar;      // Menu bar
  int xClient;               // Client area width
  int yClient;               // Client area height
  int fDoResize;             // Resize pending (flag)
  int fDoRedraw;             // Redraw pending (flag)
  int fDoCast;               // Recast chart pending (flag)
  int xMouse;                // Last mouse X position
  int yMouse;                // Last mouse Y position
} FI;

extern FI fi;

// ChartWidget - Custom widget for rendering Astrolog charts
class ChartWidget : public Fl_Widget {
public:
  ChartWidget(int x, int y, int w, int h);
  virtual ~ChartWidget();

  void draw() FL_OVERRIDE;
  int handle(int event) FL_OVERRIDE;

  void requestRedraw() { redraw(); }
  void requestResize(int w, int h);

  int handleKey(int key);

private:
  int mousex_, mousey_;      // Current mouse position
  int buttonx_, buttony_;    // Button press position
};

// AstrologWindow - Main application window
class AstrologWindow : public Fl_Double_Window {
public:
  AstrologWindow(int w, int h, const char *title);
  virtual ~AstrologWindow();

  ChartWidget *chartWidget() { return chart_; }
  Fl_Menu_Bar *menuBar() { return menubar_; }

  void resize(int x, int y, int w, int h) FL_OVERRIDE;

  static void timer_callback(void *data);
  void startAnimation();
  void stopAnimation();

private:
  ChartWidget *chart_;
  Fl_Menu_Bar *menubar_;
  bool animating_;
  double aspectRatio_;    // Initial aspect ratio (width/height)

  void createMenus();
};

// Function declarations
extern void BeginFltk(void);
extern void InteractFltk(void);
extern void EndFltk(void);

// Menu callback declarations
extern void FMenuFileOpen(Fl_Widget *w, void *data);
extern void FMenuFileSave(Fl_Widget *w, void *data);
extern void FMenuFileSaveAs(Fl_Widget *w, void *data);
#ifdef CAIRO
extern void FMenuExportSVG(Fl_Widget *w, void *data);
extern void FMenuExportPDF(Fl_Widget *w, void *data);
#endif
extern void FMenuExportBitmap(Fl_Widget *w, void *data);
extern void FMenuFileExit(Fl_Widget *w, void *data);
extern void FMenuEditCopy(Fl_Widget *w, void *data);
extern void FMenuCommand(Fl_Widget *w, void *data);
extern void FMenuInfoChart(Fl_Widget *w, void *data);
extern void FMenuInfoChart2(Fl_Widget *w, void *data);
extern void FMenuCalcSettings(Fl_Widget *w, void *data);
extern void FMenuDisplaySettings(Fl_Widget *w, void *data);
extern void FMenuGraphicsSettings(Fl_Widget *w, void *data);
extern void FMenuAnimSettings(Fl_Widget *w, void *data);
extern void FMenuAspectSettings(Fl_Widget *w, void *data);
extern void FMenuHelpAbout(Fl_Widget *w, void *data);

// Convert Astrolog color index to FLTK color
extern Fl_Color FltkColorFromKI(int ki);

// Dialog function declarations (fdialog.cpp)
// Note: Using int instead of flag since header is included before typedef
extern void FShowDlgInfo(int nChart);
extern void FShowDlgAbout(void);
extern int FShowDlgCommand(char *szCommand, int cchMax);
extern void FShowDlgGraphics(void);
extern void FShowDlgCalc(void);
extern void FShowDlgDisplay(void);
extern void FShowDlgAnim(void);
extern void FShowDlgAspect(void);
extern void FShowDlgRestrict(int fTransit);
extern void FShowDlgColor(void);
extern void FShowDlgChartType(void);

#endif // FLTK
#endif // _FDRIVER_H
