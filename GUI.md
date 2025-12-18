# Astrolog GUI Build Guide

This fork adds a cross-platform GUI using FLTK, with a CMake build system.

## Requirements

- CMake 3.16+
- C++17 compiler
- FLTK 1.3+

### Installing FLTK

**macOS:**
```bash
brew install fltk
```

**Ubuntu/Debian:**
```bash
sudo apt install libfltk1.3-dev
```

**Fedora:**
```bash
sudo dnf install fltk-devel
```

**Windows (vcpkg):**
```bash
vcpkg install fltk
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

For a release build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## GUI Backend Options

The build system supports three mutually exclusive GUI backends:

| Option | Description | Default |
|--------|-------------|---------|
| `USE_FLTK` | Cross-platform FLTK GUI | ON |
| `USE_X11` | Native X11 (Unix only) | OFF |
| `USE_WIN` | Native Win32 (Windows only) | OFF |

To select a different backend:
```bash
cmake -DUSE_FLTK=OFF -DUSE_X11=ON ..
```

## Troubleshooting

If CMake can't find FLTK:
```bash
# Verify FLTK installation
fltk-config --version
fltk-config --cxxflags
fltk-config --ldflags
```

## Releasing to GitHub

To update the macOS release on dfl/Astrolog:

```bash
# 1. Build the app
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# 2. Create the zip
zip -r Astrolog-macOS.zip Astrolog.app

# 3. Delete old asset and upload new one
gh release delete-asset v7.80-gui Astrolog-macOS.zip --repo dfl/Astrolog --yes
gh release upload v7.80-gui Astrolog-macOS.zip --repo dfl/Astrolog

# 4. Verify
gh release view v7.80-gui --repo dfl/Astrolog
```
