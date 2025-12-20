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
- [ ] Export Chart Text Output...
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
- [ ] Copy Chart Text Output
- [ ] Copy Chart Bitmap
- [ ] Copy Chart Metafile
- [ ] Copy Chart PostScript
- [ ] Copy Chart Wireframe

---

## View Menu

### FLTK Has
- [x] Show Graphics toggle (v)
- [x] Standard Radix (V)
- [x] House Wheel
- Aspect Midpoint Grid
- [x] Aspect List
- [x] Midpoint List
- Horizon Chart, Orbit Chart, Gauquelin Sectors
- Calendar, Influence, Astro-Graph, Ephemeris
- [x] Esoteric
- [x] Arabic Parts
- Rising and Setting, Globe, Sphere, Local Horizon
- Telescope, Polar, World Map
- [x] Nearest Cities
- [x] Moons Chart
- 3D with OpenGL toggle
- [x] Window Settings submenu (Redraw Screen, Clear Screen)
- [x] Show Interpretations toggle
- [x] Print Nearest Second toggle
- [x] Parallel Aspects toggle
- [x] Applying Aspects toggle
- [x] Transits... dialog
- [x] Progressions... dialog
- [x] Chart Settings... dialog

### Windows Has (FLTK Missing)
- [ ] Window Settings submenu items:
  - [ ] Buffer Redraws
  - [ ] Hourglass on Redraw
  - [ ] Chart Resizes Window
  - [ ] Window Resizes Chart
  - [ ] Size Chart to Window
  - [ ] Size Window to Chart
  - [ ] Size Window Full Screen
  - [ ] Scroll Page Up/Down/Home/End
- [ ] Colored Text
- [ ] Set Colors...

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

### Windows Has (FLTK Missing)
- [ ] Object Settings...
- [ ] More Object Settings...
- [ ] Star Restrictions...
- [ ] Planetary Moons submenu:
  - [ ] Moons Chart
  - [ ] Exoplanets Chart
  - [ ] Moon Restrictions...
  - [ ] Moon Object Settings...
  - [ ] Object Customization...
  - [ ] Star Customization...

---

## Graphics Menu

### FLTK Has
- Reverse Background, Monochrome, Show Border
- Show Chart Info, Thicker Lines, Show Glyph Labels
- Show Glyphs on Aspects, Show House Details, Show Equator
- Modify Display
- [x] Modify Chart (0)
- [x] Square Screen
- [x] Show Info Sidebar
- [x] Antialias Lines
- [x] Character Scale submenu (Decrease/Increase, Small/Medium/Large/Huge)
- [x] Text Scale submenu (Decrease/Increase)
- Map Effects submenu
- [x] Map Orientation submenu (Reset, Tilt North/South, Rotate West/East, Zoom In/Out)
- Indian Style submenu

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
- Documentation (placeholder)
- List Signs/Objects/Aspects/Constellations/Planet Info/Rays/General Meanings/Switches/Obscure Switches/Keystrokes/Credits

### Windows Has (FLTK Missing)
- [ ] More Documentation submenu:
  - [ ] Open Changes
  - [ ] Open License
  - [ ] Open Website
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

### Windows Dialogs (FLTK Missing)
- [ ] Object Settings (cmdObject / dlgObject)
- [ ] More Object Settings (cmdObject2 / dlgObject2)
- [ ] Star Restrictions (cmdStar / dlgStar)
- [ ] Chart List (cmdList / dlgList)
