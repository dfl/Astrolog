cmake_minimum_required(VERSION 3.16)
project(astrolog VERSION 7.80 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_FLTK "Build with FLTK GUI" ON)
option(USE_X11 "Build with X11 GUI (Unix only)" OFF)
option(USE_WIN "Build with Win32 GUI (Windows only)" OFF)

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
  xcharts0.cpp
  xcharts1.cpp
  xcharts2.cpp
  xdata.cpp
  xdevice.cpp
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

# Create executable
add_executable(astrolog ${SOURCES})

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
endif()

if(USE_X11)
  target_include_directories(astrolog PRIVATE ${X11_INCLUDE_DIR})
  target_link_libraries(astrolog ${X11_LIBRARIES})
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
    -Wno-write-strings
    -Wno-format-security
  )
endif()

# Installation
install(TARGETS astrolog RUNTIME DESTINATION bin)

# Print configuration summary
message(STATUS "")
message(STATUS "Astrolog ${PROJECT_VERSION} Configuration Summary")
message(STATUS "====================================")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "FLTK Backend: ${USE_FLTK}")
message(STATUS "X11 Backend: ${USE_X11}")
message(STATUS "Win32 Backend: ${USE_WIN}")
message(STATUS "")
