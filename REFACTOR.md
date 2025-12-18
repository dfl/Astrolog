# Astrolog Code Quality TODO

Suggestions for improving readability and maintainability of the Astrolog codebase.

## High Priority

### 3. Use Descriptive Variable Names

The codebase uses cryptic single-letter variables extensively (e.g., `i, j, k` appears 466 times in calc.cpp alone).

**Suggested renames:**
| Current | Suggested |
|---------|-----------|
| `x1, y1, x2, y2` | `startX, startY, endX, endY` |
| `i, j, k` | `objectIdx, houseIdx, signIdx` (context-dependent) |
| `pch` | `charPtr` or `currentChar` |
| `sz` | `inputString` or more specific like `objectName` |
| `rT` | `tempReal` or describe purpose |
| `cpA, cpB` | `chartPosA, chartPosB` |

---

## Lower Priority

### 7. Add Algorithm Documentation

Complex calculations lack explanatory comments.

**Priority areas:**
- [ ] Ephemeris calculations in `sweph.cpp`, `swecl.cpp`
- [ ] House system calculations in `swehouse.cpp`
- [ ] Matrix transformations in `calc.cpp` (`CoorXform`, `CoorXformFast`)
- [ ] Chart casting logic

**Minimum documentation per function:**
- Purpose/algorithm overview
- Parameter descriptions
- Return value meaning
- References to astronomical formulas used

### 10. Add Debug Logging

Complex calculations have no observability for debugging.

- [ ] Add optional debug logging at key calculation points
- [ ] Create logging macros that compile out in release builds
- [ ] Focus on ephemeris and chart casting code paths

---

## Completed / Deprioritized

### 1. Break Up Large Functions

The `CastChart()` function in `calc.cpp` was 359+ lines with ~466 control flow statements.

**Status:** Complete. The following helper functions were extracted:
- [x] `CastChartValidation()` - input validation and early returns (line 1160)
- [x] `CastChartTime()` - time zone normalization and Julian Day calculation (line 1184)
- [x] `CastChartHouses()` - house cusp calculations (line 1238)
- [x] `CastChartPlanets()` - planet position calculations (line 1269)
- [x] `CastChartPOFAndCusps()` - Part of Fortune and house cusp objects (line 1319)
- [x] `CastChartModify()` - chart modifications (line 1366)
- [x] `CastChartFinalize()` - sorting and AstroExpression adjustments (line 1506)

The main `CastChart()` function is now ~25 lines calling these helpers in sequence.

### 2. Create Platform Abstraction Layer

