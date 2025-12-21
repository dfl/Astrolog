cmake_minimum_required(VERSION 3.16)
project(astrolog VERSION 7.80 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_FLTK "Build with FLTK GUI" ON)
option(USE_X11 "Build with X11 GUI (Unix only)" OFF)
option(USE_WIN "Build with Win32 GUI (Windows only)" OFF)
option(USE_CAIRO "Build with Cairo for vector rendering (SVG/PDF export)" ON)
option(USE_OPENGL "Build with OpenGL for 3D chart rendering" OFF)

# Make GUI options mutually exclusive
if(USE_FLTK AND (USE_X11 OR USE_WIN))
  message(FATAL_ERROR "Cannot enable FLTK with X11 or WIN. Choose only one GUI backend.")
endif()
if(USE_X11 AND USE_WIN)
  message(FATAL_ERROR "Cannot enable both X11 and WIN backends.")
endif()

# Source files
set(CORE_SOURCES
  astrolog.cpp
  atlas.cpp
  calc.cpp
  charts0.cpp
  charts1.cpp
  charts2.cpp
  charts3.cpp
  data.cpp
  express.cpp
  general.cpp
  intrpret.cpp
  io.cpp
  matrix.cpp
  placalc.cpp
  placalc2.cpp
)

# Swiss Ephemeris sources
set(SWISS_SOURCES
  swecl.cpp
  swedate.cpp
  swehouse.cpp
  swejpl.cpp
  swemmoon.cpp
  swemplan.cpp
  sweph.cpp
  swephlib.cpp
)

# Graphics sources
set(GRAPHICS_SOURCES
  xbackend.cpp
  xcharts0.cpp
  xcharts1.cpp
  xcharts2.cpp
  xdata.cpp
  xdevice.cpp
  xdialog.cpp
  xevent.cpp
  xgeneral.cpp
  xscreen.cpp
)

# Combine all sources
set(SOURCES
  ${CORE_SOURCES}
  ${SWISS_SOURCES}
  ${GRAPHICS_SOURCES}
)

# Compile definitions
set(COMPILE_DEFS
  GRAPH
  SWISS
  PLACALC
  MATRIX
  PS
  META
  WIRE
  ATLAS
  INTERPRET
  ARABIC
  CONSTEL
  BIORHYTHM
  EXPRESS
)

# Platform-specific settings
if(WIN32)
  list(APPEND COMPILE_DEFS PC)
endif()

# FLTK Backend
if(USE_FLTK)
  find_package(FLTK QUIET)
  if(NOT FLTK_FOUND)
    # Try pkg-config as fallback
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
      pkg_check_modules(FLTK fltk)
    endif()
  endif()

  if(NOT FLTK_FOUND AND NOT FLTK_LIBRARIES)
    message(FATAL_ERROR "FLTK not found. Install FLTK development libraries:\n"
      "  macOS: brew install fltk\n"
      "  Ubuntu/Debian: sudo apt install libfltk1.3-dev\n"
      "  Fedora: sudo dnf install fltk-devel\n"
      "  Windows: vcpkg install fltk")
  endif()

  list(APPEND SOURCES fdriver.cpp fdialog.cpp)
  list(APPEND COMPILE_DEFS FLTK)
  message(STATUS "Building with FLTK GUI backend")
endif()

# X11 Backend
if(USE_X11)
  find_package(X11 REQUIRED)
  list(APPEND COMPILE_DEFS X11)
  message(STATUS "Building with X11 GUI backend")
endif()

# Win32 Backend
if(USE_WIN)
  list(APPEND SOURCES wdriver.cpp wdialog.cpp)
  list(APPEND COMPILE_DEFS WIN)
  message(STATUS "Building with Win32 GUI backend")
endif()

# Cairo Backend (optional, for SVG/PDF rendering)
if(USE_CAIRO)
  find_package(PkgConfig)
  if(PkgConfig_FOUND)
    pkg_check_modules(CAIRO cairo)
    if(CAIRO_FOUND)
      list(APPEND COMPILE_DEFS CAIRO)
      message(STATUS "Building with Cairo vector rendering support")
    else()
      message(STATUS "Cairo not found - SVG/PDF export will be disabled")
      message(STATUS "  Install Cairo: brew install cairo (macOS) or apt install libcairo2-dev (Linux)")
    endif()
  else()
    message(STATUS "pkg-config not found - Cairo detection skipped")
  endif()
