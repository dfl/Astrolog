/*
** Astrolog (Version 7.80) File: fdialog.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** FLTK Dialog implementations for Astrolog
*/

#include "astrolog.h"
#include "xdialog.h"

#ifdef FLTK
#include "fdriver.h"
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/fl_ask.H>

/*
******************************************************************************
** Chart Info Dialog
******************************************************************************
*/

// Dialog data storage for callbacks
static CI s_ciEdit;
static int s_nDlgChart = 1;
static Fl_Window *s_dlgInfo = NULL;
static Fl_Input *s_inName = NULL;
static Fl_Input *s_inLoc = NULL;
static Fl_Choice *s_chMon = NULL;
static Fl_Int_Input *s_inDay = NULL;
static Fl_Int_Input *s_inYea = NULL;
static Fl_Input *s_inTim = NULL;
static Fl_Choice *s_chDst = NULL;
static Fl_Input *s_inZon = NULL;
static Fl_Float_Input *s_inLon = NULL;
static Fl_Float_Input *s_inLat = NULL;

// Update dialog fields from CI struct
static void UpdateInfoFields(CI &ci)
{
  char sz[cchSzMax];

  if (s_inName) s_inName->value(ci.nam ? ci.nam : "");
  if (s_inLoc) s_inLoc->value(ci.loc ? ci.loc : "");

  if (s_chMon && FValidMon(ci.mon))
    s_chMon->value(ci.mon - 1);

  if (s_inDay) {
    sprintf(sz, "%d", ci.day);
    s_inDay->value(sz);
  }

  if (s_inYea) {
    sprintf(sz, "%d", ci.yea);
    s_inYea->value(sz);
  }

  if (s_inTim) {
    sprintf(sz, "%s", SzTim(ci.tim));
    s_inTim->value(sz);
  }

  if (s_chDst) {
    if (ci.dst == 0.0)
      s_chDst->value(0);  // No
    else if (ci.dst == 1.0)
      s_chDst->value(1);  // Yes
    else if (ci.dst == dstAuto)
      s_chDst->value(2);  // Autodetect
    else
      s_chDst->value(0);
  }

  if (s_inZon) {
    sprintf(sz, "%s", SzZone(ci.zon));
    s_inZon->value(sz);
  }

  if (s_inLon) {
    sprintf(sz, "%s", SzLocation(ci.lon, 0.0));
    // Extract just the longitude part
    char *p = strchr(sz, ',');
    if (p) *p = '\0';
    s_inLon->value(sz);
  }

  if (s_inLat) {
    sprintf(sz, "%s", SzLocation(0.0, ci.lat));
    // Extract just the latitude part
    char *p = strchr(sz, ',');
    if (p) p++;
    else p = sz;
    while (*p == ' ') p++;
    s_inLat->value(p);
  }
}

// Read fields from dialog into CI struct
static void ReadInfoFields(CI &ci)
{
  char sz[cchSzMax];

  if (s_chMon)
    ci.mon = s_chMon->value() + 1;

  if (s_inDay) {
    strncpy(sz, s_inDay->value(), sizeof(sz)-1);
    ci.day = NParseSz(sz, pmDay);
  }

  if (s_inYea) {
    strncpy(sz, s_inYea->value(), sizeof(sz)-1);
    ci.yea = NParseSz(sz, pmYea);
  }

  if (s_inTim) {
    strncpy(sz, s_inTim->value(), sizeof(sz)-1);
    ci.tim = RParseSz(sz, pmTim);
  }

  if (s_chDst) {
    int v = s_chDst->value();
    if (v == 0) ci.dst = 0.0;
    else if (v == 1) ci.dst = 1.0;
    else ci.dst = dstAuto;
  }

  if (s_inZon) {
    strncpy(sz, s_inZon->value(), sizeof(sz)-1);
    ci.zon = RParseSz(sz, pmZon);
  }

  if (s_inLon) {
    strncpy(sz, s_inLon->value(), sizeof(sz)-1);
    ci.lon = RParseSz(sz, pmLon);
  }

  if (s_inLat) {
    strncpy(sz, s_inLat->value(), sizeof(sz)-1);
    ci.lat = RParseSz(sz, pmLat);
  }

  if (s_inName)
    ci.nam = SzClone((char *)s_inName->value());

  if (s_inLoc)
    ci.loc = SzClone((char *)s_inLoc->value());
}

// Callback for Now button
static void cb_InfoNow(Fl_Widget *w, void *data)
{
#ifdef TIME
  GetTimeNow(&s_ciEdit.mon, &s_ciEdit.day, &s_ciEdit.yea, &s_ciEdit.tim,
    ciDefa.dst, ciDefa.zon);
  s_ciEdit.dst = ciDefa.dst;
  s_ciEdit.zon = ciDefa.zon;
  s_ciEdit.lon = ciDefa.lon;
  s_ciEdit.lat = ciDefa.lat;
  UpdateInfoFields(s_ciEdit);
#endif
}

// Callback for Set (saved values) button
static void cb_InfoSet(Fl_Widget *w, void *data)
{
  s_ciEdit = ciSave;
  UpdateInfoFields(s_ciEdit);
}

// Callback for OK button
static void cb_InfoOK(Fl_Widget *w, void *data)
{
  const char *szError;

  ReadInfoFields(s_ciEdit);

  // Validate inputs using shared helper
  if (!FValidateCI(&s_ciEdit, &szError)) {
    fl_alert("%s", szError);
    return;
  }

  // Apply changes
  if (s_nDlgChart >= 1) {
    *rgpci[s_nDlgChart] = s_ciEdit;
    if (s_nDlgChart == 1)
      ciCore = s_ciEdit;
  } else {
    is.rgci[-s_nDlgChart] = s_ciEdit;
  }

  // Request chart recast
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgInfo)
    s_dlgInfo->hide();
}

// Callback for Cancel button
static void cb_InfoCancel(Fl_Widget *w, void *data)
{
  if (s_dlgInfo)
    s_dlgInfo->hide();
}

// Show the Chart Info dialog
void FShowDlgInfo(int nChart)
{
  s_nDlgChart = nChart;

  // Get current chart info
  if (nChart >= 1)
    s_ciEdit = *rgpci[nChart];
  else
    s_ciEdit = is.rgci[-nChart];

  // Create dialog window
  int w = 480, h = 340;
  char szTitle[64];
  if (nChart == 1)
    sprintf(szTitle, "Set Chart Info");
  else if (nChart > 1)
    sprintf(szTitle, "Set Chart #%d Info", nChart);
  else
    sprintf(szTitle, "Set Chart List #%d Info", 1 - nChart);

  s_dlgInfo = new Fl_Window(w, h, szTitle);
  s_dlgInfo->begin();

  int y = 10, lw = 80, iw = 130, gap = 30;

  // Name field
  new Fl_Box(10, y, lw, 25, "Name:");
  s_inName = new Fl_Input(10 + lw, y, w - lw - 20, 25);
  y += gap;

  // Location field
  new Fl_Box(10, y, lw, 25, "Location:");
  s_inLoc = new Fl_Input(10 + lw, y, w - lw - 20, 25);
  y += gap + 10;

  // Date row - balanced sizing
  new Fl_Box(10, y, 50, 25, "Month:");
  s_chMon = new Fl_Choice(60, y, 90, 25);
  for (int i = 1; i <= 12; i++)
    s_chMon->add(szMonth[i]);

  new Fl_Box(160, y, 30, 25, "Day:");
  s_inDay = new Fl_Int_Input(190, y, 35, 25);

  new Fl_Box(235, y, 35, 25, "Year:");
  s_inYea = new Fl_Int_Input(270, y, 50, 25);

  new Fl_Box(330, y, 35, 25, "Time:");
  s_inTim = new Fl_Input(365, y, 100, 25);
  y += gap;

  // Zone row
  new Fl_Box(10, y, 50, 25, "DST:");
  s_chDst = new Fl_Choice(60, y, 90, 25);
  s_chDst->add("No");
  s_chDst->add("Yes");
  s_chDst->add("Autodetect");

  new Fl_Box(160, y, 35, 25, "Zone:");
  s_inZon = new Fl_Input(200, y, 80, 25);
  y += gap;

  // Location row
  new Fl_Box(10, y, 70, 25, "Longitude:");
  s_inLon = new Fl_Float_Input(80, y, 110, 25);

  new Fl_Box(200, y, 60, 25, "Latitude:");
  s_inLat = new Fl_Float_Input(260, y, 110, 25);
  y += gap + 10;

  // Helper buttons row
  Fl_Button *btnNow = new Fl_Button(10, y, 80, 25, "Now");
  btnNow->callback(cb_InfoNow);

  Fl_Button *btnSet = new Fl_Button(100, y, 80, 25, "Saved");
  btnSet->callback(cb_InfoSet);
  y += gap + 20;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_InfoOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_InfoCancel);

  s_dlgInfo->end();
  s_dlgInfo->set_modal();

  // Initialize fields with current values
  UpdateInfoFields(s_ciEdit);

  // Default DST to Autodetect for user convenience
  s_chDst->value(2);

  s_dlgInfo->show();

  // Wait for dialog to close
  while (s_dlgInfo->visible())
    Fl::wait();

  delete s_dlgInfo;
  s_dlgInfo = NULL;
  s_inName = s_inLoc = s_inTim = s_inZon = NULL;
  s_inDay = s_inYea = NULL;
  s_inLon = s_inLat = NULL;
  s_chMon = s_chDst = NULL;
}

/*
******************************************************************************
** About Dialog
******************************************************************************
*/

void FShowDlgAbout()
{
  char sz[cchSzMax * 4];
  sprintf(sz, "%s %s\n\n"
    "A free astrology program for Windows, macOS, and Linux.\n\n"
    "%s\n\n"
    "By Walter D. Pullen\n"
    "%s",
    szAppNameCore, szVersionCore, szDateCore, szAddressCore);
  fl_message_title("About Astrolog");
  fl_message("%s", sz);
}

/*
******************************************************************************
** Command Line Dialog
******************************************************************************
*/

static Fl_Window *s_dlgCommand = NULL;
static Fl_Input *s_inCommand = NULL;
static flag s_fCommandOK = fFalse;

static void cb_CommandOK(Fl_Widget *w, void *data)
{
  s_fCommandOK = fTrue;
  if (s_dlgCommand)
    s_dlgCommand->hide();
}

static void cb_CommandCancel(Fl_Widget *w, void *data)
{
  s_fCommandOK = fFalse;
  if (s_dlgCommand)
    s_dlgCommand->hide();
}

flag FShowDlgCommand(char *szCommand, int cchMax)
{
  s_fCommandOK = fFalse;

  int w = 500, h = 120;
  s_dlgCommand = new Fl_Window(w, h, "Enter Command Line");
  s_dlgCommand->begin();

  new Fl_Box(10, 10, w - 20, 25, "Enter Astrolog command switches:");
  s_inCommand = new Fl_Input(10, 40, w - 20, 25);
  s_inCommand->value(szCommand);

  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_CommandOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_CommandCancel);

  s_dlgCommand->end();
  s_dlgCommand->set_modal();
  s_dlgCommand->show();

  while (s_dlgCommand->visible())
    Fl::wait();

  if (s_fCommandOK && s_inCommand) {
    strncpy(szCommand, s_inCommand->value(), cchMax - 1);
    szCommand[cchMax - 1] = '\0';
  }

  delete s_dlgCommand;
  s_dlgCommand = NULL;
  s_inCommand = NULL;

  return s_fCommandOK;
}

