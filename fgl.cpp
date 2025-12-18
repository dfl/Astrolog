/*
** Astrolog (Version 7.80) File: fgl.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** OpenGL 3D rendering implementation for FLTK GUI
*/

#include "astrolog.h"

#if defined(FLTK) && defined(OPENGL)

#include "fgl.h"
#include "fdriver.h"
#include <cmath>

// Check if the current chart mode is a 3D mode that benefits from OpenGL
bool FIs3DChartMode(int nMode)
{
  return nMode == gGlobe || nMode == gSphere ||
         nMode == gLocal || nMode == gTelescope || nMode == gPolar;
}

/*
******************************************************************************
** Globe3DWidget Implementation
******************************************************************************
*/

Globe3DWidget::Globe3DWidget(int x, int y, int w, int h)
  : Fl_Gl_Window(x, y, w, h),
    chartMode_(gGlobe),
    initialized_(false),
    showGrid_(true),
    showEquator_(true),
    showEcliptic_(true),
    lightingEnabled_(false),
    lastMouseX_(0),
    lastMouseY_(0),
    dragging_(false),
    animAngle_(0.0f)
{
  mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_MULTISAMPLE);
}

Globe3DWidget::~Globe3DWidget()
{
}

void Globe3DWidget::initGL()
{
  if (initialized_)
    return;

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Enable antialiasing
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

  // Set line width
  glLineWidth(1.5f);

  initialized_ = true;
}

void Globe3DWidget::setupProjection()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float aspect = (float)w() / (float)h();
  float fov = 45.0f;  // Field of view in degrees

  // Perspective projection for 3D effect
  gluPerspective(fov, aspect, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
}

