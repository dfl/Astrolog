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
  ReadInfoFields(s_ciEdit);

  // Validate inputs
  if (!FValidMon(s_ciEdit.mon)) {
    fl_alert("Invalid month value.");
    return;
  }
  if (!FValidYea(s_ciEdit.yea)) {
    fl_alert("Invalid year value.");
    return;
  }
  if (!FValidDay(s_ciEdit.day, s_ciEdit.mon, s_ciEdit.yea)) {
    fl_alert("Invalid day value.");
    return;
  }
  if (!FValidTim(s_ciEdit.tim)) {
    fl_alert("Invalid time value.");
    return;
  }
  if (!FValidZon(s_ciEdit.zon)) {
    fl_alert("Invalid time zone value.");
    return;
  }
  if (!FValidLon(s_ciEdit.lon)) {
    fl_alert("Invalid longitude value.");
    return;
  }
  if (!FValidLat(s_ciEdit.lat)) {
    fl_alert("Invalid latitude value.");
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
  int w = 400, h = 340;
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

  // Date row
  new Fl_Box(10, y, 50, 25, "Month:");
  s_chMon = new Fl_Choice(60, y, 70, 25);
  for (int i = 1; i <= 12; i++)
    s_chMon->add(szMonth[i]);

  new Fl_Box(140, y, 30, 25, "Day:");
  s_inDay = new Fl_Int_Input(170, y, 40, 25);

  new Fl_Box(220, y, 35, 25, "Year:");
  s_inYea = new Fl_Int_Input(255, y, 60, 25);

  new Fl_Box(325, y, 35, 25, "Time:");
  s_inTim = new Fl_Input(360, y, w - 370, 25);
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
  int w = 350, h = 220;

  s_dlgCalc = new Fl_Window(w, h, "Calculation Settings");
  s_dlgCalc->begin();

  int y = 10;

  // House system
  new Fl_Box(10, y, 100, 25, "House System:");
  s_chHouse = new Fl_Choice(120, y, 200, 25);
  for (int i = 0; i < cSystem; i++)
    s_chHouse->add(szSystem[i]);
  s_chHouse->value(us.nHouseSystem);
  y += 40;

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

#endif // FLTK
