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
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>


// Forward declarations
class ChartWidget;
class AstrologWindow;
#ifdef OPENGL
class Globe3DWidget;
#endif

// FI is now defined in astrolog_types.h

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
  void showChartPopup();  // Show Chart submenu as context menu
  int mousex_, mousey_;   // Current mouse position
  int buttonx_, buttony_; // Button press position
  flag fMoved_;           // Whether mouse moved during right-click drag
};

// AstrologWindow - Main application window
class AstrologWindow : public Fl_Double_Window {
public:
  AstrologWindow(int w, int h, const char *title);
  virtual ~AstrologWindow();

  ChartWidget *chartWidget() { return chart_; }
#ifdef OPENGL
  Globe3DWidget *chart3DWidget() { return chart3D_; }
  void switchTo3D(bool use3D); // Switch between 2D and 3D widgets
#endif
  Fl_Menu_Bar *menuBar() { return menubar_; }

  void resize(int x, int y, int w, int h) FL_OVERRIDE;
  int handle(int event) FL_OVERRIDE;

  static void timer_callback(void *data);
  void startAnimation();
  void stopAnimation();

private:
  ChartWidget *chart_;
#ifdef OPENGL
  Globe3DWidget *chart3D_;
#endif
  Fl_Menu_Bar *menubar_;
  bool animating_;

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
extern void FMenuViewCairo(Fl_Widget *w, void *data);
#endif
extern void FMenuExportBitmap(Fl_Widget *w, void *data);
extern void FMenuFileExit(Fl_Widget *w, void *data);
extern void FMenuFilePrint(Fl_Widget *w, void *data);
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
extern void FMenuShowTextWindow(Fl_Widget *w, void *data);
extern void RefreshTextWindow(void);

// Convert Astrolog color index to FLTK color
extern Fl_Color FltkColorFromKI(int ki);

// Atlas list browser globals for atlas.cpp FLTK support
#ifdef ATLAS
#include <FL/Fl_Hold_Browser.H>
extern Fl_Hold_Browser *pfbAtlas;
extern int rgAtlasData[];
extern int cAtlasData;
#endif

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
extern void FShowDlgTransit(void);
extern void FShowDlgProgress(void);
extern void FShowDlgChartSettings(void);
extern void FShowDlgDefaultInfo(void);
extern void FShowDlgObject(void);
extern void FShowDlgObject2(void);
extern void FShowDlgStar(void);
extern void FShowDlgInfoAll(void);
extern void FShowDlgList(void);
extern void FShowDlgMoons(void);
extern void FShowDlgMoonObj(void);
extern void FShowDlgCustom(void);
extern void FShowDlgCustomS(void);

#endif // FLTK
#endif // _FDRIVER_H