/*
******************************************************************************
** Graphics Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgGraphics = NULL;
static Fl_Int_Input *s_inWidth = NULL;
static Fl_Int_Input *s_inHeight = NULL;
static Fl_Int_Input *s_inScale = NULL;
static Fl_Int_Input *s_inScaleText = NULL;
static Fl_Check_Button *s_cbColor = NULL;
static Fl_Check_Button *s_cbInverse = NULL;
static Fl_Check_Button *s_cbThick = NULL;

static void cb_GraphicsOK(Fl_Widget *w, void *data)
{
  char sz[64];
  int n;

  if (s_inWidth) {
    n = atoi(s_inWidth->value());
    if (n >= BITMAPX1 && n <= BITMAPX)
      gs.xWin = n;
  }
  if (s_inHeight) {
    n = atoi(s_inHeight->value());
    if (n >= BITMAPY1 && n <= BITMAPY)
      gs.yWin = n;
  }
  if (s_inScale) {
    n = atoi(s_inScale->value());
    if (n >= 100 && n <= MAXSCALE) {
      gs.nScale = n;
      gi.nScale = n / 100;
    }
  }
  if (s_inScaleText) {
    n = atoi(s_inScaleText->value());
    if (n >= 100 && n <= MAXSCALE) {
      gs.nScaleText = n;
      gi.nScaleText = n / 50;
    }
  }
  if (s_cbColor)
    gs.fColor = s_cbColor->value();
  if (s_cbInverse) {
    flag fNew = s_cbInverse->value();
    if (fNew != gs.fInverse) {
      gs.fInverse = fNew;
      InitColorPalette(gs.fInverse);
    }
  }
  if (s_cbThick)
    gs.fThick = s_cbThick->value();

  fi.fDoResize = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgGraphics)
    s_dlgGraphics->hide();
}

static void cb_GraphicsCancel(Fl_Widget *w, void *data)
{
  if (s_dlgGraphics)
    s_dlgGraphics->hide();
}

void FShowDlgGraphics()
{
  int w = 350, h = 280;
  char sz[64];

  s_dlgGraphics = new Fl_Window(w, h, "Graphics Settings");
  s_dlgGraphics->begin();

  int y = 10, lw = 100;

  // Width
  new Fl_Box(10, y, lw, 25, "Width:");
  s_inWidth = new Fl_Int_Input(10 + lw, y, 80, 25);
  sprintf(sz, "%d", gs.xWin);
  s_inWidth->value(sz);
  y += 30;

  // Height
  new Fl_Box(10, y, lw, 25, "Height:");
  s_inHeight = new Fl_Int_Input(10 + lw, y, 80, 25);
  sprintf(sz, "%d", gs.yWin);
  s_inHeight->value(sz);
  y += 30;

  // Scale
  new Fl_Box(10, y, lw, 25, "Scale (%):");
  s_inScale = new Fl_Int_Input(10 + lw, y, 80, 25);
  sprintf(sz, "%d", gs.nScale);
  s_inScale->value(sz);
  y += 30;

  // Text Scale
  new Fl_Box(10, y, lw, 25, "Text Scale (%):");
  s_inScaleText = new Fl_Int_Input(10 + lw, y, 80, 25);
  sprintf(sz, "%d", gs.nScaleText);
  s_inScaleText->value(sz);
  y += 40;

  // Checkboxes
  s_cbColor = new Fl_Check_Button(10, y, 150, 25, "Color charts");
  s_cbColor->value(gs.fColor);
  y += 25;

  s_cbInverse = new Fl_Check_Button(10, y, 150, 25, "Inverse background");
  s_cbInverse->value(gs.fInverse);
  y += 25;

  s_cbThick = new Fl_Check_Button(10, y, 150, 25, "Thick lines");
  s_cbThick->value(gs.fThick);
  y += 35;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_GraphicsOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_GraphicsCancel);

  s_dlgGraphics->end();
  s_dlgGraphics->set_modal();
  s_dlgGraphics->show();

  while (s_dlgGraphics->visible())
    Fl::wait();

  delete s_dlgGraphics;
  s_dlgGraphics = NULL;
  s_inWidth = s_inHeight = s_inScale = s_inScaleText = NULL;
  s_cbColor = s_cbInverse = s_cbThick = NULL;
}

/*
******************************************************************************
** Calculation Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgCalc = NULL;
static Fl_Choice *s_chHouse = NULL;
static Fl_Check_Button *s_cbSidereal = NULL;
static Fl_Check_Button *s_cbHelio = NULL;
static Fl_Check_Button *s_cbTrueNode = NULL;
static Fl_Input *s_inHarmonic = NULL;
static Fl_Int_Input *s_inDwad = NULL;

static void cb_CalcOK(Fl_Widget *w, void *data)
{
  if (s_chHouse)
    us.nHouseSystem = s_chHouse->value();
  if (s_cbSidereal)
    us.fSidereal = s_cbSidereal->value();
  if (s_cbHelio)
    us.objCenter = s_cbHelio->value() ? oSun : oEar;
  if (s_cbTrueNode)
    us.fTrueNode = s_cbTrueNode->value();

  // Harmonic chart factor - supports 'd' prefix for dial degrees
  if (s_inHarmonic) {
    const char *sz = s_inHarmonic->value();
    int i = (ChCap(sz[0]) == 'D');
    real rx = atof(sz + i);
    if (i != 0 && rx != 0.0)
      rx = rDegMax / rx;
    if (FValidHarmonic(rx))
      us.rHarmonic = rx;
  }

  // Dwad nesting level
  if (s_inDwad) {
    int n = atoi(s_inDwad->value());
    if (FValidDwad(n))
      us.nDwad = n;
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgCalc)
    s_dlgCalc->hide();
}

static void cb_CalcCancel(Fl_Widget *w, void *data)
{
  if (s_dlgCalc)
    s_dlgCalc->hide();
}

void FShowDlgCalc()
{
  int w = 350, h = 290;
  char sz[cchSzDef];

  s_dlgCalc = new Fl_Window(w, h, "Calculation Settings");
  s_dlgCalc->begin();

  int y = 10;

  // House system
  new Fl_Box(10, y, 100, 25, "House System:");
  s_chHouse = new Fl_Choice(120, y, 200, 25);
  for (int i = 0; i < cSystem; i++)
    s_chHouse->add(szSystem[i]);
  s_chHouse->value(us.nHouseSystem);
  y += 35;

  // Harmonic chart factor
  new Fl_Box(10, y, 140, 25, "Harmonic Chart Factor:");
  s_inHarmonic = new Fl_Input(160, y, 80, 25);
  sprintf(sz, "%.6g", us.rHarmonic);
  s_inHarmonic->value(sz);
  s_inHarmonic->tooltip("Enter harmonic value (e.g., 4) or dial degrees with 'd' prefix (e.g., d90)");
  y += 30;

  // Dwad nesting level
  new Fl_Box(10, y, 140, 25, "Dwad Nesting Level:");
  s_inDwad = new Fl_Int_Input(160, y, 80, 25);
  sprintf(sz, "%d", us.nDwad);
  s_inDwad->value(sz);
  y += 35;

  // Checkboxes
  s_cbSidereal = new Fl_Check_Button(10, y, 200, 25, "Sidereal zodiac");
  s_cbSidereal->value(us.fSidereal);
  y += 25;

  s_cbHelio = new Fl_Check_Button(10, y, 200, 25, "Heliocentric positions");
  s_cbHelio->value(us.objCenter != oEar);
  y += 25;

  s_cbTrueNode = new Fl_Check_Button(10, y, 200, 25, "True node (not mean)");
  s_cbTrueNode->value(us.fTrueNode);
  y += 35;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_CalcOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_CalcCancel);

  s_dlgCalc->end();
  s_dlgCalc->set_modal();
  s_dlgCalc->show();

  while (s_dlgCalc->visible())
    Fl::wait();

  delete s_dlgCalc;
  s_dlgCalc = NULL;
  s_chHouse = NULL;
  s_cbSidereal = s_cbHelio = s_cbTrueNode = NULL;
  s_inHarmonic = NULL;
  s_inDwad = NULL;
}

/*
******************************************************************************
** Display Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgDisplay = NULL;
static Fl_Check_Button *s_cbRound = NULL;
static Fl_Check_Button *s_cbEuroDate = NULL;
static Fl_Check_Button *s_cbEuroTime = NULL;
static Fl_Check_Button *s_cbEclipse = NULL;
static Fl_Int_Input *s_inAspects = NULL;
static Fl_Int_Input *s_inScreenWidth = NULL;

static void cb_DisplayOK(Fl_Widget *w, void *data)
{
  if (s_cbRound)
    us.fRound = s_cbRound->value();
  if (s_cbEuroDate)
    us.fEuroDate = s_cbEuroDate->value();
  if (s_cbEuroTime)
    us.fEuroTime = s_cbEuroTime->value();
  if (s_cbEclipse)
    us.fEclipse = s_cbEclipse->value();

  if (s_inAspects) {
    int n = atoi(s_inAspects->value());
    if (FValidAspect(n)) {
      us.nAsp = n;
      for (int i = n + 1; i <= cAspect; i++)
        ignorea[i] = fTrue;
    }
  }

  if (s_inScreenWidth) {
    int n = atoi(s_inScreenWidth->value());
    if (FValidScreen(n))
      us.nScreenWidth = n;
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgDisplay)
    s_dlgDisplay->hide();
}

static void cb_DisplayCancel(Fl_Widget *w, void *data)
{
  if (s_dlgDisplay)
    s_dlgDisplay->hide();
}

void FShowDlgDisplay()
{
  int w = 350, h = 280;
  char sz[64];

  s_dlgDisplay = new Fl_Window(w, h, "Display Settings");
  s_dlgDisplay->begin();

  int y = 10;

  // Aspect count
  new Fl_Box(10, y, 120, 25, "Number of aspects:");
  s_inAspects = new Fl_Int_Input(140, y, 60, 25);
  sprintf(sz, "%d", us.nAsp);
  s_inAspects->value(sz);
  y += 35;

  // Screen width
  new Fl_Box(10, y, 120, 25, "Text columns:");
  s_inScreenWidth = new Fl_Int_Input(140, y, 60, 25);
  sprintf(sz, "%d", us.nScreenWidth);
  s_inScreenWidth->value(sz);
  y += 40;

  // Checkboxes
  s_cbRound = new Fl_Check_Button(10, y, 200, 25, "Round positions to nearest minute");
  s_cbRound->value(us.fRound);
  y += 25;

  s_cbEuroDate = new Fl_Check_Button(10, y, 200, 25, "European date format (DD/MM/YYYY)");
  s_cbEuroDate->value(us.fEuroDate);
  y += 25;

  s_cbEuroTime = new Fl_Check_Button(10, y, 200, 25, "European time format (24 hour)");
  s_cbEuroTime->value(us.fEuroTime);
  y += 25;

  s_cbEclipse = new Fl_Check_Button(10, y, 200, 25, "Show eclipse information");
  s_cbEclipse->value(us.fEclipse);
  y += 35;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_DisplayOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_DisplayCancel);

  s_dlgDisplay->end();
  s_dlgDisplay->set_modal();
  s_dlgDisplay->show();

  while (s_dlgDisplay->visible())
    Fl::wait();

  delete s_dlgDisplay;
  s_dlgDisplay = NULL;
  s_cbRound = s_cbEuroDate = s_cbEuroTime = s_cbEclipse = NULL;
  s_inAspects = s_inScreenWidth = NULL;
}

/*
******************************************************************************
** Animation Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgAnim = NULL;
static Fl_Choice *s_chAnimRate = NULL;
static Fl_Int_Input *s_inAnimFactor = NULL;

static void cb_AnimOK(Fl_Widget *w, void *data)
{
  if (s_chAnimRate) {
    int rate = s_chAnimRate->value();
    // Rates: 0=seconds, 1=minutes, 2=hours, 3=days, 4=months, 5=years
    static int animRates[] = {1, 2, 3, 4, 5, 6};
    if (rate >= 0 && rate < 6)
      gs.nAnim = animRates[rate];
  }

  if (s_inAnimFactor) {
    int n = atoi(s_inAnimFactor->value());
    if (n >= 1 && n <= 1000)
      gi.nDir = n;
  }

  if (s_dlgAnim)
    s_dlgAnim->hide();
}

static void cb_AnimCancel(Fl_Widget *w, void *data)
{
  if (s_dlgAnim)
    s_dlgAnim->hide();
}

void FShowDlgAnim()
{
  int w = 300, h = 160;
  char sz[64];

  s_dlgAnim = new Fl_Window(w, h, "Animation Settings");
  s_dlgAnim->begin();

  int y = 10;

  // Animation rate
  new Fl_Box(10, y, 120, 25, "Animation unit:");
  s_chAnimRate = new Fl_Choice(130, y, 120, 25);
  s_chAnimRate->add("Seconds");
  s_chAnimRate->add("Minutes");
  s_chAnimRate->add("Hours");
  s_chAnimRate->add("Days");
  s_chAnimRate->add("Months");
  s_chAnimRate->add("Years");
  // Set current rate
  int rate = (gs.nAnim >= 1 && gs.nAnim <= 6) ? gs.nAnim - 1 : 3;
  s_chAnimRate->value(rate);
  y += 35;

  // Animation factor
  new Fl_Box(10, y, 120, 25, "Step size:");
  s_inAnimFactor = new Fl_Int_Input(130, y, 60, 25);
  sprintf(sz, "%d", NAbs(gi.nDir));
  s_inAnimFactor->value(sz);
  y += 40;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_AnimOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_AnimCancel);

  s_dlgAnim->end();
  s_dlgAnim->set_modal();
  s_dlgAnim->show();

  while (s_dlgAnim->visible())
    Fl::wait();

  delete s_dlgAnim;
  s_dlgAnim = NULL;
  s_chAnimRate = NULL;
  s_inAnimFactor = NULL;
}

/*
******************************************************************************
** Aspect Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgAspect = NULL;
static Fl_Check_Button *s_cbAsp[cAspect + 1];
static Fl_Float_Input *s_inOrb[cAspect + 1];

static void cb_AspectOK(Fl_Widget *w, void *data)
{
  for (int i = 1; i <= Min(us.nAsp, cAspect); i++) {
    if (s_cbAsp[i])
      ignorea[i] = !s_cbAsp[i]->value();
    if (s_inOrb[i]) {
      real r = atof(s_inOrb[i]->value());
      if (r >= 0.0 && r <= rDegMax)
        rAspOrb[i] = r;
    }
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgAspect)
    s_dlgAspect->hide();
}

static void cb_AspectCancel(Fl_Widget *w, void *data)
{
  if (s_dlgAspect)
    s_dlgAspect->hide();
}

void FShowDlgAspect()
{
  int nAsp = Min(us.nAsp, 18);  // Show up to 18 aspects
  int w = 400, h = 30 * nAsp + 80;
  char sz[64];

  s_dlgAspect = new Fl_Window(w, h, "Aspect Settings");
  s_dlgAspect->begin();

  int y = 10;

  // Header
  new Fl_Box(10, y, 150, 20, "Aspect");
  new Fl_Box(180, y, 80, 20, "Orb");
  y += 25;

  // Aspect rows
  for (int i = 1; i <= nAsp; i++) {
    s_cbAsp[i] = new Fl_Check_Button(10, y, 160, 25, szAspectDisp[i]);
    s_cbAsp[i]->value(!ignorea[i]);

    s_inOrb[i] = new Fl_Float_Input(180, y, 60, 25);
    sprintf(sz, "%.1f", rAspOrb[i]);
    s_inOrb[i]->value(sz);

    y += 25;
  }

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_AspectOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_AspectCancel);

  s_dlgAspect->end();
  s_dlgAspect->set_modal();
  s_dlgAspect->show();

  while (s_dlgAspect->visible())
    Fl::wait();

  delete s_dlgAspect;
  s_dlgAspect = NULL;
  for (int i = 1; i <= nAsp; i++) {
    s_cbAsp[i] = NULL;
    s_inOrb[i] = NULL;
  }
}

/*
******************************************************************************
** Object Restrictions Dialog
******************************************************************************
*/

