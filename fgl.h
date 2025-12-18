/*
** Astrolog (Version 7.80) File: fgl.h
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** OpenGL 3D rendering widget for FLTK GUI
*/

#ifndef _FGL_H
#define _FGL_H

#if defined(FLTK) && defined(OPENGL)

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

// Silence OpenGL deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Forward declaration
class AstrologWindow;

/*
** Globe3DWidget - OpenGL widget for 3D chart rendering
**
** This widget renders globe, sphere, local, and telescope charts
** using OpenGL for hardware-accelerated 3D graphics with:
** - Antialiased lines
** - Smooth rotation/tilting via mouse drag
** - Optional lighting for depth perception
** - Optional world map texture
*/

class Globe3DWidget : public Fl_Gl_Window {
public:
  Globe3DWidget(int x, int y, int w, int h);
  virtual ~Globe3DWidget();

  void draw() FL_OVERRIDE;
  int handle(int event) FL_OVERRIDE;

  // Chart rendering modes
  void setChartMode(int mode) { chartMode_ = mode; invalidate(); }
  int chartMode() const { return chartMode_; }

  // Rendering options
  void setShowGrid(bool show) { showGrid_ = show; invalidate(); }
  void setShowEquator(bool show) { showEquator_ = show; invalidate(); }
  void setShowEcliptic(bool show) { showEcliptic_ = show; invalidate(); }
  void setLightingEnabled(bool enabled) { lightingEnabled_ = enabled; invalidate(); }

protected:
  // OpenGL setup
  void initGL();
  void setupProjection();
  void setupLighting();

  // Rendering functions
  void drawGlobe();       // World globe (gGlobe)
  void drawSphere();      // Celestial sphere (gSphere)
  void drawLocal();       // Local horizon (gLocal)
  void drawTelescope();   // Telescope view (gTelescope)
  void drawPolar();       // Polar projection (gPolar)

  // Helper functions
  void drawSphereGrid(float radius, int latDiv, int lonDiv);
  void drawGreatCircle(float radius, float tilt, float rotation);
  void drawPlanet(int obj, float x, float y, float z, float size);
  void drawConstellationLines();
  void drawWorldMap();

  // Coordinate conversion (reusing Astrolog math)
  void sphereToCartesian(float lon, float lat, float radius,
                         float *x, float *y, float *z);

private:
  int chartMode_;         // Current chart mode (gGlobe, gSphere, etc.)
  bool initialized_;      // OpenGL context initialized
  bool showGrid_;         // Show coordinate grid
  bool showEquator_;      // Show equator line
  bool showEcliptic_;     // Show ecliptic line
  bool lightingEnabled_;  // Use OpenGL lighting

  // Mouse interaction state
  int lastMouseX_;
  int lastMouseY_;
  bool dragging_;

  // Animation
  float animAngle_;
};

// Function declarations
extern void InitOpenGL3D(void);
extern bool FIs3DChartMode(int nMode);

#endif // FLTK && OPENGL
#endif // _FGL_H
