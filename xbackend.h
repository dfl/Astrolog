/*
** Astrolog (Version 7.80) File: xbackend.h
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** Platform Abstraction Layer for Graphics Backends
**
** This module provides a clean abstraction for screen-based graphics rendering,
** eliminating duplication across X11, Windows, and FLTK backends.
*/

#ifndef _XBACKEND_H
#define _XBACKEND_H

#ifdef GRAPH

/*
** Graphics Backend Interface
**
** This structure contains function pointers for all screen drawing operations.
** Each platform (X11, WIN, WCLI, FLTK) provides its own implementation.
** File output (PS, META, WIRE, BMP) is handled separately in the drawing
** functions since those write to files rather than screen buffers.
*/

typedef struct _GraphicsBackend {
  // Backend identification
  const char *szName;

  // Core drawing operations (using Put* naming to avoid macro conflicts)
  void (*PutColor)(int ki);           // Set current drawing color
  void (*PutPixel)(int x, int y);     // Draw single pixel
  void (*PutPixelThick)(int x, int y); // Draw pixel with thickness
  void (*PutLine)(int x1, int y1, int x2, int y2);  // Draw line
  void (*PutLineThick)(int x1, int y1, int x2, int y2);  // Draw thick line
  void (*PutRect)(int x1, int y1, int w, int h);    // Draw filled rectangle
  void (*PutArc)(int x, int y, int w, int h, double deg1, double deg2);  // Draw arc
  void (*PutEllipse)(int x, int y, int w, int h);   // Draw filled ellipse

  // Screen management
  void (*ClearScreen)(int ki);        // Clear screen with color
  void (*Flush)(void);                // Flush pending drawing operations

  // Backend-specific data (opaque pointer)
  void *pData;
} GB;

// Global pointer to current graphics backend
extern GB *gpBackend;

// Backend initialization functions
extern void InitBackendX11(void);
extern void InitBackendWin(void);
extern void InitBackendFltk(void);

// Convenience macros for calling backend functions
// These check if backend exists and call the appropriate function

#define GBSetColor(ki) \
  do { if (gpBackend && gpBackend->PutColor) gpBackend->PutColor(ki); } while(0)

#define GBDrawPixel(x, y) \
  do { if (gpBackend && gpBackend->PutPixel) gpBackend->PutPixel(x, y); } while(0)

#define GBDrawPixelThick(x, y) \
  do { if (gpBackend && gpBackend->PutPixelThick) gpBackend->PutPixelThick(x, y); } while(0)

#define GBDrawLine(x1, y1, x2, y2) \
  do { if (gpBackend && gpBackend->PutLine) gpBackend->PutLine(x1, y1, x2, y2); } while(0)

#define GBDrawLineThick(x1, y1, x2, y2) \
  do { if (gpBackend && gpBackend->PutLineThick) gpBackend->PutLineThick(x1, y1, x2, y2); } while(0)

#define GBDrawRect(x1, y1, w, h) \
  do { if (gpBackend && gpBackend->PutRect) gpBackend->PutRect(x1, y1, w, h); } while(0)

#define GBDrawArc(x, y, w, h, d1, d2) \
  do { if (gpBackend && gpBackend->PutArc) gpBackend->PutArc(x, y, w, h, (double)(d1), (double)(d2)); } while(0)

#define GBDrawEllipse(x, y, w, h) \
  do { if (gpBackend && gpBackend->PutEllipse) gpBackend->PutEllipse(x, y, w, h); } while(0)

#define GBClearScreen(ki) \
  do { if (gpBackend && gpBackend->ClearScreen) gpBackend->ClearScreen(ki); } while(0)

#define GBFlush() \
  do { if (gpBackend && gpBackend->Flush) gpBackend->Flush(); } while(0)

// Check if a screen backend is active (not writing to file)
#define FBackendActive() (gpBackend != NULL)

#endif // GRAPH
#endif // _XBACKEND_H