#define cObjShow 21  // Show main planets + nodes

static Fl_Window *s_dlgRestrict = NULL;
static Fl_Check_Button *s_cbObj[cObjShow + 1];
static flag s_fTransit = fFalse;

static void cb_RestrictAll(Fl_Widget *w, void *data)
{
  for (int i = 0; i <= cObjShow; i++)
    if (s_cbObj[i])
      s_cbObj[i]->value(0);
}

static void cb_RestrictNone(Fl_Widget *w, void *data)
{
  for (int i = 0; i <= cObjShow; i++)
    if (s_cbObj[i])
      s_cbObj[i]->value(1);
}

static void cb_RestrictOK(Fl_Widget *w, void *data)
{
  byte *pb = s_fTransit ? ignore2 : ignore;
  for (int i = 0; i <= cObjShow; i++)
    if (s_cbObj[i])
      pb[i] = !s_cbObj[i]->value();

  AdjustRestrictions();
  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgRestrict)
    s_dlgRestrict->hide();
}

static void cb_RestrictCancel(Fl_Widget *w, void *data)
{
  if (s_dlgRestrict)
    s_dlgRestrict->hide();
}

void FShowDlgRestrict(flag fTransit)
{
  s_fTransit = fTransit;
  byte *pb = fTransit ? ignore2 : ignore;

  int nObj = cObjShow;
  int nCols = 2;
  int nRows = (nObj + 1 + nCols - 1) / nCols;
  int w = 320, h = nRows * 25 + 90;

  s_dlgRestrict = new Fl_Window(w, h,
    fTransit ? "Transit Object Restrictions" : "Object Restrictions");
  s_dlgRestrict->begin();

  int y = 10;
  int colW = (w - 20) / nCols;

  // Object checkboxes
  for (int i = 0; i <= nObj; i++) {
    int col = i / nRows;
    int row = i % nRows;
    int x = 10 + col * colW;
    int yy = y + row * 25;

    s_cbObj[i] = new Fl_Check_Button(x, yy, colW - 10, 25, szObjName[i]);
    s_cbObj[i]->value(!pb[i]);  // Checked means shown (not restricted)
  }

  y += nRows * 25 + 10;

  // Helper buttons
  Fl_Button *btnAll = new Fl_Button(10, y, 70, 25, "Show All");
  btnAll->callback(cb_RestrictAll);

  Fl_Button *btnNone = new Fl_Button(85, y, 70, 25, "Hide All");
  btnNone->callback(cb_RestrictNone);
  y += 35;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_RestrictOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_RestrictCancel);

  s_dlgRestrict->end();
  s_dlgRestrict->set_modal();
  s_dlgRestrict->show();

  while (s_dlgRestrict->visible())
    Fl::wait();

  delete s_dlgRestrict;
  s_dlgRestrict = NULL;
  for (int i = 0; i <= nObj; i++)
    s_cbObj[i] = NULL;
}

/*
******************************************************************************
** Color Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgColor = NULL;
static Fl_Choice *s_chColorItem = NULL;
static Fl_Int_Input *s_inColorR = NULL;
static Fl_Int_Input *s_inColorG = NULL;
static Fl_Int_Input *s_inColorB = NULL;
static int s_nColorIndex = 0;

static void UpdateColorFields()
{
  if (s_nColorIndex >= 0 && s_nColorIndex < cColor) {
    char sz[32];
    KV kv = rgbbmp[s_nColorIndex];
    sprintf(sz, "%d", RgbR(kv));
    s_inColorR->value(sz);
    sprintf(sz, "%d", RgbG(kv));
    s_inColorG->value(sz);
    sprintf(sz, "%d", RgbB(kv));
    s_inColorB->value(sz);
  }
}

static void cb_ColorChoice(Fl_Widget *w, void *data)
{
  if (s_chColorItem) {
    s_nColorIndex = s_chColorItem->value();
    UpdateColorFields();
  }
}

static void cb_ColorOK(Fl_Widget *w, void *data)
{
  if (s_nColorIndex >= 0 && s_nColorIndex < cColor) {
    int r = atoi(s_inColorR->value());
    int g = atoi(s_inColorG->value());
    int b = atoi(s_inColorB->value());
    r = Max(0, Min(255, r));
    g = Max(0, Min(255, g));
    b = Max(0, Min(255, b));
    rgbbmp[s_nColorIndex] = Rgb(r, g, b);
  }

  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgColor)
    s_dlgColor->hide();
}

static void cb_ColorCancel(Fl_Widget *w, void *data)
{
  if (s_dlgColor)
    s_dlgColor->hide();
}

void FShowDlgColor()
{
  int w = 300, h = 200;

  s_dlgColor = new Fl_Window(w, h, "Color Settings");
  s_dlgColor->begin();

  int y = 10;

  // Color selection dropdown
  new Fl_Box(10, y, 60, 25, "Color:");
  s_chColorItem = new Fl_Choice(70, y, 200, 25);
  for (int i = 0; i < cColor; i++)
    s_chColorItem->add(szColor[i]);
  s_chColorItem->value(0);
  s_chColorItem->callback(cb_ColorChoice);
  y += 35;

  // RGB inputs
  new Fl_Box(10, y, 40, 25, "Red:");
  s_inColorR = new Fl_Int_Input(50, y, 60, 25);
  new Fl_Box(120, y, 50, 25, "Green:");
  s_inColorG = new Fl_Int_Input(170, y, 60, 25);
  y += 30;

  new Fl_Box(10, y, 40, 25, "Blue:");
  s_inColorB = new Fl_Int_Input(50, y, 60, 25);
  y += 40;

  // Initialize color fields
  s_nColorIndex = 0;
  UpdateColorFields();

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_ColorOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_ColorCancel);

  s_dlgColor->end();
  s_dlgColor->set_modal();
  s_dlgColor->show();

  while (s_dlgColor->visible())
    Fl::wait();

  delete s_dlgColor;
  s_dlgColor = NULL;
  s_chColorItem = NULL;
  s_inColorR = s_inColorG = s_inColorB = NULL;
}

/*
******************************************************************************
** Chart Type Dialog
******************************************************************************
*/

static Fl_Window *s_dlgChartType = NULL;
static Fl_Choice *s_chChartType = NULL;
static Fl_Check_Button *s_cbChartMono = NULL;
static Fl_Check_Button *s_cbChartGrid = NULL;
static Fl_Check_Button *s_cbChartHouse = NULL;

static void cb_ChartTypeOK(Fl_Widget *w, void *data)
{
  if (s_chChartType) {
    int type = s_chChartType->value();
    // Map choice to chart mode
    switch (type) {
    case 0: gi.nMode = gWheel; break;
    case 1: gi.nMode = gHouse; break;
    case 2: gi.nMode = gGrid; break;
    case 3: gi.nMode = gHorizon; break;
    case 4: gi.nMode = gOrbit; break;
    case 5: gi.nMode = gSector; break;
    case 6: gi.nMode = gAstroGraph; break;
    case 7: gi.nMode = gEphemeris; break;
    case 8: gi.nMode = gWorldMap; break;
    case 9: gi.nMode = gGlobe; break;
    case 10: gi.nMode = gPolar; break;
    case 11: gi.nMode = gTelescope; break;
    }
  }

  if (s_cbChartMono)
    gs.fColor = !s_cbChartMono->value();
  if (s_cbChartGrid)
    us.fGridConfig = s_cbChartGrid->value();
  if (s_cbChartHouse)
    us.fWheelReverse = s_cbChartHouse->value();

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgChartType)
    s_dlgChartType->hide();
}

static void cb_ChartTypeCancel(Fl_Widget *w, void *data)
{
  if (s_dlgChartType)
    s_dlgChartType->hide();
}

void FShowDlgChartType()
{
  int w = 300, h = 230;

  s_dlgChartType = new Fl_Window(w, h, "Chart Type");
  s_dlgChartType->begin();

  int y = 10;

  // Chart type dropdown
  new Fl_Box(10, y, 80, 25, "Chart Type:");
  s_chChartType = new Fl_Choice(90, y, 180, 25);
  s_chChartType->add("Standard Wheel");
  s_chChartType->add("House Wheel");
  s_chChartType->add("Aspect Grid");
  s_chChartType->add("Horizon Chart");
  s_chChartType->add("Orbit Chart");
  s_chChartType->add("Sector Chart");
  s_chChartType->add("Astro-Graph");
  s_chChartType->add("Ephemeris");
  s_chChartType->add("World Map");
  s_chChartType->add("Globe");
  s_chChartType->add("Polar Globe");
  s_chChartType->add("Telescope");

  // Set current mode
  int current = 0;
  switch (gi.nMode) {
  case gWheel: current = 0; break;
  case gHouse: current = 1; break;
  case gGrid: current = 2; break;
  case gHorizon: current = 3; break;
  case gOrbit: current = 4; break;
  case gSector: current = 5; break;
  case gAstroGraph: current = 6; break;
  case gEphemeris: current = 7; break;
  case gWorldMap: current = 8; break;
  case gGlobe: current = 9; break;
  case gPolar: current = 10; break;
  case gTelescope: current = 11; break;
  }
  s_chChartType->value(current);
  y += 40;

  // Options
  s_cbChartMono = new Fl_Check_Button(10, y, 200, 25, "Monochrome");
  s_cbChartMono->value(!gs.fColor);
  y += 25;

  s_cbChartGrid = new Fl_Check_Button(10, y, 200, 25, "Include aspects in grid");
  s_cbChartGrid->value(us.fGridConfig);
  y += 25;

  s_cbChartHouse = new Fl_Check_Button(10, y, 200, 25, "Reverse wheel direction");
  s_cbChartHouse->value(us.fWheelReverse);
  y += 35;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_ChartTypeOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_ChartTypeCancel);

  s_dlgChartType->end();
  s_dlgChartType->set_modal();
  s_dlgChartType->show();

  while (s_dlgChartType->visible())
    Fl::wait();

  delete s_dlgChartType;
  s_dlgChartType = NULL;
  s_chChartType = NULL;
  s_cbChartMono = s_cbChartGrid = s_cbChartHouse = NULL;
}

/*
******************************************************************************
** Transits Dialog
******************************************************************************
*/

static Fl_Window *s_dlgTransit = NULL;
static Fl_Choice *s_chTrMon = NULL;
static Fl_Int_Input *s_inTrDay = NULL;
static Fl_Int_Input *s_inTrYea = NULL;
static Fl_Input *s_inTrTim = NULL;
static Fl_Choice *s_chTrType = NULL;
static Fl_Choice *s_chTrSpan = NULL;
static Fl_Int_Input *s_inTrDiv = NULL;

static void cb_TransitOK(Fl_Widget *w, void *data)
{
  // Get date/time
  int mon = s_chTrMon ? s_chTrMon->value() + 1 : MonT;
  int day = s_inTrDay ? atoi(s_inTrDay->value()) : DayT;
  int yea = s_inTrYea ? atoi(s_inTrYea->value()) : YeaT;
  real tim = s_inTrTim ? atof(s_inTrTim->value()) : TimT;
  int div = s_inTrDiv ? atoi(s_inTrDiv->value()) : us.nDivision;

  // Validate
  if (!FValidMon(mon) || !FValidYea(yea) || !FValidDay(day, mon, yea) ||
      !FValidTim(tim) || !FValidDivision(div)) {
    fl_alert("Invalid date, time, or division value");
    return;
  }

  // Set transit chart info
  SetCI(ciTran, mon, day, yea, tim, ciDefa.dst, ciDefa.zon, ciDefa.lon, ciDefa.lat);
  us.nDivision = div;

  // Set transit type based on selection
  int type = s_chTrType ? s_chTrType->value() : 0;
  int span = s_chTrSpan ? s_chTrSpan->value() : 0;

  // Clear all transit flags first
  us.fInDay = us.fInDayInf = us.fInDayGra = fFalse;
  us.fTransit = us.fTransitInf = us.fTransitGra = fFalse;

  // Set span flags
  us.fInDayMonth = span >= 1;
  us.fInDayYear = span >= 2;

  // Set chart mode based on type
  switch (type) {
  case 0:  // Transit Hits
    us.fInDay = fTrue;
    gi.nMode = gTraTraTim;
    us.fGraphics = fFalse;
    break;
  case 1:  // Transit Influence
    us.fInDayInf = fTrue;
    gi.nMode = gTraTraInf;
    break;
  case 2:  // Transit Graph
    us.fInDayGra = fTrue;
    gi.nMode = gTraTraGra;
    break;
  case 3:  // Transit to Natal Hits
    us.fTransit = fTrue;
    gi.nMode = gTraNatTim;
    us.fGraphics = fFalse;
    break;
  case 4:  // Transit to Natal Influence
    us.fTransitInf = fTrue;
    gi.nMode = gTraNatInf;
    break;
  case 5:  // Transit to Natal Graph
    us.fTransitGra = fTrue;
    gi.nMode = gTraNatGra;
    break;
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgTransit)
    s_dlgTransit->hide();
}

