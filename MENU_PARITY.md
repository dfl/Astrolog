# FLTK vs Windows Menu Parity Analysis

## Summary

This document catalogs the gaps between FLTK and Windows GUI implementations.
Many features have been implemented - marked with [x] below.

---

## File Menu

### FLTK Has
- Open Chart...
- [x] Open Chart #2...
- Save Chart...
- Save As...
- [x] Save Program Settings...
- Export (SVG, PDF, Bitmap)
- Exit

### Windows Has (FLTK Missing)
- [ ] Save Chart Positions...
- [ ] Other Formats submenu:
  - [ ] Open Charts in Folder...
  - [ ] Save Chart List...
  - [ ] Save Chart Exchange...
  - [ ] Save Chart Quick*Chart...
- [x] Export Chart Text Output...
- [ ] Export Chart Metafile...
- [ ] Export Chart PostScript...
- [ ] Export Chart Wireframe...
- [ ] Export as Wallpaper submenu
- [ ] Open Bitmap submenu:
  - [ ] Open Chart Background...
  - [ ] Open World Map...
- [ ] Print...
- [ ] Print Setup...

---

## Edit Menu

### FLTK Has
- Copy
- Paste (placeholder)
- Command Line...

### Windows Has (FLTK Missing)
- [ ] Run Macro submenus (48 macros via F1-F12 with modifiers)
- [x] Copy Chart Text Output
- [ ] Copy Chart Bitmap
- [ ] Copy Chart Metafile
- [ ] Copy Chart PostScript
- [ ] Copy Chart Wireframe

---

## View Menu

### FLTK Has (now matches Windows structure)
- [x] Show Graphics toggle (v)
- [x] Window Settings submenu:
  - [x] Redraw Screen
  - [x] Clear Screen
  - [x] Size Chart to Window
  - [x] Size Window to Chart
  - [x] Full Screen (F11)
- [x] Show Interpretations toggle
- [x] Print Nearest Second toggle
- [x] Parallel Aspects toggle
- [x] Applying Aspects toggle

### Windows Has (FLTK Missing)
- [ ] Buffer Redraws
- [ ] Hourglass on Redraw
- [ ] Chart Resizes Window
- [ ] Window Resizes Chart
- [ ] Scroll Page Up/Down/Home/End
- [ ] Colored Text
- [ ] Set Colors...

---

## Chart Menu (NEW - split from View)

### FLTK Has
- [x] Standard Radix (V)
- [x] House Wheel
- [x] Aspect Midpoint Grid
- [x] Aspect List
- [x] Midpoint List
- [x] Local Horizon
- [x] Solar System Orbit
- [x] Gauquelin Sectors
- [x] Calendar
- [x] Influence
- [x] Esoteric
- [x] Astro-Graph
- [x] Ephemeris
- [x] Arabic Parts
- [x] Rising and Setting
- [x] Nearest Cities
- [x] Moons Chart
- [x] Transits... dialog
- [x] Progressions... dialog
- [x] Chart Settings... dialog
- [x] Chart Type... dialog

---

## Info Menu

### FLTK Has
- Set Chart Info...
- Chart for Now
- Set Chart #2 Info...
- [x] Default Chart Info...
- [x] Swap Chart #1 and #2
- Relationship submenu (No Relationship, Comparison, Synastry, Composite, Time Space Midpoint, Date Difference, Biorhythm, Transit and Natal, Progressed and Natal)

### Windows Has (FLTK Missing)
- [ ] Charts #3 Through #6...
- [ ] Chart List submenu:
  - [ ] Chart List...
  - [ ] Previous/Next Chart
  - [ ] First/Last Chart

---

## Settings Menu

### FLTK Has
- Sidereal Zodiac, Heliocentric
- House System submenu (all systems + Solar Chart, 3D Houses, Show Decans, Flip Signs)
- [x] House Settings submenu:
  - [x] Show Dwads
  - [x] Geodetic Houses
  - [x] Show Navamsas
  - [x] Indian Wheel Order
- Calculation Settings... (with Harmonic Chart Factor, Dwad Nesting Level)
- Display Settings...
- Graphics Settings...
- Aspect Settings...
- Object Restrictions...
- Transit Restrictions...
- Color Settings...
- Glyph Fonts submenu
- Include Minors/Cusps/Uranians/Dwarfs/Moons/Body Centers/Fixed Stars

- [x] Object Settings...
- [x] Star Restrictions...