Graphics code duplicated logic across platform conditionals (#ifdef X11, WIN, WCLI, FLTK).

**Status:** Complete. `xbackend.h` and `xbackend.cpp` provide a clean abstraction:
- [x] `GraphicsBackend` (GB) struct with function pointers for all drawing operations
- [x] Core operations: `PutColor`, `PutPixel`, `PutLine`, `PutRect`, `PutArc`, `PutEllipse`
- [x] Alpha blending support: `PutColorAlpha`
- [x] Text/font rendering: `PutGlyph`, `PutText`
- [x] Screen management: `ClearScreen`, `Flush`
- [x] Convenience macros: `GBSetColor`, `GBDrawPixel`, `GBDrawLine`, etc.
- [x] FLTK backend implementation (670 lines in xbackend.cpp)

### 4. Reorganize Header Files

`astrolog.h` was 2,324 lines with mixed concerns.

**Status:** Complete (commit `540559f`). Split into focused sub-headers while keeping `astrolog.h` as the single include point for backwards compatibility:
- [x] `astrolog_config.h` - Feature flags and platform configuration
- [x] `astrolog_constants.h` - Program constants (math, sizes, limits)
- [x] `astrolog_enums.h` - All enumerations (objects, aspects, house systems, etc.)
- [x] `astrolog_types.h` - Type definitions and structures (CI, CP, US, IS, GS, GI, etc.)
- [x] `astrolog_macros.h` - Macro functions (validation, math, graphics helpers)

**Benefits:**
- Backwards compatible: existing code only needs to include `astrolog.h`
- Faster incremental builds: modifying constants doesn't recompile type-dependent code
- Better organization: related definitions grouped logically
- Easier maintenance: find definitions faster in focused files

**Note:** `extern.h` already has good organization via comments grouping declarations by module.

### 5. Fix Unsafe String Operations

Replace `sprintf()` with `snprintf()` throughout the codebase.

**Status:** Addressed via two separate mechanisms:

1. **macOS blanket redirect** (commit 0c898ec) in `astrolog.h`:
   ```c
   #ifdef __APPLE__
   #define sprintf(buf, ...) snprintf(buf, __INT_MAX__, __VA_ARGS__)
   #endif
   ```
   Silences macOS deprecation warnings. Converting 1300+ calls individually was impractical.

2. **Windows `sprintf2` macro** (upstream, Walter Pullen, v7.80) in `astrolog_config.h`:
   ```c
   #ifdef PC
   #define sprintf2 snprintf
   #define S(sz) (sz), (int)sizeof(sz)
   #define SO(pch, sz) (pch), (sizeof(sz) - ((pch) - (sz)))
   #endif
   ```
   Opt-in for new code with explicit buffer size passing via `S()` and `SO()` helpers.

### 6. Enable Compiler Warnings

CMakeLists.txt previously disabled important warnings:
```cmake
-Wno-write-strings      # Hides string mutation issues
-Wno-format-security    # Hides format string bugs
```

**Status:** Complete (commit `8de69ca`). Fixed the underlying issues and removed warning suppressions:
- [x] `sweph.cpp`: Added `const` to 3 string literal assignments
- [x] `charts3.cpp`: Used explicit `"%s"` format string instead of function result
- [x] `general.cpp`: Used `%lx` format for `unsigned long` (dword) type
- [x] `CMakeLists.txt`: Removed `-Wno-write-strings` and `-Wno-format-security`

The compiler can now catch potential bugs in string handling and format string usage.

### 8. Consolidate Dialog Code

`wdialog.cpp` (Windows) and `fdialog.cpp` (FLTK) share similar validation and layout patterns.

- [x] Identify common dialog patterns
- [x] Extract shared validation logic
- [x] Create `DialogHelper` utilities for common operations (`xdialog.h`, `xdialog.cpp`)
- [~] Reduce maintenance burden for dual-platform dialogs (partial)

**Status:** Partially complete. Shared utilities exist in `xdialog.h`/`xdialog.cpp`:
- `ValidationResult` (VR) struct for structured validation
- Field validators: `ValidateMon`, `ValidateYea`, `ValidateDay`, `ValidateTim`, etc.
- Composite validator: `FValidateCI()` for full chart info validation
- Parsing helpers: `ParseDateFields`, `ParseTimeField`, `ParseZoneFields`, `ParseCoordFields`
- Formatting helpers: `FormatMon`, `FormatTim`, `FormatCI`, etc.
- Utility functions: `NClamp`, `RClamp`, `FInRange`

`fdialog.cpp` (FLTK) uses these shared helpers. `wdialog.cpp` (Windows) does not—it still uses inline `EnsureN`/`EnsureR` macros with direct `FValid*` calls.

**Recommendation:** Deprioritized. The core validation logic (`FValidMon`, etc.) is already shared at the lower level. The Windows code is mature and stable, and FLTK is the cross-platform path forward. Migration effort exceeds benefit.

---

## What's Already Good (Preserve These)

- **Modular architecture** - Graphics, calculations, and UI are well-separated
- **Compile-time feature system** - Feature flags are well-organized in astrolog.h
- **Multi-platform support** - Structure supports X11, Windows, FLTK, and CLI
- **Rich type system** - Enumerations for objects, houses, aspects are well-defined
- **Performance optimizations** - Inline functions and fast paths are thoughtfully placed

---

## Notes

- This codebase has 30+ years of history; changes should be incremental
- Prioritize changes that reduce maintenance burden for new platforms
- Consider backward compatibility with existing user configurations