static void cb_TransitNow(Fl_Widget *w, void *data)
{
#ifdef TIME
  int mon, day, yea;
  real tim;
  char sz[32];

  GetTimeNow(&mon, &day, &yea, &tim, ciDefa.dst, ciDefa.zon);
  if (s_chTrMon) s_chTrMon->value(mon - 1);
  if (s_inTrDay) { sprintf(sz, "%d", day); s_inTrDay->value(sz); }
  if (s_inTrYea) { sprintf(sz, "%d", yea); s_inTrYea->value(sz); }
  if (s_inTrTim) { sprintf(sz, "%.2f", tim); s_inTrTim->value(sz); }
#endif
}

static void cb_TransitCancel(Fl_Widget *w, void *data)
{
  if (s_dlgTransit)
    s_dlgTransit->hide();
}

void FShowDlgTransit()
{
  int w = 380, h = 300;
  char sz[64];

  s_dlgTransit = new Fl_Window(w, h, "Transits");
  s_dlgTransit->begin();

  int y = 10;

  // Date row
  new Fl_Box(10, y, 50, 25, "Date:");
  s_chTrMon = new Fl_Choice(60, y, 90, 25);
  for (int i = 1; i <= cSign; i++)
    s_chTrMon->add(szMonth[i]);
  s_chTrMon->value(MonT - 1);

  s_inTrDay = new Fl_Int_Input(155, y, 40, 25);
  sprintf(sz, "%d", DayT);
  s_inTrDay->value(sz);

  s_inTrYea = new Fl_Int_Input(200, y, 60, 25);
  sprintf(sz, "%d", YeaT);
  s_inTrYea->value(sz);

  Fl_Button *btnNow = new Fl_Button(270, y, 60, 25, "Now");
  btnNow->callback(cb_TransitNow);
  y += 35;

  // Time row
  new Fl_Box(10, y, 50, 25, "Time:");
  s_inTrTim = new Fl_Input(60, y, 80, 25);
  sprintf(sz, "%.2f", TimT);
  s_inTrTim->value(sz);
  y += 40;

  // Transit type
  new Fl_Box(10, y, 100, 25, "Transit Type:");
  s_chTrType = new Fl_Choice(115, y, 200, 25);
  s_chTrType->add("Transit Hits");
  s_chTrType->add("Transit Influence");
  s_chTrType->add("Transit Graph");
  s_chTrType->add("Transit to Natal Hits");
  s_chTrType->add("Transit to Natal Influence");
  s_chTrType->add("Transit to Natal Graph");
  s_chTrType->value(0);
  y += 35;

  // Time span
  new Fl_Box(10, y, 100, 25, "Time Span:");
  s_chTrSpan = new Fl_Choice(115, y, 120, 25);
  s_chTrSpan->add("One Day");
  s_chTrSpan->add("One Month");
  s_chTrSpan->add("One Year");
  s_chTrSpan->value(0);
  y += 35;

  // Division
  new Fl_Box(10, y, 100, 25, "Divisions:");
  s_inTrDiv = new Fl_Int_Input(115, y, 60, 25);
  sprintf(sz, "%d", us.nDivision);
  s_inTrDiv->value(sz);
  y += 40;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_TransitOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_TransitCancel);

  s_dlgTransit->end();
  s_dlgTransit->set_modal();
  s_dlgTransit->show();

  while (s_dlgTransit->visible())
    Fl::wait();

  delete s_dlgTransit;
  s_dlgTransit = NULL;
  s_chTrMon = NULL;
  s_inTrDay = s_inTrYea = s_inTrDiv = NULL;
  s_inTrTim = NULL;
  s_chTrType = s_chTrSpan = NULL;
}

/*
******************************************************************************
** Progressions Dialog
******************************************************************************
*/

static Fl_Window *s_dlgProgress = NULL;
static Fl_Choice *s_chPrMon = NULL;
static Fl_Int_Input *s_inPrDay = NULL;
static Fl_Int_Input *s_inPrYea = NULL;
static Fl_Input *s_inPrTim = NULL;
static Fl_Choice *s_chPrType = NULL;
static Fl_Float_Input *s_inPrCusp = NULL;

static void cb_ProgressOK(Fl_Widget *w, void *data)
{
  // Get date/time
  int mon = s_chPrMon ? s_chPrMon->value() + 1 : MonT;
  int day = s_inPrDay ? atoi(s_inPrDay->value()) : DayT;
  int yea = s_inPrYea ? atoi(s_inPrYea->value()) : YeaT;
  real tim = s_inPrTim ? atof(s_inPrTim->value()) : TimT;
  real cusp = s_inPrCusp ? atof(s_inPrCusp->value()) : us.rProgCusp;

  // Validate
  if (!FValidMon(mon) || !FValidYea(yea) || !FValidDay(day, mon, yea) ||
      !FValidTim(tim)) {
    fl_alert("Invalid date or time value");
    return;
  }

  // Set progression chart info
  SetCI(ciTran, mon, day, yea, tim, ciDefa.dst, ciDefa.zon, ciDefa.lon, ciDefa.lat);
  us.rProgCusp = cusp;

  // Set progression type
  int type = s_chPrType ? s_chPrType->value() : 0;
  us.fProgress = fTrue;
  us.nProgress = (type == 0) ? ptCast : ((type == 1) ? ptSolarArc : ptMixed);

  // Set relationship mode
  us.nRel = rcProgress;

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgProgress)
    s_dlgProgress->hide();
}

static void cb_ProgressNow(Fl_Widget *w, void *data)
{
#ifdef TIME
  int mon, day, yea;
  real tim;
  char sz[32];

  GetTimeNow(&mon, &day, &yea, &tim, ciDefa.dst, ciDefa.zon);
  if (s_chPrMon) s_chPrMon->value(mon - 1);
  if (s_inPrDay) { sprintf(sz, "%d", day); s_inPrDay->value(sz); }
  if (s_inPrYea) { sprintf(sz, "%d", yea); s_inPrYea->value(sz); }
  if (s_inPrTim) { sprintf(sz, "%.2f", tim); s_inPrTim->value(sz); }
#endif
}

static void cb_ProgressCancel(Fl_Widget *w, void *data)
{
  if (s_dlgProgress)
    s_dlgProgress->hide();
}

void FShowDlgProgress()
{
  int w = 380, h = 260;
  char sz[64];

  s_dlgProgress = new Fl_Window(w, h, "Progressions");
  s_dlgProgress->begin();

  int y = 10;

  // Date row
  new Fl_Box(10, y, 80, 25, "Progress to:");
  s_chPrMon = new Fl_Choice(95, y, 90, 25);
  for (int i = 1; i <= cSign; i++)
    s_chPrMon->add(szMonth[i]);
  s_chPrMon->value(MonT - 1);

  s_inPrDay = new Fl_Int_Input(190, y, 40, 25);
  sprintf(sz, "%d", DayT);
  s_inPrDay->value(sz);

  s_inPrYea = new Fl_Int_Input(235, y, 60, 25);
  sprintf(sz, "%d", YeaT);
  s_inPrYea->value(sz);

  Fl_Button *btnNow = new Fl_Button(305, y, 60, 25, "Now");
  btnNow->callback(cb_ProgressNow);
  y += 35;

  // Time row
  new Fl_Box(10, y, 50, 25, "Time:");
  s_inPrTim = new Fl_Input(95, y, 80, 25);
  sprintf(sz, "%.2f", TimT);
  s_inPrTim->value(sz);
  y += 40;

  // Progression type
  new Fl_Box(10, y, 100, 25, "Progression Type:");
  s_chPrType = new Fl_Choice(120, y, 180, 25);
  s_chPrType->add("Secondary (Day for Year)");
  s_chPrType->add("Solar Arc");
  s_chPrType->add("Mixed (Solar + Lunar)");
  s_chPrType->value(us.nProgress);
  y += 35;

  // Cusp progression factor
  new Fl_Box(10, y, 120, 25, "Cusp Prog Factor:");
  s_inPrCusp = new Fl_Float_Input(135, y, 80, 25);
  sprintf(sz, "%.6g", us.rProgCusp);
  s_inPrCusp->value(sz);
  y += 40;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_ProgressOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_ProgressCancel);

  s_dlgProgress->end();
  s_dlgProgress->set_modal();
  s_dlgProgress->show();

  while (s_dlgProgress->visible())
    Fl::wait();

  delete s_dlgProgress;
  s_dlgProgress = NULL;
  s_chPrMon = NULL;
  s_inPrDay = s_inPrYea = NULL;
  s_inPrTim = NULL;
  s_chPrType = NULL;
  s_inPrCusp = NULL;
}