void Globe3DWidget::setupLighting()
{
  if (!lightingEnabled_) {
    glDisable(GL_LIGHTING);
    return;
  }

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // Light position (from upper right)
  GLfloat lightPos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  // Light properties
  GLfloat ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
  GLfloat diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

  // Material properties
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Globe3DWidget::draw()
{
  if (!valid()) {
    initGL();
    setupProjection();
  }

  // Clear with background color
  KV kv = rgbbmp[gi.kiOff];
  glClearColor(
    (float)RgbR(kv) / 255.0f,
    (float)RgbG(kv) / 255.0f,
    (float)RgbB(kv) / 255.0f,
    1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  // Position camera
  float distance = 3.5f;
  gluLookAt(0, 0, distance,   // Camera position
            0, 0, 0,          // Look at origin
            0, 1, 0);         // Up vector

  // Apply rotation and tilt from Astrolog settings
  glRotatef((float)gs.rTilt, 1.0f, 0.0f, 0.0f);   // Tilt (around X axis)
  glRotatef((float)gs.rRot, 0.0f, 1.0f, 0.0f);    // Rotation (around Y axis)

  setupLighting();

  // Draw based on chart mode
  switch (chartMode_) {
  case gGlobe:
    drawGlobe();
    break;
  case gSphere:
    drawSphere();
    break;
  case gLocal:
    drawLocal();
    break;
  case gTelescope:
    drawTelescope();
    break;
  case gPolar:
    drawPolar();
    break;
  default:
    drawGlobe();
    break;
  }
}

int Globe3DWidget::handle(int event)
{
  switch (event) {
  case FL_PUSH:
    lastMouseX_ = Fl::event_x();
    lastMouseY_ = Fl::event_y();
    dragging_ = true;
    return 1;

  case FL_DRAG:
    if (dragging_) {
      int dx = Fl::event_x() - lastMouseX_;
      int dy = Fl::event_y() - lastMouseY_;

      // Update rotation based on mouse movement
      gs.rRot += (float)dx * 0.5f;
      gs.rTilt += (float)dy * 0.5f;

      // Clamp rotation values
      while (gs.rRot >= rDegMax) gs.rRot -= rDegMax;
      while (gs.rRot < 0.0) gs.rRot += rDegMax;
      if (gs.rTilt > rDegQuad) gs.rTilt = rDegQuad;
      if (gs.rTilt < -rDegQuad) gs.rTilt = -rDegQuad;

      lastMouseX_ = Fl::event_x();
      lastMouseY_ = Fl::event_y();
      redraw();
    }
    return 1;

  case FL_RELEASE:
    dragging_ = false;
    return 1;

  case FL_MOUSEWHEEL:
    // Zoom with mouse wheel (adjust camera distance)
    // For now, just redraw
    redraw();
    return 1;

  case FL_FOCUS:
  case FL_UNFOCUS:
    return 1;

  default:
    return Fl_Gl_Window::handle(event);
  }
}

void Globe3DWidget::sphereToCartesian(float lon, float lat, float radius,
                                       float *x, float *y, float *z)
{
  // Convert longitude/latitude (in degrees) to 3D cartesian coordinates
  float lonRad = lon * (float)rPi / 180.0f;
  float latRad = lat * (float)rPi / 180.0f;

  *x = radius * cosf(latRad) * sinf(lonRad);
  *y = radius * sinf(latRad);
  *z = radius * cosf(latRad) * cosf(lonRad);
}

void Globe3DWidget::drawSphereGrid(float radius, int latDiv, int lonDiv)
{
  int i, j;
  float x1, y1, z1, x2, y2, z2;

  // Set grid color
  KV kv = rgbbmp[gi.kiGray];
  glColor3f((float)RgbR(kv) / 255.0f,
            (float)RgbG(kv) / 255.0f,
            (float)RgbB(kv) / 255.0f);

  // Draw latitude lines (horizontal circles)
  for (i = -80; i <= 80; i += (180 / latDiv)) {
    glBegin(GL_LINE_STRIP);
    for (j = 0; j <= 360; j += 5) {
      sphereToCartesian((float)j, (float)i, radius, &x1, &y1, &z1);
      glVertex3f(x1, y1, z1);
    }
    glEnd();
  }

  // Draw longitude lines (vertical great circles)
  for (j = 0; j < 360; j += (360 / lonDiv)) {
    glBegin(GL_LINE_STRIP);
    for (i = -90; i <= 90; i += 5) {
      sphereToCartesian((float)j, (float)i, radius, &x1, &y1, &z1);
      glVertex3f(x1, y1, z1);
    }
    glEnd();
  }
}

void Globe3DWidget::drawGreatCircle(float radius, float tilt, float rotation)
{
  float x, y, z;
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < 360; i += 2) {
    float angle = (float)i * (float)rPi / 180.0f;
    // Start with a circle in the XZ plane
    x = radius * cosf(angle);
    z = radius * sinf(angle);
    y = 0;

    // Apply tilt rotation around X axis
    float tiltRad = tilt * (float)rPi / 180.0f;
    float newY = y * cosf(tiltRad) - z * sinf(tiltRad);
    float newZ = y * sinf(tiltRad) + z * cosf(tiltRad);
    y = newY;
    z = newZ;

    // Apply rotation around Y axis
    float rotRad = rotation * (float)rPi / 180.0f;
    float newX = x * cosf(rotRad) + z * sinf(rotRad);
    newZ = -x * sinf(rotRad) + z * cosf(rotRad);
    x = newX;
    z = newZ;

    glVertex3f(x, y, z);
  }
  glEnd();
}

void Globe3DWidget::drawPlanet(int obj, float x, float y, float z, float size)
{
  // Get planet color
  int ki = kObjB[obj];
  if (ki < 0 || ki >= cColor)
    ki = kWhite;
  KV kv = rgbbmp[ki];

  glColor3f((float)RgbR(kv) / 255.0f,
            (float)RgbG(kv) / 255.0f,
            (float)RgbB(kv) / 255.0f);

  // Draw as a point for now
  glPointSize(size);
  glBegin(GL_POINTS);
  glVertex3f(x, y, z);
  glEnd();
}

void Globe3DWidget::drawGlobe()
{
  float radius = 1.0f;
  int i;

  // Draw sphere outline/grid
  if (showGrid_) {
    drawSphereGrid(radius, 9, 12);  // 20-degree divisions
  }

  // Draw equator
  if (showEquator_) {
    KV kv = rgbbmp[kPurpleB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glLineWidth(2.0f);
    drawGreatCircle(radius, 0.0f, 0.0f);
    glLineWidth(1.5f);
  }

  // Draw ecliptic
  if (showEcliptic_) {
    KV kv = rgbbmp[kYellowB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glLineWidth(2.0f);
    drawGreatCircle(radius, 23.44f, 0.0f);  // Earth's axial tilt
    glLineWidth(1.5f);
  }

  // Draw chart location marker
  if (us.fLatitudeCross) {
    float x, y, z;
    // Convert chart location to 3D
    sphereToCartesian((float)(180.0 - Lon), (float)Lat, radius * 1.01f, &x, &y, &z);

    KV kv = rgbbmp[kMagentaB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    glVertex3f(x, y, z);
    glEnd();
  }

  // Draw planets at their zenith positions
  for (i = 0; i <= cObj; i++) {
    if (FIgnore(i))
      continue;

    // Get planet's zenith location on globe
    // Zenith is where the planet appears directly overhead
    float lon = (float)Mod(180.0 - planet[i]);
    float lat = (float)planetalt[i];

    float x, y, z;
    sphereToCartesian(lon, lat, radius * 1.02f, &x, &y, &z);

    drawPlanet(i, x, y, z, 6.0f);
  }
}

void Globe3DWidget::drawSphere()
{
  float radius = 1.0f;
  int i;

  // Draw celestial sphere grid
  if (showGrid_) {
    drawSphereGrid(radius, 6, 12);
  }

  // Draw celestial equator
  if (showEquator_) {
    KV kv = rgbbmp[kPurpleB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glLineWidth(2.0f);
    drawGreatCircle(radius, 0.0f, 0.0f);
    glLineWidth(1.5f);
  }

  // Draw ecliptic
  if (showEcliptic_) {
    KV kv = rgbbmp[kYellowB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glLineWidth(2.0f);

    // Draw zodiac sign divisions on ecliptic
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < 360; i += 2) {
      float lon = (float)i;
      float lat = 0.0f;
      // Apply ecliptic tilt
      float x, y, z;
      sphereToCartesian(lon, lat, radius, &x, &y, &z);
      // Rotate by ecliptic obliquity
      float tilt = 23.44f * (float)rPi / 180.0f;
      float newY = y * cosf(tilt) - z * sinf(tilt);
      float newZ = y * sinf(tilt) + z * cosf(tilt);
      glVertex3f(x, newY, newZ);
    }
    glEnd();
    glLineWidth(1.5f);

    // Draw zodiac sign boundaries
    for (i = 0; i < 12; i++) {
      float signLon = (float)(i * 30);
      float x, y, z;
      sphereToCartesian(signLon, 0.0f, radius * 1.02f, &x, &y, &z);
      // Apply ecliptic tilt
      float tilt = 23.44f * (float)rPi / 180.0f;
      float newY = y * cosf(tilt) - z * sinf(tilt);
      float newZ = y * sinf(tilt) + z * cosf(tilt);

      // Draw short line at sign boundary
      glBegin(GL_LINES);
      glVertex3f(x * 0.95f, newY * 0.95f, newZ * 0.95f);
      glVertex3f(x * 1.05f, newY * 1.05f, newZ * 1.05f);
      glEnd();
    }
  }

  // Draw planets on the celestial sphere
  for (i = 0; i <= cObj; i++) {
    if (FIgnore(i))
      continue;

    // Get planet's ecliptic position
    float lon = (float)planet[i];
    float lat = (float)planetalt[i];

    // Convert to celestial sphere position (with ecliptic tilt)
    float x, y, z;
    sphereToCartesian(lon, lat, radius * 1.02f, &x, &y, &z);

    // Apply ecliptic tilt to align with ecliptic coordinate system
    float tilt = 23.44f * (float)rPi / 180.0f;
    float newY = y * cosf(tilt) - z * sinf(tilt);
    float newZ = y * sinf(tilt) + z * cosf(tilt);

    drawPlanet(i, x, newY, newZ, 8.0f);
  }
}

void Globe3DWidget::drawLocal()
{
  float radius = 1.0f;
  int i;

  // Draw local horizon sphere (sky dome)
  if (showGrid_) {
    // Altitude circles (at 30 degree intervals)
    for (i = 0; i <= 90; i += 30) {
      KV kv = rgbbmp[gi.kiGray];
      glColor3f((float)RgbR(kv) / 255.0f,
                (float)RgbG(kv) / 255.0f,
                (float)RgbB(kv) / 255.0f);

      float altRadius = radius * cosf((float)i * (float)rPi / 180.0f);
      float altY = radius * sinf((float)i * (float)rPi / 180.0f);

      glBegin(GL_LINE_LOOP);
      for (int j = 0; j < 360; j += 5) {
        float angle = (float)j * (float)rPi / 180.0f;
        glVertex3f(altRadius * cosf(angle), altY, altRadius * sinf(angle));
      }
      glEnd();
    }

    // Azimuth lines (at 30 degree intervals)
    for (i = 0; i < 360; i += 30) {
      glBegin(GL_LINE_STRIP);
      for (int j = 0; j <= 90; j += 5) {
        float azimuth = (float)i * (float)rPi / 180.0f;
        float altitude = (float)j * (float)rPi / 180.0f;
        float x = radius * cosf(altitude) * sinf(azimuth);
        float y = radius * sinf(altitude);
        float z = radius * cosf(altitude) * cosf(azimuth);
        glVertex3f(x, y, z);
      }
      glEnd();
    }
  }

  // Draw horizon line
  {
    KV kv = rgbbmp[kGreenB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < 360; i += 2) {
      float angle = (float)i * (float)rPi / 180.0f;
      glVertex3f(radius * cosf(angle), 0.0f, radius * sinf(angle));
    }
    glEnd();
    glLineWidth(1.5f);
  }

  // Draw cardinal directions
  {
    KV kv = rgbbmp[kRedB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    // N, S, E, W markers at horizon
    const char *dirs[] = { "N", "E", "S", "W" };
    float angles[] = { 0.0f, 90.0f, 180.0f, 270.0f };
    for (i = 0; i < 4; i++) {
      float angle = angles[i] * (float)rPi / 180.0f;
      float x = radius * 1.05f * sinf(angle);
      float z = radius * 1.05f * cosf(angle);
      glPointSize(8.0f);
      glBegin(GL_POINTS);
      glVertex3f(x, 0.0f, z);
      glEnd();
    }
  }

  // Draw zenith point
  {
    KV kv = rgbbmp[kCyanB];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glVertex3f(0.0f, radius, 0.0f);  // Zenith
    glEnd();
  }

  // Draw planets in local horizon coordinates
  for (i = 0; i <= cObj; i++) {
    if (FIgnore(i))
      continue;

    // Get planet's altitude/azimuth
    // Note: This uses the calculated values from Astrolog
    // which should be in local horizon coordinates
    float alt = (float)planetalt[i];
    float azi = (float)planet[i];  // Simplified - would need proper conversion

    // Only draw planets above horizon
    if (alt < 0)
      continue;

    float altRad = alt * (float)rPi / 180.0f;
    float aziRad = azi * (float)rPi / 180.0f;

    float x = radius * cosf(altRad) * sinf(aziRad);
    float y = radius * sinf(altRad);
    float z = radius * cosf(altRad) * cosf(aziRad);

    drawPlanet(i, x, y, z, 8.0f);
  }
}

void Globe3DWidget::drawTelescope()
{
  // Similar to local but with narrower field of view
  // For now, just reuse local drawing
  drawLocal();
}

void Globe3DWidget::drawPolar()
{
  // Draw polar projection of globe
  // This is a 2D projection, so we'll draw it as a flat circle
  float radius = 1.0f;
  int i;

  // Draw concentric circles for latitude
  if (showGrid_) {
    KV kv = rgbbmp[gi.kiGray];
    glColor3f((float)RgbR(kv) / 255.0f,
              (float)RgbG(kv) / 255.0f,
              (float)RgbB(kv) / 255.0f);

    for (i = 15; i <= 90; i += 15) {
      float r = radius * (float)i / 90.0f;
      glBegin(GL_LINE_LOOP);
      for (int j = 0; j < 360; j += 5) {
        float angle = (float)j * (float)rPi / 180.0f;
        glVertex3f(r * cosf(angle), 0.0f, r * sinf(angle));
      }
      glEnd();
    }

    // Radial lines for longitude
    for (i = 0; i < 360; i += 30) {
      float angle = (float)i * (float)rPi / 180.0f;
      glBegin(GL_LINES);
      glVertex3f(0.0f, 0.0f, 0.0f);
      glVertex3f(radius * cosf(angle), 0.0f, radius * sinf(angle));
      glEnd();
    }
  }

  // Draw planets in polar projection
  for (i = 0; i <= cObj; i++) {
    if (FIgnore(i))
      continue;

    float lon = (float)Mod(180.0 - planet[i]);
    float lat = (float)planetalt[i];

    // Project to polar coordinates
    float r = radius * (90.0f - (float)RAbs(lat)) / 90.0f;
    float angle = lon * (float)rPi / 180.0f;

    float x = r * cosf(angle);
    float z = r * sinf(angle);

    drawPlanet(i, x, 0.0f, z, 8.0f);
  }
}

void Globe3DWidget::drawConstellationLines()
{
  // TODO: Draw constellation lines using Astrolog's constellation data
  // This would iterate through rgszCnstl and constellation boundaries
}

void Globe3DWidget::drawWorldMap()
{
  // TODO: Draw world map outline using Astrolog's map data
  // This would use the map data from xdata.cpp
}

void InitOpenGL3D(void)
{
  // Called when OpenGL support is initialized
  // Could load textures, compile shaders, etc.
}

#endif // FLTK && OPENGL
