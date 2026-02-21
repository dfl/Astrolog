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
option(FLTK_STATIC "Link FLTK statically (recommended for distribution)" ON)
option(BUNDLE_LIBS "Bundle dynamic libraries into app (macOS only)" ON)

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
  if(FLTK_STATIC AND APPLE)
    # Static FLTK linking for macOS distribution
    # Find FLTK installation path
    find_program(FLTK_CONFIG fltk-config)
    if(FLTK_CONFIG)
      execute_process(COMMAND ${FLTK_CONFIG} --prefix
        OUTPUT_VARIABLE FLTK_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      set(FLTK_LIB_DIR "${FLTK_PREFIX}/lib")
      set(FLTK_INCLUDE_DIR "${FLTK_PREFIX}/include")
    else()
      # Fallback to common Homebrew paths
      if(EXISTS "/opt/homebrew/opt/fltk")
        set(FLTK_PREFIX "/opt/homebrew/opt/fltk")
      elseif(EXISTS "/usr/local/opt/fltk")
        set(FLTK_PREFIX "/usr/local/opt/fltk")
      else()
        message(FATAL_ERROR "FLTK not found. Install with: brew install fltk")
      endif()
      set(FLTK_LIB_DIR "${FLTK_PREFIX}/lib")
      set(FLTK_INCLUDE_DIR "${FLTK_PREFIX}/include")
    endif()

    # Find static libraries
    find_library(FLTK_STATIC_LIB libfltk.a PATHS ${FLTK_LIB_DIR} NO_DEFAULT_PATH)
    find_library(FLTK_IMAGES_STATIC_LIB libfltk_images.a PATHS ${FLTK_LIB_DIR} NO_DEFAULT_PATH)
    find_library(FLTK_FORMS_STATIC_LIB libfltk_forms.a PATHS ${FLTK_LIB_DIR} NO_DEFAULT_PATH)
    find_library(FLTK_GL_STATIC_LIB libfltk_gl.a PATHS ${FLTK_LIB_DIR} NO_DEFAULT_PATH)

    if(NOT FLTK_STATIC_LIB)
      message(FATAL_ERROR "Static FLTK library not found at ${FLTK_LIB_DIR}/libfltk.a")
    endif()

    set(FLTK_STATIC_LIBS
      ${FLTK_IMAGES_STATIC_LIB}
      ${FLTK_FORMS_STATIC_LIB}
      ${FLTK_GL_STATIC_LIB}
      ${FLTK_STATIC_LIB}
    )
    set(FLTK_INCLUDE_DIRS ${FLTK_INCLUDE_DIR})
    set(FLTK_FOUND TRUE)
    message(STATUS "Using static FLTK from ${FLTK_PREFIX}")
  else()
    # Dynamic FLTK linking
    find_package(FLTK QUIET)
    if(NOT FLTK_FOUND)
      # Try pkg-config as fallback
      find_package(PkgConfig)
      if(PkgConfig_FOUND)
        pkg_check_modules(FLTK fltk)
      endif()
    endif()
  endif()

  if(NOT FLTK_FOUND AND NOT FLTK_LIBRARIES AND NOT FLTK_STATIC_LIBS)
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

  # Bundle Astrolog data files (exoplanet catalog, world map, etc.)
  set(ASTROLOG_DATA_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/astexo.csv"
    "${CMAKE_CURRENT_SOURCE_DIR}/earth.bmp"
  )
  set_source_files_properties(${ASTROLOG_DATA_FILES} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )

  # Bundle documentation files (for Help / More Documentation menu)
  set(ASTROLOG_DOC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/astrolog.htm"
    "${CMAKE_CURRENT_SOURCE_DIR}/changes.htm"
    "${CMAKE_CURRENT_SOURCE_DIR}/license.htm"
    "${CMAKE_CURRENT_SOURCE_DIR}/astrolog.url"
    "${CMAKE_CURRENT_SOURCE_DIR}/astrlog2.url"
  )
  set_source_files_properties(${ASTROLOG_DOC_FILES} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )

  # Bundle Astrolog config files in Resources (not MacOS, which breaks codesigning)
  set(ASTROLOG_CONFIG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/astrolog.as"
    "${CMAKE_CURRENT_SOURCE_DIR}/atlas.as"
    "${CMAKE_CURRENT_SOURCE_DIR}/timezone.as"
  )
  set_source_files_properties(${ASTROLOG_CONFIG_FILES} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )

  target_sources(astrolog PRIVATE ${APP_ICON_MACOS} ${ASTROLOGY_FONTS} ${EPHE_FILES} ${EPHE_DATA_FILES} ${ASTROLOG_DATA_FILES} ${ASTROLOG_DOC_FILES} ${ASTROLOG_CONFIG_FILES})
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
  target_include_directories(astrolog PRIVATE ${FLTK_INCLUDE_DIRS})

  if(FLTK_STATIC AND APPLE AND FLTK_STATIC_LIBS)
    # Static FLTK linking - must add all required frameworks
    target_link_libraries(astrolog ${FLTK_STATIC_LIBS})
    target_link_libraries(astrolog
      "-framework Cocoa"
      "-framework Carbon"
      "-framework ApplicationServices"
      "-framework CoreText"
      "-framework CoreFoundation"
      "-framework CoreGraphics"
      "-framework IOKit"
    )
    # FLTK images library needs these
    find_library(JPEG_LIB jpeg PATHS /opt/homebrew/lib /usr/local/lib)
    find_library(PNG_LIB png PATHS /opt/homebrew/lib /usr/local/lib)
    if(JPEG_LIB)
      target_link_libraries(astrolog ${JPEG_LIB})
    endif()
    if(PNG_LIB)
      target_link_libraries(astrolog ${PNG_LIB})
    endif()
    target_link_libraries(astrolog z)
  elseif(FLTK_FOUND)
    target_link_libraries(astrolog ${FLTK_LIBRARIES})
    if(APPLE)
      target_link_libraries(astrolog "-framework CoreText" "-framework CoreFoundation")
    endif()
  else()
    # Using pkg-config results
    target_link_libraries(astrolog ${FLTK_LDFLAGS})
    if(APPLE)
      target_link_libraries(astrolog "-framework CoreText" "-framework CoreFoundation")
    endif()
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

# Bundle dynamic libraries for distribution (macOS)
if(APPLE AND USE_FLTK AND BUNDLE_LIBS AND USE_CAIRO AND CAIRO_FOUND)
  # Find the Cairo library path for bundling
  find_library(CAIRO_LIBRARY_PATH cairo PATHS ${CAIRO_LIBRARY_DIRS})

  if(CAIRO_LIBRARY_PATH)
    # Create install script that bundles libraries
    # BundleUtilities must be included at install time, not configure time
    install(CODE "
      include(BundleUtilities)
      set(BU_CHMOD_BUNDLE_ITEMS ON)

      # Get absolute path to bundle
      get_filename_component(bundle_path \"\${CMAKE_INSTALL_PREFIX}/Astrolog.app\" ABSOLUTE)

      # Fix up the bundle - copies dependencies and fixes paths
      fixup_bundle(\"\${bundle_path}\" \"\" \"\")

      message(STATUS \"Bundle libraries fixed up successfully\")

      # Re-sign the app after modifying dylibs (required for macOS security)
      # This uses ad-hoc signing; for distribution, use a proper Developer ID
      message(STATUS \"Re-signing the application bundle...\")
      execute_process(
        COMMAND codesign --force --deep --sign - \"\${bundle_path}\"
        RESULT_VARIABLE codesign_result
        OUTPUT_VARIABLE codesign_output
        ERROR_VARIABLE codesign_error
      )
      if(codesign_result EQUAL 0)
        message(STATUS \"Application signed successfully (ad-hoc)\")
      else()
        message(WARNING \"Code signing failed: \${codesign_error}\")
      endif()
    " COMPONENT Runtime)

    message(STATUS "Cairo will be bundled with the application")
  endif()
endif()

# Object library for tests (all sources except astrolog.cpp which has main())
set(LIB_SOURCES ${SOURCES})
list(REMOVE_ITEM LIB_SOURCES astrolog.cpp)
add_library(astrolog_lib OBJECT ${LIB_SOURCES})
target_compile_definitions(astrolog_lib PRIVATE ${COMPILE_DEFS})
target_include_directories(astrolog_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(USE_FLTK)
  target_include_directories(astrolog_lib PRIVATE ${FLTK_INCLUDE_DIRS})
endif()
if(USE_CAIRO AND CAIRO_FOUND)
  target_include_directories(astrolog_lib PRIVATE ${CAIRO_INCLUDE_DIRS})
endif()
if(USE_X11)
  target_include_directories(astrolog_lib PRIVATE ${X11_INCLUDE_DIR})
endif()

# Unit tests
enable_testing()

# Helper function to configure a test executable with the same compile
# definitions, include paths, and library linkage as the main target.
function(add_astrolog_test name source)
  add_executable(${name} ${source} tests/test_stubs.cpp $<TARGET_OBJECTS:astrolog_lib>)
  target_compile_definitions(${name} PRIVATE ${COMPILE_DEFS})
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  if(USE_FLTK)
    target_include_directories(${name} PRIVATE ${FLTK_INCLUDE_DIRS})
    if(FLTK_STATIC AND APPLE AND FLTK_STATIC_LIBS)
      target_link_libraries(${name} ${FLTK_STATIC_LIBS}
        "-framework Cocoa" "-framework Carbon" "-framework ApplicationServices"
        "-framework CoreText" "-framework CoreFoundation" "-framework CoreGraphics"
        "-framework IOKit")
      if(JPEG_LIB)
        target_link_libraries(${name} ${JPEG_LIB})
      endif()
      if(PNG_LIB)
        target_link_libraries(${name} ${PNG_LIB})
      endif()
      target_link_libraries(${name} z)
    elseif(FLTK_FOUND)
      target_link_libraries(${name} ${FLTK_LIBRARIES})
      if(APPLE)
        target_link_libraries(${name} "-framework CoreText" "-framework CoreFoundation")
      endif()
    else()
      target_link_libraries(${name} ${FLTK_LDFLAGS})
      if(APPLE)
        target_link_libraries(${name} "-framework CoreText" "-framework CoreFoundation")
      endif()
    endif()
  endif()
  if(USE_X11)
    target_link_libraries(${name} ${X11_LIBRARIES})
  endif()
  if(USE_CAIRO AND CAIRO_FOUND)
    target_link_libraries(${name} ${CAIRO_LIBRARIES})
  endif()
  if(USE_OPENGL AND USE_FLTK AND OpenGL_FOUND)
    target_link_libraries(${name} OpenGL::GL)
    if(APPLE)
      target_link_libraries(${name} "-framework OpenGL")
    endif()
  endif()
  if(UNIX)
    target_link_libraries(${name} m)
  endif()
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_astrolog_test(test_location   tests/test_location.cpp)
add_astrolog_test(test_validation tests/test_validation.cpp)
add_astrolog_test(test_formatting tests/test_formatting.cpp)
add_astrolog_test(test_roundtrip  tests/test_roundtrip.cpp)
add_astrolog_test(test_utils      tests/test_utils.cpp)

# Print configuration summary
message(STATUS "")
message(STATUS "Astrolog ${PROJECT_VERSION} Configuration Summary")
message(STATUS "====================================")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "FLTK Backend: ${USE_FLTK}")
if(FLTK_STATIC AND APPLE)
  message(STATUS "FLTK Linking: Static")
else()
  message(STATUS "FLTK Linking: Dynamic")
endif()
message(STATUS "X11 Backend: ${USE_X11}")
message(STATUS "Win32 Backend: ${USE_WIN}")
message(STATUS "Cairo Support: ${CAIRO_FOUND}")
if(BUNDLE_LIBS AND CAIRO_FOUND)
  message(STATUS "Cairo Bundling: Enabled")
endif()
message(STATUS "OpenGL Support: ${OpenGL_FOUND}")
message(STATUS "")