/*
******************************************************************************
** Chart Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgChartSettings = NULL;
static Fl_Check_Button *s_chVelocity = NULL;
static Fl_Int_Input *s_inWheelRows = NULL;
static Fl_Check_Button *s_chWheelReverse = NULL;
static Fl_Check_Button *s_chGridConfig = NULL;
static Fl_Check_Button *s_chGridMidpoint = NULL;
static Fl_Check_Button *s_chAspSummary = NULL;
static Fl_Check_Button *s_chMidSummary = NULL;
static Fl_Check_Button *s_chMidAspect = NULL;
static Fl_Check_Button *s_chPrimeVert = NULL;
static Fl_Check_Button *s_chSectorApprox = NULL;
static Fl_Check_Button *s_chCalendarYear = NULL;
static Fl_Check_Button *s_chInfluenceSign = NULL;
static Fl_Int_Input *s_inAstroStep = NULL;
static Fl_Check_Button *s_chLatCross = NULL;
static Fl_Check_Button *s_chEphemYear = NULL;
static Fl_Int_Input *s_inArabicParts = NULL;
static Fl_Check_Button *s_chArabicFlip = NULL;
static Fl_Int_Input *s_inNearestCity = NULL;
static Fl_Int_Input *s_inBioday = NULL;

static void cb_ChartSettingsOK(Fl_Widget *w, void *data)
{
  if (s_inWheelRows) {
    int n = atoi(s_inWheelRows->value());
    if (FValidWheel(n))
      us.nWheelRows = n;
  }
  if (s_inAstroStep) {
    int n = atoi(s_inAstroStep->value());
    if (FValidAstrograph(n))
      us.nAstroGraphStep = n;
  }
  if (s_inArabicParts) {
    int n = atoi(s_inArabicParts->value());
    if (FValidPart(n))
      us.nArabicParts = n;
  }
  if (s_inNearestCity) {
    int n = atoi(s_inNearestCity->value());
    if (n >= 0)
      us.nAtlasList = n;
  }
  if (s_inBioday) {
    int n = atoi(s_inBioday->value());
    if (FValidBioday(n))
      us.nBioday = n;
  }

  us.fVelocity = s_chVelocity ? s_chVelocity->value() : us.fVelocity;
  us.fWheelReverse = s_chWheelReverse ? s_chWheelReverse->value() : us.fWheelReverse;
  us.fGridConfig = s_chGridConfig ? s_chGridConfig->value() : us.fGridConfig;
  us.fGridMidpoint = s_chGridMidpoint ? s_chGridMidpoint->value() : us.fGridMidpoint;
  us.fAspSummary = s_chAspSummary ? s_chAspSummary->value() : us.fAspSummary;
  us.fMidSummary = s_chMidSummary ? s_chMidSummary->value() : us.fMidSummary;
  us.fMidAspect = s_chMidAspect ? s_chMidAspect->value() : us.fMidAspect;
  us.fPrimeVert = s_chPrimeVert ? s_chPrimeVert->value() : us.fPrimeVert;
  us.fSectorApprox = s_chSectorApprox ? s_chSectorApprox->value() : us.fSectorApprox;
  us.fCalendarYear = s_chCalendarYear ? s_chCalendarYear->value() : us.fCalendarYear;
  us.fInfluenceSign = s_chInfluenceSign ? s_chInfluenceSign->value() : us.fInfluenceSign;
  us.fLatitudeCross = s_chLatCross ? s_chLatCross->value() : us.fLatitudeCross;
  us.nEphemYears = s_chEphemYear ? s_chEphemYear->value() : us.nEphemYears;
  us.fArabicFlip = s_chArabicFlip ? s_chArabicFlip->value() : us.fArabicFlip;

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgChartSettings)
    s_dlgChartSettings->hide();
}

static void cb_ChartSettingsCancel(Fl_Widget *w, void *data)
{
  if (s_dlgChartSettings)
    s_dlgChartSettings->hide();
}

void FShowDlgChartSettings()
{
  int w = 420, h = 440;
  char sz[64];

  s_dlgChartSettings = new Fl_Window(w, h, "Chart Settings");
  s_dlgChartSettings->begin();

  int y = 10;
  int col1 = 10, col2 = 210;

  // Column 1 - Display options
  s_chVelocity = new Fl_Check_Button(col1, y, 190, 25, "Show Velocity in Listing");
  s_chVelocity->value(us.fVelocity);
  y += 25;

  s_chWheelReverse = new Fl_Check_Button(col1, y, 190, 25, "Reverse Wheel Rotation");
  s_chWheelReverse->value(us.fWheelReverse);
  y += 25;

  s_chGridConfig = new Fl_Check_Button(col1, y, 190, 25, "Grid Config Display");
  s_chGridConfig->value(us.fGridConfig);
  y += 25;

  s_chGridMidpoint = new Fl_Check_Button(col1, y, 190, 25, "Grid Midpoints");
  s_chGridMidpoint->value(us.fGridMidpoint);
  y += 25;

  s_chAspSummary = new Fl_Check_Button(col1, y, 190, 25, "Aspect Summary");
  s_chAspSummary->value(us.fAspSummary);
  y += 25;

  s_chMidSummary = new Fl_Check_Button(col1, y, 190, 25, "Midpoint Summary");
  s_chMidSummary->value(us.fMidSummary);
  y += 25;

  s_chMidAspect = new Fl_Check_Button(col1, y, 190, 25, "Midpoint Aspects");
  s_chMidAspect->value(us.fMidAspect);
  y += 25;

  s_chPrimeVert = new Fl_Check_Button(col1, y, 190, 25, "Prime Vertical");
  s_chPrimeVert->value(us.fPrimeVert);
  y += 25;

  s_chSectorApprox = new Fl_Check_Button(col1, y, 190, 25, "Sector Approximation");
  s_chSectorApprox->value(us.fSectorApprox);
  y += 25;

  s_chCalendarYear = new Fl_Check_Button(col1, y, 190, 25, "Calendar Year Mode");
  s_chCalendarYear->value(us.fCalendarYear);
  y += 25;

  s_chInfluenceSign = new Fl_Check_Button(col1, y, 190, 25, "Influence By Sign");
  s_chInfluenceSign->value(us.fInfluenceSign);
  y += 25;

  s_chLatCross = new Fl_Check_Button(col1, y, 190, 25, "Show Latitude Crossings");
  s_chLatCross->value(us.fLatitudeCross);
  y += 25;

  s_chEphemYear = new Fl_Check_Button(col1, y, 190, 25, "Ephemeris Year Mode");
  s_chEphemYear->value(us.nEphemYears != 0);
  y += 25;

  s_chArabicFlip = new Fl_Check_Button(col1, y, 190, 25, "Flip Arabic Parts");
  s_chArabicFlip->value(us.fArabicFlip);

  // Column 2 - Number inputs
  y = 10;
  new Fl_Box(col2, y, 100, 25, "Wheel Rows:");
  s_inWheelRows = new Fl_Int_Input(col2 + 105, y, 50, 25);
  sprintf(sz, "%d", us.nWheelRows);
  s_inWheelRows->value(sz);
  y += 35;

  new Fl_Box(col2, y, 100, 25, "Astro Step:");
  s_inAstroStep = new Fl_Int_Input(col2 + 105, y, 50, 25);
  sprintf(sz, "%d", us.nAstroGraphStep);
  s_inAstroStep->value(sz);
  y += 35;

  new Fl_Box(col2, y, 100, 25, "Arabic Parts:");
  s_inArabicParts = new Fl_Int_Input(col2 + 105, y, 50, 25);
  sprintf(sz, "%d", us.nArabicParts);
  s_inArabicParts->value(sz);
  y += 35;

  new Fl_Box(col2, y, 100, 25, "Nearest Cities:");
  s_inNearestCity = new Fl_Int_Input(col2 + 105, y, 50, 25);
  sprintf(sz, "%d", us.nAtlasList);
  s_inNearestCity->value(sz);
  y += 35;

  new Fl_Box(col2, y, 100, 25, "Biorhythm Days:");
  s_inBioday = new Fl_Int_Input(col2 + 105, y, 50, 25);
  sprintf(sz, "%d", us.nBioday);
  s_inBioday->value(sz);

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_ChartSettingsOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_ChartSettingsCancel);

  s_dlgChartSettings->end();
  s_dlgChartSettings->set_modal();
  s_dlgChartSettings->show();

  while (s_dlgChartSettings->visible())
    Fl::wait();

  delete s_dlgChartSettings;
  s_dlgChartSettings = NULL;
  s_chVelocity = NULL;
  s_inWheelRows = NULL;
  s_chWheelReverse = NULL;
  s_chGridConfig = NULL;
  s_chGridMidpoint = NULL;
  s_chAspSummary = NULL;
  s_chMidSummary = NULL;
  s_chMidAspect = NULL;
  s_chPrimeVert = NULL;
  s_chSectorApprox = NULL;
  s_chCalendarYear = NULL;
  s_chInfluenceSign = NULL;
  s_inAstroStep = NULL;
  s_chLatCross = NULL;
  s_chEphemYear = NULL;
  s_inArabicParts = NULL;
  s_chArabicFlip = NULL;
  s_inNearestCity = NULL;
  s_inBioday = NULL;
}

/*
******************************************************************************
** Default Chart Info Dialog
******************************************************************************
*/

static Fl_Window *s_dlgDefaultInfo = NULL;
static Fl_Input *s_inDefDst = NULL;
static Fl_Input *s_inDefZon = NULL;
static Fl_Input *s_inDefLon = NULL;
static Fl_Input *s_inDefLat = NULL;
static Fl_Input *s_inDefNam = NULL;
static Fl_Input *s_inDefLoc = NULL;

static void cb_DefaultInfoOK(Fl_Widget *w, void *data)
{
  real dst, zon, lon, lat;
  const char *sz;

  // Parse the input values
  sz = s_inDefDst ? s_inDefDst->value() : "";
  dst = RParseSz(sz, pmDst);
  if (!FValidDst(dst)) {
    fl_alert("Invalid daylight saving value");
    return;
  }

  sz = s_inDefZon ? s_inDefZon->value() : "";
  zon = RParseSz(sz, pmZon);
  if (!FValidZon(zon)) {
    fl_alert("Invalid time zone value");
    return;
  }

  sz = s_inDefLon ? s_inDefLon->value() : "";
  lon = RParseSz(sz, pmLon);
  if (!FValidLon(lon)) {
    fl_alert("Invalid longitude value");
    return;
  }

  sz = s_inDefLat ? s_inDefLat->value() : "";
  lat = RParseSz(sz, pmLat);
  if (!FValidLat(lat)) {
    fl_alert("Invalid latitude value");
    return;
  }

  // Apply values to default chart info
  ciDefa.dst = dst;
  ciDefa.zon = zon;
  ciDefa.lon = lon;
  ciDefa.lat = lat;
  if (s_inDefNam)
    ciDefa.nam = SzClone((char *)s_inDefNam->value());
  if (s_inDefLoc)
    ciDefa.loc = SzClone((char *)s_inDefLoc->value());

  if (s_dlgDefaultInfo)
    s_dlgDefaultInfo->hide();
}

static void cb_DefaultInfoCancel(Fl_Widget *w, void *data)
{
  if (s_dlgDefaultInfo)
    s_dlgDefaultInfo->hide();
}

void FShowDlgDefaultInfo()
{
  int w = 400, h = 280;
  char sz[cchSzDef];

  s_dlgDefaultInfo = new Fl_Window(w, h, "Default Chart Info");
  s_dlgDefaultInfo->begin();

  int y = 10, lw = 100;

  // DST row
  new Fl_Box(10, y, lw, 25, "Daylight Saving:");
  s_inDefDst = new Fl_Input(115, y, 120, 25);
  sprintf(sz, "%s", SzZone(ciDefa.dst));
  s_inDefDst->value(sz);
  y += 35;

  // Time zone row
  new Fl_Box(10, y, lw, 25, "Time Zone:");
  s_inDefZon = new Fl_Input(115, y, 120, 25);
  sprintf(sz, "%s", SzZone(ciDefa.zon));
  s_inDefZon->value(sz);
  y += 35;

  // Longitude row
  new Fl_Box(10, y, lw, 25, "Longitude:");
  s_inDefLon = new Fl_Input(115, y, 160, 25);
  sprintf(sz, "%s", SzLocation(ciDefa.lon, ciDefa.lat));
  // Extract just longitude from location string
  char *p = strchr(sz, ' ');
  if (p) *p = '\0';
  s_inDefLon->value(sz);
  y += 35;

  // Latitude row
  new Fl_Box(10, y, lw, 25, "Latitude:");
  s_inDefLat = new Fl_Input(115, y, 160, 25);
  sprintf(sz, "%s", SzLocation(ciDefa.lon, ciDefa.lat));
  p = strchr(sz, ' ');
  s_inDefLat->value(p ? p + 1 : "");
  y += 35;

  // Name row
  new Fl_Box(10, y, lw, 25, "Name:");
  s_inDefNam = new Fl_Input(115, y, 270, 25);
  s_inDefNam->value(ciDefa.nam ? ciDefa.nam : "");
  y += 35;

  // Location row
  new Fl_Box(10, y, lw, 25, "Location:");
  s_inDefLoc = new Fl_Input(115, y, 270, 25);
  s_inDefLoc->value(ciDefa.loc ? ciDefa.loc : "");

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_DefaultInfoOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_DefaultInfoCancel);

  s_dlgDefaultInfo->end();
  s_dlgDefaultInfo->set_modal();
  s_dlgDefaultInfo->show();

  while (s_dlgDefaultInfo->visible())
    Fl::wait();

  delete s_dlgDefaultInfo;
  s_dlgDefaultInfo = NULL;
  s_inDefDst = NULL;
  s_inDefZon = NULL;
  s_inDefLon = NULL;
  s_inDefLat = NULL;
  s_inDefNam = NULL;
  s_inDefLoc = NULL;
}

/*
******************************************************************************
** Object Settings Dialog
******************************************************************************
*/

#include <FL/Fl_Scroll.H>

static Fl_Window *s_dlgObject = NULL;
static Fl_Float_Input *s_inObjOrb[oCore+1];
static Fl_Float_Input *s_inObjAdd[oCore+1];
static Fl_Float_Input *s_inObjInf[oCore+1];

static void cb_ObjectOK(Fl_Widget *w, void *data)
{
  char sz[64];
  real r;

  for (int i = 0; i <= oCore; i++) {
    if (s_inObjOrb[i]) {
      r = atof(s_inObjOrb[i]->value());
      if (r >= -rDegMax && r <= rDegMax)
        rObjOrb[i] = r;
    }
    if (s_inObjAdd[i]) {
      r = atof(s_inObjAdd[i]->value());
      if (r >= -rDegMax && r <= rDegMax)
        rObjAdd[i] = r;
    }
    if (s_inObjInf[i]) {
      r = atof(s_inObjInf[i]->value());
      rObjInf[i] = r;
    }
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgObject)
    s_dlgObject->hide();
}

static void cb_ObjectCancel(Fl_Widget *w, void *data)
{
  if (s_dlgObject)
    s_dlgObject->hide();
}

void FShowDlgObject()
{
  int w = 450, h = 400;
  char sz[64];

  s_dlgObject = new Fl_Window(w, h, "Object Settings");
  s_dlgObject->begin();

  // Header row
  new Fl_Box(10, 10, 100, 20, "Object");
  new Fl_Box(115, 10, 80, 20, "Max Orb");
  new Fl_Box(200, 10, 80, 20, "Orb Add");
  new Fl_Box(285, 10, 80, 20, "Influence");

  // Scrollable area for objects
  Fl_Scroll *scroll = new Fl_Scroll(5, 35, w - 10, h - 85);
  scroll->begin();

  int y = 0;
  for (int i = 0; i <= oCore; i++) {
    new Fl_Box(5, y, 100, 25, szObjName[i]);

    s_inObjOrb[i] = new Fl_Float_Input(110, y, 75, 25);
    sprintf(sz, "%.2f", rObjOrb[i]);
    s_inObjOrb[i]->value(sz);

    s_inObjAdd[i] = new Fl_Float_Input(195, y, 75, 25);
    sprintf(sz, "%.1f", rObjAdd[i]);
    s_inObjAdd[i]->value(sz);

    s_inObjInf[i] = new Fl_Float_Input(280, y, 75, 25);
    sprintf(sz, "%.2f", rObjInf[i]);
    s_inObjInf[i]->value(sz);

    y += 28;
  }

  scroll->end();

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_ObjectOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_ObjectCancel);

  s_dlgObject->end();
  s_dlgObject->set_modal();
  s_dlgObject->show();

  while (s_dlgObject->visible())
    Fl::wait();

  delete s_dlgObject;
  s_dlgObject = NULL;
  for (int i = 0; i <= oCore; i++) {
    s_inObjOrb[i] = NULL;
    s_inObjAdd[i] = NULL;
    s_inObjInf[i] = NULL;
  }
}

/*
******************************************************************************
** More Object Settings Dialog (Cusps, Uranians, Dwarfs, Stars)
******************************************************************************
*/