### Windows Has (FLTK Missing)
- [x] More Object Settings...
- [ ] Planetary Moons submenu:
  - [ ] Moons Chart
  - [ ] Exoplanets Chart
  - [ ] Moon Restrictions...
  - [ ] Moon Object Settings...
  - [ ] Object Customization...
  - [ ] Star Customization...

---

## Graphics Menu

### FLTK Has (now matches Windows structure with 3D modes)
- [x] Draw Chart Sphere (X)
- [x] Draw World Map (W)
- [x] Draw Globe (G)
- [x] Draw Polar Globe (P)
- [x] Draw Telescope (T)
- [x] Draw Local 3D Horizon
- [x] 3D with OpenGL toggle
- [x] Reverse Background (x)
- [x] Monochrome (m)
- [x] Square Screen (Q)
- [x] Character Scale submenu (Decrease/Increase, Small/Medium/Large/Huge)
- [x] Chart Effects submenu (Border, Info, Sidebar, Thick Lines, Antialias, Labels, Glyphs on Aspects)
- [x] Map Effects submenu (Constellations, Stars, Cities, Equator, etc.)
- [x] Map Orientation submenu (Reset, Tilt North/South, Rotate West/East, Zoom In/Out)
- [x] Indian Style submenu
- [x] Modify Display (i)
- [x] Modify Chart (0)
- [x] Graphics Settings...

### Windows Has (FLTK Missing)
- [ ] Scribble Color submenu (16 colors)

---

## Animate Menu

### FLTK Has
- Do Animation
- Jump Rate submenu (including 1/10th, 1/100th, 1/1000th seconds)
- Jump Factor submenu
- Reverse Direction, Pause Animation, Timed Exposure
- Step Forward/Backward
- [x] Store Chart Info (o)
- [x] Recall Chart Info (O)
- Animation Settings...

### Windows Has (FLTK Missing)
(All major features now implemented)

---

## Help Menu

### FLTK Has
- About Astrolog...
- [x] Open Website
- [x] Open Changes Log
- [x] Show License
- List Signs/Objects/Aspects/Constellations/Planet Info/Rays/General Meanings/Switches/Obscure Switches/Keystrokes/Credits

### Windows Has (FLTK Missing)
- [ ] Open Website Mirror
- [ ] Open Data Files submenu:
  - [ ] Open Default Settings
  - [ ] Open Atlas
  - [ ] Open Time Zone Changes
  - [ ] Open Star List
  - [ ] Open Orbital Elements
  - [ ] Open Exoplanet List
- [ ] Setup submenu:
  - [ ] Create Program Group (User/All)
  - [ ] Create Desktop Icon
  - [ ] Install/Uninstall File Extensions

---

## Priority Recommendations

### High Priority (Core Functionality) - COMPLETED
1. [x] Aspect List chart
2. [x] Midpoint List
3. [x] Transits dialog
4. [x] Progressions dialog
5. [x] Chart Settings dialog
6. [x] Standard Radix / House Wheel distinction

### Medium Priority (Usability) - MOSTLY COMPLETED
1. [x] Show Interpretations
2. [x] Parallel Aspects / Applying Aspects toggles
3. [ ] Object Settings dialogs
4. [x] Character Scale controls
5. [x] Store/Recall Chart Info

### Lower Priority (Advanced)
1. [ ] Macro system
2. [ ] Wallpaper export
3. [ ] Print support
4. [ ] Setup/installation helpers

---

## Dialogs Comparison

### FLTK Dialogs Exist
- Set Chart Info (FShowDlgInfo)
- Calculation Settings (FShowDlgCalc) - has Harmonic/Dwad
- Display Settings (FShowDlgDisplay)
- Graphics Settings (FShowDlgGraphics)
- Aspect Settings (FShowDlgAspect)
- Object Restrictions (FShowDlgRestrict)
- Transit Restrictions (FShowDlgRestrictTrans)
- Color Settings (FShowDlgColor)
- Animation Settings (FShowDlgAnim)
- Command Line (FShowDlgCommand)
- Chart Type (FShowDlgChartType)
- [x] Transits (FShowDlgTransit)
- [x] Progressions (FShowDlgProgress)
- [x] Chart Settings (FShowDlgChartSettings)
- [x] Default Chart Info (FShowDlgDefaultInfo)

- [x] Object Settings (FShowDlgObject)
- [x] Star Restrictions (FShowDlgStar)

### Windows Dialogs (FLTK Missing)
- [ ] More Object Settings (cmdObject2 / dlgObject2)
- [ ] Chart List (cmdList / dlgList)