endif()

# OpenGL Backend (optional, for 3D chart rendering)
if(USE_OPENGL AND USE_FLTK)
  find_package(OpenGL)
  if(OpenGL_FOUND)
    list(APPEND SOURCES fgl.cpp)
    list(APPEND COMPILE_DEFS OPENGL)
    message(STATUS "Building with OpenGL 3D rendering support")
  else()
    message(STATUS "OpenGL not found - 3D chart rendering will be disabled")
  endif()
endif()

# Create executable
add_executable(astrolog ${SOURCES})

# macOS app bundle
if(APPLE AND USE_FLTK)
  set(MACOSX_BUNDLE_ICON_FILE Astrolog.icns)
  set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/resources/Astrolog.icns")
  set_source_files_properties(${APP_ICON_MACOS} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )

  # Bundle astrology fonts
  set(ASTROLOGY_FONTS
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/astfont/Astro.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/astfont/Astronomicon.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/astfont/EnigmaAstrology.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/astfont/HamburgSymbols.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/astfont/HanksNakshatra.ttf"
  )
  set_source_files_properties(${ASTROLOGY_FONTS} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources/Fonts"
  )

  # Bundle Swiss Ephemeris data files
  file(GLOB EPHE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.se1")
  set(EPHE_DATA_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/sefstars.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/seorbel.txt"
  )
  set_source_files_properties(${EPHE_FILES} ${EPHE_DATA_FILES} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources/ephe"
  )

  target_sources(astrolog PRIVATE ${APP_ICON_MACOS} ${ASTROLOGY_FONTS} ${EPHE_FILES} ${EPHE_DATA_FILES})
  set_target_properties(astrolog PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Astrolog"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.astrolog.Astrolog"
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2024 Astrolog"
    MACOSX_BUNDLE_ICON_FILE "Astrolog.icns"
    OUTPUT_NAME "Astrolog"
  )
endif()

# Apply compile definitions
target_compile_definitions(astrolog PRIVATE ${COMPILE_DEFS})

# Include directories
target_include_directories(astrolog PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries based on backend
if(USE_FLTK)
  if(FLTK_FOUND)
    target_include_directories(astrolog PRIVATE ${FLTK_INCLUDE_DIRS})
    target_link_libraries(astrolog ${FLTK_LIBRARIES})
  else()
    # Using pkg-config results
    target_include_directories(astrolog PRIVATE ${FLTK_INCLUDE_DIRS})
    target_link_libraries(astrolog ${FLTK_LDFLAGS})
  endif()
  if(APPLE)
    # CoreText for loading bundled fonts
    target_link_libraries(astrolog "-framework CoreText" "-framework CoreFoundation")
  endif()
endif()

if(USE_X11)
  target_include_directories(astrolog PRIVATE ${X11_INCLUDE_DIR})
  target_link_libraries(astrolog ${X11_LIBRARIES})
endif()

if(USE_CAIRO AND CAIRO_FOUND)
  target_include_directories(astrolog PRIVATE ${CAIRO_INCLUDE_DIRS})
  target_link_libraries(astrolog ${CAIRO_LIBRARIES})
endif()

if(USE_OPENGL AND USE_FLTK AND OpenGL_FOUND)
  target_link_libraries(astrolog OpenGL::GL)
  if(APPLE)
    # macOS needs the OpenGL framework
    target_link_libraries(astrolog "-framework OpenGL")
  endif()
endif()

if(USE_WIN)
  target_link_libraries(astrolog
    comdlg32
    gdi32
    user32
    shell32
    ole32
  )
endif()

# Math library (Unix)
if(UNIX)
  target_link_libraries(astrolog m)
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(astrolog PRIVATE
    -Wall
  )
endif()

# Installation
if(APPLE AND USE_FLTK)
  install(TARGETS astrolog BUNDLE DESTINATION .)
else()
  install(TARGETS astrolog RUNTIME DESTINATION bin)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Astrolog ${PROJECT_VERSION} Configuration Summary")
message(STATUS "====================================")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "FLTK Backend: ${USE_FLTK}")
message(STATUS "X11 Backend: ${USE_X11}")
message(STATUS "Win32 Backend: ${USE_WIN}")
message(STATUS "Cairo Support: ${CAIRO_FOUND}")
message(STATUS "OpenGL Support: ${OpenGL_FOUND}")
message(STATUS "")