// Calculate number of objects: oAsc through dwarfHi, plus starLo
#define cObjExt (dwarfHi - oAsc + 2)  // +1 for starLo, +1 for inclusive range

static Fl_Window *s_dlgObject2 = NULL;
static Fl_Float_Input *s_inObj2Orb[cObjExt];
static Fl_Float_Input *s_inObj2Add[cObjExt];
static Fl_Float_Input *s_inObj2Inf[cObjExt];

// Map index to actual object index
static int NObj2Index(int idx)
{
  if (idx < dwarfHi - oAsc + 1)
    return oAsc + idx;
  else
    return starLo;
}

static void cb_Object2OK(Fl_Widget *w, void *data)
{
  for (int idx = 0; idx < cObjExt; idx++) {
    int i = NObj2Index(idx);
    if (s_inObj2Orb[idx])
      rObjOrb[i] = atof(s_inObj2Orb[idx]->value());
    if (s_inObj2Add[idx])
      rObjAdd[i] = atof(s_inObj2Add[idx]->value());
    if (s_inObj2Inf[idx])
      rObjInf[i] = atof(s_inObj2Inf[idx]->value());
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgObject2)
    s_dlgObject2->hide();
}

static void cb_Object2Cancel(Fl_Widget *w, void *data)
{
  if (s_dlgObject2)
    s_dlgObject2->hide();
}

void FShowDlgObject2()
{
  int w = 480, h = 450;
  char sz[64];

  s_dlgObject2 = new Fl_Window(w, h, "More Object Settings");
  s_dlgObject2->begin();

  // Header row
  new Fl_Box(10, 10, 120, 20, "Object");
  new Fl_Box(140, 10, 80, 20, "Max Orb");
  new Fl_Box(230, 10, 80, 20, "Orb Add");
  new Fl_Box(320, 10, 80, 20, "Influence");

  // Scrollable area for objects
  Fl_Scroll *scroll = new Fl_Scroll(5, 35, w - 10, h - 85);
  scroll->begin();

  int y = 0;
  for (int idx = 0; idx < cObjExt; idx++) {
    int i = NObj2Index(idx);

    new Fl_Box(5, y, 120, 25, szObjName[i]);

    s_inObj2Orb[idx] = new Fl_Float_Input(135, y, 75, 25);
    sprintf(sz, "%.2f", rObjOrb[i]);
    s_inObj2Orb[idx]->value(sz);

    s_inObj2Add[idx] = new Fl_Float_Input(220, y, 75, 25);
    sprintf(sz, "%.1f", rObjAdd[i]);
    s_inObj2Add[idx]->value(sz);

    s_inObj2Inf[idx] = new Fl_Float_Input(305, y, 75, 25);
    sprintf(sz, "%.2f", rObjInf[i]);
    s_inObj2Inf[idx]->value(sz);

    y += 28;
  }

  scroll->end();

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_Object2OK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_Object2Cancel);

  s_dlgObject2->end();
  s_dlgObject2->set_modal();
  s_dlgObject2->show();

  while (s_dlgObject2->visible())
    Fl::wait();

  delete s_dlgObject2;
  s_dlgObject2 = NULL;
  for (int idx = 0; idx < cObjExt; idx++) {
    s_inObj2Orb[idx] = NULL;
    s_inObj2Add[idx] = NULL;
    s_inObj2Inf[idx] = NULL;
  }
}

/*
******************************************************************************
** Star Restrictions Dialog
******************************************************************************
*/

static Fl_Window *s_dlgStar = NULL;
static Fl_Check_Button *s_cbStar[cStar+1];

static void cb_StarOK(Fl_Widget *w, void *data)
{
  for (int i = 1; i <= cStar; i++) {
    if (s_cbStar[i])
      ignore[starLo - 1 + i] = !s_cbStar[i]->value();
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgStar)
    s_dlgStar->hide();
}

static void cb_StarCancel(Fl_Widget *w, void *data)
{
  if (s_dlgStar)
    s_dlgStar->hide();
}

static void cb_StarAll(Fl_Widget *w, void *data)
{
  for (int i = 1; i <= cStar; i++)
    if (s_cbStar[i])
      s_cbStar[i]->value(1);
}

static void cb_StarNone(Fl_Widget *w, void *data)
{
  for (int i = 1; i <= cStar; i++)
    if (s_cbStar[i])
      s_cbStar[i]->value(0);
}

void FShowDlgStar()
{
  int w = 500, h = 450;

  s_dlgStar = new Fl_Window(w, h, "Star Restrictions");
  s_dlgStar->begin();

  // Button row
  Fl_Button *btnAll = new Fl_Button(10, 10, 80, 25, "Show All");
  btnAll->callback(cb_StarAll);
  Fl_Button *btnNone = new Fl_Button(100, 10, 80, 25, "Hide All");
  btnNone->callback(cb_StarNone);

  // Scrollable area for stars
  Fl_Scroll *scroll = new Fl_Scroll(5, 45, w - 10, h - 95);
  scroll->begin();

  int cols = 3;
  int colW = (w - 30) / cols;
  int y = 0;
  int x = 0;

  for (int i = 1; i <= cStar; i++) {
    int col = (i - 1) % cols;
    int row = (i - 1) / cols;
    x = 5 + col * colW;
    y = row * 25;

    int objIdx = starLo - 1 + i;
    s_cbStar[i] = new Fl_Check_Button(x, y, colW - 5, 25, szObjName[objIdx]);
    s_cbStar[i]->value(!ignore[objIdx]);
  }

  scroll->end();

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_StarOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_StarCancel);

  s_dlgStar->end();
  s_dlgStar->set_modal();
  s_dlgStar->show();

  while (s_dlgStar->visible())
    Fl::wait();

  delete s_dlgStar;
  s_dlgStar = NULL;
  for (int i = 1; i <= cStar; i++)
    s_cbStar[i] = NULL;
}

/*
******************************************************************************
** Charts #3 Through #6 Info Dialog
******************************************************************************
*/

// Dialog to display info for all 6 chart slots (matching Windows DlgInfoAll)
static Fl_Window *s_dlgInfoAll = NULL;
static Fl_Output *s_outInfo[cRing + 1] = {NULL};
static Fl_Output *s_outName[cRing + 1] = {NULL};
static Fl_Round_Button *s_rbRel[cRing] = {NULL};
static int s_nRelSel = 0;

static void cb_InfoAllOpen(Fl_Widget *w, void *data)
{
  int i = (int)(long)data;
  if (i < 1 || i > cRing) return;

  // Use file chooser to open a chart for this slot
  const char *szFile = fl_file_chooser("Open Chart", "*.as", NULL);
  if (szFile == NULL) return;

  char szCmd[cchSzMax];
  sprintf(szCmd, "-i%d \"%s\"", i, szFile);
  FProcessCommandLine(szCmd);
  fi.fDoCast = fTrue;

  // Update the display
  CI *pci = rgpci[i];
  int n = DayOfWeek(pci->mon, pci->day, pci->yea);
  char sz[cchSzLine];
  sprintf(sz, "%.3s %s %s (Zone %s) %s", szDay[n],
    SzDate(pci->mon, pci->day, pci->yea, 3), SzTim(pci->tim),
    SzZone(pci->zon), SzLocation(pci->lon, pci->lat));
  s_outInfo[i]->value(sz);
  sprintf(sz, "%s%s%s", pci->nam,
    (pci->nam[0] && pci->loc[0]) ? "; " : "", pci->loc);
  s_outName[i]->value(sz);
}

static void cb_InfoAllEdit(Fl_Widget *w, void *data)
{
  int i = (int)(long)data;
  if (i < 1 || i > cRing) return;

  FShowDlgInfo(i);
  fi.fDoCast = fTrue;

  // Update the display
  CI *pci = rgpci[i];
  int n = DayOfWeek(pci->mon, pci->day, pci->yea);
  char sz[cchSzLine];
  sprintf(sz, "%.3s %s %s (Zone %s) %s", szDay[n],
    SzDate(pci->mon, pci->day, pci->yea, 3), SzTim(pci->tim),
    SzZone(pci->zon), SzLocation(pci->lon, pci->lat));
  s_outInfo[i]->value(sz);
  sprintf(sz, "%s%s%s", pci->nam,
    (pci->nam[0] && pci->loc[0]) ? "; " : "", pci->loc);
  s_outName[i]->value(sz);
}

static void cb_InfoAllOK(Fl_Widget *w, void *data)
{
  // Get selected relationship type
  for (int i = 0; i < cRing - 1; i++) {
    if (s_rbRel[i] && s_rbRel[i]->value()) {
      s_nRelSel = i;
      break;
    }
  }
  // Set relationship: 0=None, 1=Comparison, 2=Synastry, 3=Composite, 4=Midpoint
  // Map to rc constants: rcDual(0), rcDual(0), rcSynastry(-1), rcComposite(-2), rcMidpoint(-3)
  static const int rgRc[] = {rcNone, rcDual, rcSynastry, rcComposite, rcMidpoint};
  if (s_nRelSel >= 0 && s_nRelSel <= 4)
    us.nRel = rgRc[s_nRelSel];
  fi.fDoCast = fTrue;

  s_dlgInfoAll->hide();
}

static void cb_InfoAllCancel(Fl_Widget *w, void *data)
{
  s_dlgInfoAll->hide();
}

void FShowDlgInfoAll()
{
  int w = 600, h = 500;

  s_dlgInfoAll = new Fl_Window(w, h, "Charts #3 Through #6");
  s_dlgInfoAll->begin();

  int y = 10;
  char sz[cchSzLine];

  // Display info for all 6 chart slots
  for (int i = 1; i <= cRing; i++) {
    CI *pci = rgpci[i];
    int n = DayOfWeek(pci->mon, pci->day, pci->yea);

    // Chart label
    char szLabel[32];
    sprintf(szLabel, "Chart #%d:", i);
    new Fl_Box(FL_NO_BOX, 10, y, 70, 25, strdup(szLabel));

    // Open button
    Fl_Button *btnOpen = new Fl_Button(80, y, 50, 25, "Open");
    btnOpen->callback(cb_InfoAllOpen, (void*)(long)i);

    // Edit button
    Fl_Button *btnEdit = new Fl_Button(135, y, 50, 25, "Edit");
    btnEdit->callback(cb_InfoAllEdit, (void*)(long)i);

    // Date/time/location output
    sprintf(sz, "%.3s %s %s (Zone %s) %s", szDay[n],
      SzDate(pci->mon, pci->day, pci->yea, 3), SzTim(pci->tim),
      SzZone(pci->zon), SzLocation(pci->lon, pci->lat));
    s_outInfo[i] = new Fl_Output(190, y, w - 200, 25);
    s_outInfo[i]->value(sz);
    y += 28;

    // Name/location output
    sprintf(sz, "%s%s%s", pci->nam,
      (pci->nam[0] && pci->loc[0]) ? "; " : "", pci->loc);
    s_outName[i] = new Fl_Output(190, y, w - 200, 25);
    s_outName[i]->value(sz);
    y += 35;
  }

  // Divider
  new Fl_Box(FL_DOWN_BOX, 10, y, w - 20, 2, NULL);
  y += 15;

  // Relationship type radio buttons
  new Fl_Box(FL_NO_BOX, 10, y, 150, 25, "Relationship Chart:");
  y += 25;

  static const char *szRelType[] = {
    "No Relationship", "Comparison", "Synastry", "Composite",
    "Time Space Midpoint"
  };

  int nRel = us.nRel;
  if (nRel > rcDual)
    nRel = 0;
  else if (nRel < rcHexaWheel)
    nRel = rcDual;
  nRel = rcDual - nRel;

  for (int i = 0; i <= rcDual; i++) {
    s_rbRel[i] = new Fl_Round_Button(30, y, 180, 25, szRelType[i]);
    s_rbRel[i]->type(FL_RADIO_BUTTON);
    if (i == nRel)
      s_rbRel[i]->value(1);
    y += 25;
  }

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_InfoAllOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_InfoAllCancel);

  s_dlgInfoAll->end();
  s_dlgInfoAll->set_modal();
  s_dlgInfoAll->show();

  while (s_dlgInfoAll->visible())
    Fl::wait();

  delete s_dlgInfoAll;
  s_dlgInfoAll = NULL;
  for (int i = 1; i <= cRing; i++) {
    s_outInfo[i] = NULL;
    s_outName[i] = NULL;
  }
  for (int i = 0; i < cRing - 1; i++)
    s_rbRel[i] = NULL;
}

/*
******************************************************************************
** Chart List Dialog
******************************************************************************
*/

static Fl_Window *s_dlgList = NULL;
static Fl_Hold_Browser *s_brList = NULL;
static Fl_Round_Button *s_rbChart[cRing + 1] = {NULL};
static int s_nChartSel = 1;

static void RefreshChartList()
{
  if (!s_brList) return;
  s_brList->clear();

  int nSav = us.fAnsiChar;
  us.fAnsiChar = 2;
  flag fSav = us.fGraphics;
  us.fGraphics = fTrue;

  for (int i = 0; i < is.cci; i++) {
    CI *pci = &is.rgci[i];
    int j = DayOfWeek(pci->mon, pci->day, pci->yea);
    char sz[cchSzLine];
    sprintf(sz, "%.3s %s %s (Zone %s) %s %s%s%s", szDay[j],
      SzDate(pci->mon, pci->day, pci->yea, 3), SzTim(pci->tim),
      SzZone(pci->zon), SzLocation(pci->lon, pci->lat),
      pci->nam, (pci->nam[0] && pci->loc[0]) ? "; " : "", pci->loc);
    s_brList->add(sz, (void*)(long)i);
  }

  us.fAnsiChar = nSav;
  us.fGraphics = fSav;

  // Select current chart
  if (is.iciCur >= 0 && is.iciCur < is.cci)
    s_brList->select(is.iciCur + 1);
}

static void cb_ListSetToChart(Fl_Widget *w, void *data)
{
  int sel = s_brList->value();
  if (sel <= 0) {
    fl_message("No chart selected in list.");
    return;
  }
  int i = (int)(long)s_brList->data(sel);
  if (i < 0 || i >= is.cci) return;

  // Get target chart slot
  for (int j = 1; j <= cRing; j++) {
    if (s_rbChart[j] && s_rbChart[j]->value()) {
      s_nChartSel = j;
      break;
    }
  }

  // Copy chart info to slot
  CI ciT = is.rgci[i];
  is.iciCur = i;
  *rgpci[s_nChartSel] = ciT;
  if (s_nChartSel == 1)
    ciCore = ciT;
  fi.fDoCast = fTrue;
}

static void cb_ListEdit(Fl_Widget *w, void *data)
{
  int sel = s_brList->value();
  if (sel <= 0) {
    fl_message("No chart selected in list.");
    return;
  }
  int i = (int)(long)s_brList->data(sel);
  if (i < 0 || i >= is.cci) return;

  // Edit the chart in the list using negative index
  FShowDlgInfo(-(i + 1));
  RefreshChartList();
}

static void cb_ListDelete(Fl_Widget *w, void *data)
{
  int sel = s_brList->value();
  if (sel <= 0) {
    fl_message("No chart selected in list.");
    return;
  }
  int i = (int)(long)s_brList->data(sel);
  if (i < 0 || i >= is.cci) return;

  // Delete chart from list
  if (i < is.cci - 1)
    memmove(&is.rgci[i], &is.rgci[i + 1], (is.cci - 1 - i) * sizeof(CI));
  is.cci--;
  if (is.iciCur >= is.cci)
    is.iciCur = is.cci - 1;
  RefreshChartList();
}

static void cb_ListCopyFrom(Fl_Widget *w, void *data)
{
  // Get source chart slot
  for (int j = 1; j <= cRing; j++) {
    if (s_rbChart[j] && s_rbChart[j]->value()) {
      s_nChartSel = j;
      break;
    }
  }

  // Add chart to list
  FAppendCIList(rgpci[s_nChartSel]);
  RefreshChartList();
  s_brList->select(is.cci);
}

static void cb_ListDeleteAll(Fl_Widget *w, void *data)
{
  is.cci = 0;
  RefreshChartList();
}

static void cb_ListOK(Fl_Widget *w, void *data)
{
  // Set to selected chart on OK
  cb_ListSetToChart(w, data);
  s_dlgList->hide();
}

static void cb_ListCancel(Fl_Widget *w, void *data)
{
  s_dlgList->hide();
}

void FShowDlgList()
{
  if (is.cci <= 0) {
    fl_message("There is no chart list in memory.\nUse -5l to load a chart list file.");
    return;
  }

  int w = 650, h = 450;

  s_dlgList = new Fl_Window(w, h, "Chart List");
  s_dlgList->begin();

  // Browser for chart list
  s_brList = new Fl_Hold_Browser(10, 10, w - 20, h - 180);

  int y = h - 165;

  // Chart slot radio buttons
  new Fl_Box(FL_NO_BOX, 10, y, 100, 25, "Set to Chart:");
  int x = 110;
  for (int i = 1; i <= cRing; i++) {
    char sz[8];
    sprintf(sz, "#%d", i);
    s_rbChart[i] = new Fl_Round_Button(x, y, 50, 25, strdup(sz));
    s_rbChart[i]->type(FL_RADIO_BUTTON);
    if (i == 1)
      s_rbChart[i]->value(1);
    x += 50;
  }
  y += 30;

  // Action buttons row 1
  Fl_Button *btnSet = new Fl_Button(10, y, 100, 25, "Set to Chart");
  btnSet->callback(cb_ListSetToChart);

  Fl_Button *btnEdit = new Fl_Button(120, y, 100, 25, "Edit Chart");
  btnEdit->callback(cb_ListEdit);

  Fl_Button *btnDelete = new Fl_Button(230, y, 100, 25, "Delete");
  btnDelete->callback(cb_ListDelete);

  Fl_Button *btnCopy = new Fl_Button(340, y, 120, 25, "Copy From Chart");
  btnCopy->callback(cb_ListCopyFrom);

  Fl_Button *btnDelAll = new Fl_Button(470, y, 100, 25, "Delete All");
  btnDelAll->callback(cb_ListDeleteAll);

  y += 35;

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_ListOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_ListCancel);

  s_dlgList->end();
  s_dlgList->set_modal();

  // Populate the list
  RefreshChartList();

  s_dlgList->show();

  while (s_dlgList->visible())
    Fl::wait();

  delete s_dlgList;
  s_dlgList = NULL;
  s_brList = NULL;
  for (int i = 1; i <= cRing; i++)
    s_rbChart[i] = NULL;
}

/*
******************************************************************************
** Moon Restrictions Dialog
******************************************************************************
*/

static Fl_Window *s_dlgMoons = NULL;
static Fl_Check_Button *s_cbMoon[cMoons2 + 1] = {NULL};

static void cb_MoonsAll(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cMoons2; i++)
    s_cbMoon[i]->value(0);  // 0 = shown (not ignored)
}

static void cb_MoonsNone(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cMoons2; i++)
    s_cbMoon[i]->value(1);  // 1 = hidden (ignored)
}

static void cb_MoonsTogglePlanet(Fl_Widget *w, void *data)
{
  int start = (int)(long)data >> 16;
  int end = (int)(long)data & 0xFFFF;
  for (int i = start; i <= end; i++)
    s_cbMoon[i]->value(!s_cbMoon[i]->value());
}

static void cb_MoonsOK(Fl_Widget *w, void *data)
{
  // Apply restrictions
  for (int i = 0; i < cMoons2; i++)
    ignore[moonsLo + i] = s_cbMoon[i]->value();

  // Update us.fMoons flag
  flag fAnyMoon = fFalse;
  for (int i = moonsLo; i <= moonsHi; i++) {
    if (!ignore[i]) {
      fAnyMoon = fTrue;
      break;
    }
  }
  us.fMoons = fAnyMoon;

  // Update us.fCOB flag
  flag fAnyCOB = fFalse;
  for (int i = cobLo; i <= cobHi; i++) {
    if (!ignore[i]) {
      fAnyCOB = fTrue;
      break;
    }
  }
  us.fCOB = fAnyCOB;

  fi.fDoCast = fTrue;
  s_dlgMoons->hide();
}

static void cb_MoonsCancel(Fl_Widget *w, void *data)
{
  s_dlgMoons->hide();
}

void FShowDlgMoons()
{
  int w = 500, h = 480;

  s_dlgMoons = new Fl_Window(w, h, "Moon Restrictions");
  s_dlgMoons->begin();

  // Button row
  Fl_Button *btnAll = new Fl_Button(10, 10, 80, 25, "Show All");
  btnAll->callback(cb_MoonsAll);
  Fl_Button *btnNone = new Fl_Button(100, 10, 80, 25, "Hide All");
  btnNone->callback(cb_MoonsNone);

  // Planet toggle buttons
  int bx = 200;
  Fl_Button *btnMar = new Fl_Button(bx, 10, 50, 25, "Mars");
  btnMar->callback(cb_MoonsTogglePlanet, (void*)(long)((0 << 16) | 1));
  bx += 55;
  Fl_Button *btnJup = new Fl_Button(bx, 10, 50, 25, "Jup");
  btnJup->callback(cb_MoonsTogglePlanet, (void*)(long)((2 << 16) | 5));
  bx += 55;
  Fl_Button *btnSat = new Fl_Button(bx, 10, 50, 25, "Sat");
  btnSat->callback(cb_MoonsTogglePlanet, (void*)(long)((6 << 16) | 13));
  bx += 55;
  Fl_Button *btnUra = new Fl_Button(bx, 10, 50, 25, "Ura");
  btnUra->callback(cb_MoonsTogglePlanet, (void*)(long)((14 << 16) | 18));

  bx = 200;
  Fl_Button *btnNep = new Fl_Button(bx, 40, 50, 25, "Nep");
  btnNep->callback(cb_MoonsTogglePlanet, (void*)(long)((19 << 16) | 21));
  bx += 55;
  Fl_Button *btnPlu = new Fl_Button(bx, 40, 50, 25, "Plu");
  btnPlu->callback(cb_MoonsTogglePlanet, (void*)(long)((22 << 16) | 26));
  bx += 55;
  Fl_Button *btnCOB = new Fl_Button(bx, 40, 50, 25, "COB");
  btnCOB->callback(cb_MoonsTogglePlanet, (void*)(long)((27 << 16) | 31));

  // Scrollable area for moons
  Fl_Scroll *scroll = new Fl_Scroll(5, 75, w - 10, h - 130);
  scroll->begin();

  int cols = 3;
  int colW = (w - 30) / cols;
  int y = 0;
  int x = 0;

  for (int i = 0; i < cMoons2; i++) {
    int col = i % cols;
    int row = i / cols;
    x = 5 + col * colW;
    y = row * 25;

    int objIdx = moonsLo + i;
    s_cbMoon[i] = new Fl_Check_Button(x, y, colW - 5, 25, szObjName[objIdx]);
    s_cbMoon[i]->value(ignore[objIdx]);
  }

  scroll->end();

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_MoonsOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_MoonsCancel);

  s_dlgMoons->end();
  s_dlgMoons->set_modal();
  s_dlgMoons->show();

  while (s_dlgMoons->visible())
    Fl::wait();

  delete s_dlgMoons;
  s_dlgMoons = NULL;
  for (int i = 0; i < cMoons2; i++)
    s_cbMoon[i] = NULL;
}

/*
******************************************************************************
** Moon Object Settings Dialog
******************************************************************************
*/

static Fl_Window *s_dlgMoonObj = NULL;
static Fl_Float_Input *s_inMoonOrb[cMoons2];
static Fl_Float_Input *s_inMoonAdd[cMoons2];
static Fl_Float_Input *s_inMoonInf[cMoons2];
static Fl_Check_Button *s_cbMoonMove = NULL;
static Fl_Check_Button *s_cbMoonChartSep = NULL;
static Fl_Check_Button *s_cbMoonWheel = NULL;

static void cb_MoonObjOK(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cMoons2; i++) {
    int obj = moonsLo + i;
    if (s_inMoonOrb[i])
      rObjOrb[obj] = atof(s_inMoonOrb[i]->value());
    if (s_inMoonAdd[i])
      rObjAdd[obj] = atof(s_inMoonAdd[i]->value());
    if (s_inMoonInf[i])
      rObjInf[obj] = atof(s_inMoonInf[i]->value());
  }

  us.fMoonMove = s_cbMoonMove ? s_cbMoonMove->value() : us.fMoonMove;
  us.fMoonChartSep = s_cbMoonChartSep ? s_cbMoonChartSep->value() : us.fMoonChartSep;
  gs.fMoonWheel = s_cbMoonWheel ? s_cbMoonWheel->value() : gs.fMoonWheel;

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgMoonObj)
    s_dlgMoonObj->hide();
}

static void cb_MoonObjCancel(Fl_Widget *w, void *data)
{
  if (s_dlgMoonObj)
    s_dlgMoonObj->hide();
}

void FShowDlgMoonObj()
{
  int w = 480, h = 450;
  char sz[64];

  s_dlgMoonObj = new Fl_Window(w, h, "Moon Object Settings");
  s_dlgMoonObj->begin();

  // Header row
  new Fl_Box(10, 10, 120, 20, "Moon/COB");
  new Fl_Box(135, 10, 80, 20, "Max Orb");
  new Fl_Box(220, 10, 80, 20, "Orb Add");
  new Fl_Box(305, 10, 80, 20, "Influence");

  // Scrollable area for moons
  Fl_Scroll *scroll = new Fl_Scroll(5, 35, w - 10, h - 135);
  scroll->begin();

  int y = 0;
  for (int i = 0; i < cMoons2; i++) {
    int obj = moonsLo + i;
    new Fl_Box(5, y, 120, 25, szObjName[obj]);

    s_inMoonOrb[i] = new Fl_Float_Input(130, y, 75, 25);
    sprintf(sz, "%.2f", rObjOrb[obj]);
    s_inMoonOrb[i]->value(sz);

    s_inMoonAdd[i] = new Fl_Float_Input(215, y, 75, 25);
    sprintf(sz, "%.1f", rObjAdd[obj]);
    s_inMoonAdd[i]->value(sz);

    s_inMoonInf[i] = new Fl_Float_Input(300, y, 75, 25);
    sprintf(sz, "%.2f", rObjInf[obj]);
    s_inMoonInf[i]->value(sz);

    y += 28;
  }

  scroll->end();

  // Options checkboxes
  int optY = h - 90;
  s_cbMoonMove = new Fl_Check_Button(10, optY, 200, 25, "Moons Move in Time");
  s_cbMoonMove->value(us.fMoonMove);

  s_cbMoonChartSep = new Fl_Check_Button(220, optY, 200, 25, "Separate Moon Chart");
  s_cbMoonChartSep->value(us.fMoonChartSep);

  s_cbMoonWheel = new Fl_Check_Button(10, optY + 25, 200, 25, "Moon Wheel Display");
  s_cbMoonWheel->value(gs.fMoonWheel);

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_MoonObjOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_MoonObjCancel);

  s_dlgMoonObj->end();
  s_dlgMoonObj->set_modal();
  s_dlgMoonObj->show();

  while (s_dlgMoonObj->visible())
    Fl::wait();

  delete s_dlgMoonObj;
  s_dlgMoonObj = NULL;
  for (int i = 0; i < cMoons2; i++) {
    s_inMoonOrb[i] = NULL;
    s_inMoonAdd[i] = NULL;
    s_inMoonInf[i] = NULL;
  }
  s_cbMoonMove = NULL;
  s_cbMoonChartSep = NULL;
  s_cbMoonWheel = NULL;
}

/*
******************************************************************************
** Object Customization Dialog
******************************************************************************
*/

// custLo = uranLo, custHi = cobHi
#define cCustObj (custHi - custLo + 1)

static Fl_Window *s_dlgCustom = NULL;
static Fl_Input *s_inCustName[cCustObj];
static Fl_Input *s_inCustDef[cCustObj];

static void cb_CustomOK(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cCustObj; i++) {
    int obj = custLo + i;
    if (s_inCustName[i]) {
      const char *sz = s_inCustName[i]->value();
      if (sz && *sz && !FEqSz(sz, szObjDisp[obj]))
        FCloneSzCore((char *)sz, (char **)&szObjDisp[obj],
          szObjDisp[obj] == szObjName[obj]);
    }
#ifdef SWISS
    if (s_inCustDef[i]) {
      const char *sz = s_inCustDef[i]->value();
      if (sz && *sz) {
        // Parse Swiss ephemeris definition
        char szBuf[cchSzMax];
        strncpy(szBuf, sz, sizeof(szBuf) - 1);
        szBuf[sizeof(szBuf) - 1] = '\0';

        char *pch = szBuf;
        int typ = (*pch == 'h' ? 0 : (*pch == 'm' ? 3 : (*pch == 'j' ? 4 :
          (FNumCh(*pch) ? 1 : 2))));
        if (typ == 0 || typ >= 3)
          pch++;

        // Find where flags start
        char *pchEnd = pch + strlen(pch);
        char *pchFlags = pchEnd;
        while (pchFlags > pch && pchFlags[-1] >= 'A')
          pchFlags--;
        if (pchFlags > pch && pchFlags[-1] == ' ')
          pchFlags[-1] = '\0';

        int objNum = (typ == 2 ? NParseSz(pch, pmObject) : atoi(pch));
        rgTypSwiss[i] = typ;
        rgObjSwiss[i] = objNum;

        // Parse flags
        int pnt = 0, flg = 0;
        for (char *p = pchFlags; *p; p++) {
          switch (*p) {
          case 'n': pnt = 1; break;
          case 's': pnt = 2; break;
          case 'p': pnt = 3; break;
          case 'a': pnt = 4; break;
          case 'H': flg |= 1; break;
          case 'S': flg |= 2; break;
          case 'B': flg |= 4; break;
          case 'N': flg |= 8; break;
          case 'T': flg |= 16; break;
          case 'V': flg |= 32; break;
          }
        }
        rgPntSwiss[i] = pnt;
        rgFlgSwiss[i] = flg;
      }
    }
#endif
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgCustom)
    s_dlgCustom->hide();
}

static void cb_CustomCancel(Fl_Widget *w, void *data)
{
  if (s_dlgCustom)
    s_dlgCustom->hide();
}

#ifdef SWISS
static void cb_CustomLookup(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cCustObj; i++) {
    if (!s_inCustName[i] || !s_inCustDef[i])
      continue;
    const char *name = s_inCustName[i]->value();
    if (name && *name && !FEqSz(name, szObjUnknown))
      continue;

    const char *def = s_inCustDef[i]->value();
    if (!def || !*def)
      continue;

    char szBuf[cchSzMax], szName[cchSzMax];
    strncpy(szBuf, def, sizeof(szBuf) - 1);
    szBuf[sizeof(szBuf) - 1] = '\0';

    char *pch = szBuf;
    int typ = (*pch == 'h' ? 0 : (*pch == 'm' ? 3 : (*pch == 'j' ? 4 :
      (FNumCh(*pch) ? 1 : 2))));
    if (typ == 0 || typ >= 3)
      pch++;

    // Strip flags
    for (char *p = pch + strlen(pch) - 1; p > pch && *p >= 'A'; p--)
      ;
    char *pEnd = pch + strlen(pch);
    while (pEnd > pch && pEnd[-1] >= 'A')
      pEnd--;
    if (pEnd > pch && pEnd[-1] == ' ')
      *--pEnd = '\0';

    int objNum = (typ == 2 ? NParseSz(pch, pmObject) : atoi(pch));

    switch (typ) {
    case 0:
      SwissGetObjName(szName, -objNum);
      break;
    case 1:
      SwissGetObjName(szName, objNum);
      break;
    case 2:
      sprintf(szName, "%s", objNum >= 0 ? szObjName[objNum] : szObjUnknown);
      break;
    case 3:
      objNum = ObjMoons(objNum);
      sprintf(szName, "%s", FItem(objNum) ? szObjName[objNum] : szObjUnknown);
      break;
    default:
      sprintf(szName, "%s", szObjUnknown);
      break;
    }
    s_inCustName[i]->value(szName);
  }
}
#endif

void FShowDlgCustom()
{
  int w = 520, h = 450;

  s_dlgCustom = new Fl_Window(w, h, "Object Customization");
  s_dlgCustom->begin();

  // Header row
  new Fl_Box(10, 10, 80, 20, "Object");
  new Fl_Box(95, 10, 150, 20, "Display Name");
#ifdef SWISS
  new Fl_Box(255, 10, 150, 20, "Definition");
#endif

  // Scrollable area
  Fl_Scroll *scroll = new Fl_Scroll(5, 35, w - 10, h - 85);
  scroll->begin();

  int y = 0;
  for (int i = 0; i < cCustObj; i++) {
    int obj = custLo + i;
    new Fl_Box(5, y, 80, 25, szObjName[obj]);

    s_inCustName[i] = new Fl_Input(90, y, 150, 25);
    s_inCustName[i]->value(szObjDisp[obj]);

#ifdef SWISS
    s_inCustDef[i] = new Fl_Input(250, y, 150, 25);
    char sz[cchSzMax];
    int typ = rgTypSwiss[i];
    int objNum = rgObjSwiss[i];
    if (typ != 2)
      sprintf(sz, "%s%d",
        typ <= 0 ? "h" : (typ == 1 ? "" : (typ == 3 ? "m" : "j")), objNum);
    else
      sprintf(sz, objNum < cobLo ? "%.3s" : "%.4s", szObjName[objNum]);
    // Add flags
    char *pch = sz + strlen(sz);
    int pnt = rgPntSwiss[i];
    int flg = rgFlgSwiss[i];
    if (pnt > 0 || flg > 0)
      *pch++ = ' ';
    if (pnt > 0)
      *pch++ = (pnt == 1 ? 'n' : (pnt == 2 ? 's' : (pnt == 3 ? 'p' : 'a')));
    if (flg & 1) *pch++ = 'H';
    if (flg & 2) *pch++ = 'S';
    if (flg & 4) *pch++ = 'B';
    if (flg & 8) *pch++ = 'N';
    if (flg & 16) *pch++ = 'T';
    if (flg & 32) *pch++ = 'V';
    *pch = '\0';
    s_inCustDef[i]->value(sz);
#else
    s_inCustDef[i] = NULL;
#endif

    y += 28;
  }

  scroll->end();

#ifdef SWISS
  Fl_Button *btnLookup = new Fl_Button(10, h - 40, 100, 30, "Lookup Names");
  btnLookup->callback(cb_CustomLookup);
#endif

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_CustomOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_CustomCancel);

  s_dlgCustom->end();
  s_dlgCustom->set_modal();
  s_dlgCustom->show();

  while (s_dlgCustom->visible())
    Fl::wait();

  delete s_dlgCustom;
  s_dlgCustom = NULL;
  for (int i = 0; i < cCustObj; i++) {
    s_inCustName[i] = NULL;
    s_inCustDef[i] = NULL;
  }
}

/*
******************************************************************************
** Star Customization Dialog
******************************************************************************
*/

static Fl_Window *s_dlgCustomS = NULL;
static Fl_Input *s_inStarName[cStar];
static Fl_Input *s_inStarDef[cStar];

static void cb_CustomSOK(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cStar; i++) {
    int obj = starLo + i;
    if (s_inStarName[i]) {
      const char *sz = s_inStarName[i]->value();
      if (sz && *sz && !FEqSz(sz, szObjDisp[obj]))
        FCloneSzCore((char *)sz, (char **)&szObjDisp[obj],
          szObjDisp[obj] == szObjName[obj]);
    }
#ifdef SWISS
    if (s_inStarDef[i]) {
      const char *sz = s_inStarDef[i]->value();
      int starIdx = i + 1;
      // Check if different from default
      const char *szDefault = *szStarNameSwiss[starIdx] ?
        szStarNameSwiss[starIdx] : szObjName[obj];
      FCloneSz(FEqSz(sz, szDefault) ? NULL : (char *)sz, &szStarCustom[starIdx]);
    }
#endif
  }

  fi.fDoCast = fTrue;
  if (fi.chart)
    fi.chart->redraw();

  if (s_dlgCustomS)
    s_dlgCustomS->hide();
}

static void cb_CustomSCancel(Fl_Widget *w, void *data)
{
  if (s_dlgCustomS)
    s_dlgCustomS->hide();
}

#ifdef SWISS
static void cb_CustomSLookup(Fl_Widget *w, void *data)
{
  for (int i = 0; i < cStar; i++) {
    if (!s_inStarName[i] || !s_inStarDef[i])
      continue;
    const char *name = s_inStarName[i]->value();
    if (name && *name && !FEqSz(name, szObjUnknown))
      continue;

    const char *def = s_inStarDef[i]->value();
    if (!def || !*def)
      continue;

    char szName[cchSzMax];
    if (!SwissTestStar((char *)def))
      sprintf(szName, "%s", szObjUnknown);
    else
      strncpy(szName, def, sizeof(szName) - 1);
    s_inStarName[i]->value(szName);
  }
}
#endif

void FShowDlgCustomS()
{
  int w = 520, h = 450;

  s_dlgCustomS = new Fl_Window(w, h, "Star Customization");
  s_dlgCustomS->begin();

  // Header row
  new Fl_Box(10, 10, 80, 20, "Star");
  new Fl_Box(95, 10, 150, 20, "Display Name");
#ifdef SWISS
  new Fl_Box(255, 10, 150, 20, "Definition");
#endif

  // Scrollable area
  Fl_Scroll *scroll = new Fl_Scroll(5, 35, w - 10, h - 85);
  scroll->begin();

  int y = 0;
  for (int i = 0; i < cStar; i++) {
    int obj = starLo + i;
    new Fl_Box(5, y, 80, 25, szObjName[obj]);

    s_inStarName[i] = new Fl_Input(90, y, 150, 25);
    s_inStarName[i]->value(szObjDisp[obj]);

#ifdef SWISS
    s_inStarDef[i] = new Fl_Input(250, y, 150, 25);
    int starIdx = i + 1;
    if (FSzSet(szStarCustom[starIdx]))
      s_inStarDef[i]->value(szStarCustom[starIdx]);
    else if (*szStarNameSwiss[starIdx])
      s_inStarDef[i]->value(szStarNameSwiss[starIdx]);
    else
      s_inStarDef[i]->value(szObjName[obj]);
#else
    s_inStarDef[i] = NULL;
#endif

    y += 28;
  }

  scroll->end();

#ifdef SWISS
  Fl_Button *btnLookup = new Fl_Button(10, h - 40, 100, 30, "Lookup Names");
  btnLookup->callback(cb_CustomSLookup);
#endif

  // OK/Cancel buttons
  Fl_Return_Button *btnOK = new Fl_Return_Button(w - 180, h - 40, 80, 30, "OK");
  btnOK->callback(cb_CustomSOK);

  Fl_Button *btnCancel = new Fl_Button(w - 90, h - 40, 80, 30, "Cancel");
  btnCancel->callback(cb_CustomSCancel);

  s_dlgCustomS->end();
  s_dlgCustomS->set_modal();
  s_dlgCustomS->show();

  while (s_dlgCustomS->visible())
    Fl::wait();

  delete s_dlgCustomS;
  s_dlgCustomS = NULL;
  for (int i = 0; i < cStar; i++) {
    s_inStarName[i] = NULL;
    s_inStarDef[i] = NULL;
  }
}

#endif // FLTK
