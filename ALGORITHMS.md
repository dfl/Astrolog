# Astrolog Algorithm Reference

This document describes the key astronomical algorithms used in Astrolog's native calculation routines. For Swiss Ephemeris algorithms, see the Swiss Ephemeris documentation.

## Table of Contents

1. [Coordinate Systems](#coordinate-systems)
2. [Coordinate Transformations](#coordinate-transformations)
3. [Julian Day Calculations](#julian-day-calculations)
4. [Planetary Position Calculations](#planetary-position-calculations)
5. [House System Calculations](#house-system-calculations)

---

## Coordinate Systems

Astrolog uses several spherical coordinate systems:

### Ecliptic Coordinates
- **Longitude**: 0-360 degrees along the ecliptic (zodiac)
- **Latitude**: Degrees north/south of the ecliptic plane
- Reference: The Sun's apparent path through the sky

### Equatorial Coordinates
- **Right Ascension (RA)**: 0-360 degrees along celestial equator
- **Declination**: Degrees north/south of celestial equator
- Reference: Earth's equatorial plane extended to the sky

### Horizon (Local) Coordinates
- **Azimuth**: 0-360 degrees from north
- **Altitude**: Degrees above/below horizon
- Reference: Observer's local horizon

### Prime Vertical Coordinates
- Used for Campanus houses
- Reference: The great circle through zenith, east, and west points

---

## Coordinate Transformations

### CoorXform() - Spherical Coordinate Rotation

**Location**: `calc.cpp:880`

Rotates spherical coordinates around the X-axis (direction of 0 degrees longitude). This is the fundamental transformation for converting between coordinate systems.

#### Mathematical Basis

Given input coordinates (azi, alt) and tilt angle T:

1. **Convert to Cartesian**:
   ```
   x = cos(alt) * cos(azi)
   y = cos(alt) * sin(azi)
   z = sin(alt)
   ```

2. **Apply rotation matrix Rx(T)**:
   ```
   | 1     0       0      |   | x |   |    x'    |
   | 0   cos(T)  -sin(T)  | * | y | = |    y'    |
   | 0   sin(T)   cos(T)  |   | z |   |    z'    |
   ```

   Which gives:
   ```
   x' = x
   y' = y*cos(T) - z*sin(T)
   z' = y*sin(T) + z*cos(T)
   ```

3. **Convert back to spherical**:
   ```
   new_azi = atan2(y', x')
   new_alt = asin(z')
   ```

#### Common Uses (via macros in extern.h)

| Macro | Tilt Angle | Purpose |
|-------|------------|---------|
| `EclToEqu(lon, lat)` | +obliquity (~23.4 deg) | Ecliptic to equatorial |
| `EquToEcl(lon, lat)` | -obliquity | Equatorial to ecliptic |
| `EquToLocal(lon, lat, T)` | co-latitude | Equatorial to horizon |

### Other Coordinate Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `RecToPol()` | calc.cpp:814 | 2D rectangular to polar |
| `SphToRec()` | calc.cpp:823 | 3D spherical to rectangular |
| `RecToSph3()` | calc.cpp:836 | 3D rectangular to spherical |
| `PolToRec()` | matrix.cpp:247 | 2D polar to rectangular |
| `RecToSph()` | matrix.cpp:258 | Complex spherical conversion |
| `RecToSph2()` | matrix.cpp:516 | Orbital plane rotation |

---

## Julian Day Calculations

**Location**: `calc.cpp:66`, `matrix.cpp:176`

The Julian Day is a continuous count of days since January 1, 4713 BCE. It provides a uniform time scale for astronomical calculations.

### MdyToJulian / MatrixMdyToJulian

Converts calendar date to Julian Day number.

**Algorithm** (Gregorian calendar):
```
im = 12*(year + 4800) + month - 3
j = (2*(im%12) + 7 + 365*im) / 12
j += day + im/48 - 32083
if j > 2299171:  // Oct 15, 1582
    j += im/4800 - im/1200 + 38
```

### Key Reference Points

| Date | Julian Day |
|------|------------|
| Jan 1, 4713 BCE | 0 |
| Oct 15, 1582 (Gregorian start) | 2299161 |
| Jan 1, 2000 12:00 UT | 2451545.0 |

---

## Planetary Position Calculations

**Location**: `matrix.cpp:594` (ComputePlanets)

### Keplerian Orbital Mechanics

Planets are positioned using classical two-body orbital mechanics:

#### Step 1: Mean Anomaly (M)

The angular position if the orbit were circular:
```
M = M0 + n*t
```
Where:
- M0 = mean anomaly at epoch
- n = mean motion (degrees per century)
- t = time from epoch in Julian centuries

#### Step 2: Solve Kepler's Equation

Relate mean anomaly to eccentric anomaly (E):
```
M = E - e*sin(E)
```

Solved iteratively:
```c
EA = M;
for (i = 1; i <= 5; i++)
    EA = M + e*sin(EA);
```

Typically converges in 3-5 iterations for planetary eccentricities.

#### Step 3: Perifocal Coordinates

Position in the orbital plane:
```
x = a * (cos(E) - e)           // toward perihelion
y = a * sqrt(1-e^2) * sin(E)   // perpendicular in orbital plane
```
Where:
- a = semi-major axis (AU)
- e = eccentricity

#### Step 4: Velocity Coordinates

For calculating apparent daily motion:
```
n' = 0.01720209 / (a^1.5 * (1 - e*cos(E)))
vx = -a * n' * sin(E)
vy = a * n' * sqrt(1-e^2) * cos(E)
```

#### Step 5: Rotation to Ecliptic

Apply three Euler angles via `RecToSph2()`:
1. **Argument of Perihelion (omega)**: Rotate within orbital plane
2. **Longitude of Ascending Node (Omega)**: Rotate around ecliptic pole
3. **Inclination (i)**: Tilt orbital plane to ecliptic

### Orbital Elements (struct OE)

Each planet's orbit is defined by polynomial coefficients evaluated at time T (Julian centuries from J2000):

| Element | Fields | Description |
|---------|--------|-------------|
| Mean Anomaly | ma0, ma1, ma2 | M = ma0 + ma1*T + ma2*T^2 |
| Eccentricity | ec0, ec1, ec2 | e = ec0 + ec1*T + ec2*T^2 |
| Semi-major Axis | sma | In AU (constant for most planets) |
| Arg. of Perihelion | ap0, ap1, ap2 | omega polynomial |
| Ascending Node | an0, an1, an2 | Omega polynomial |
| Inclination | in0, in1, in2 | i polynomial |

### Error Corrections

**Location**: `matrix.cpp:533` (ErrorCorrect)

For Jupiter through Pluto, harmonic correction terms are added to account for mutual gravitational perturbations. These are Fourier series approximations stored in `rErrorData[]`.

---

## House System Calculations

**Location**: `matrix.cpp:344-517`

House systems divide the celestial sphere into 12 sectors. Each system uses different geometric principles.

### Overview of House Systems

| System | Type | Principle | Polar Behavior |
|--------|------|-----------|----------------|
| Placidus | Time | Semi-arc division | Fails |
| Koch | Time | Birth horizon projection | Fails |
| Campanus | Space | Prime vertical division | Works |
| Meridian | Space | Equator division, ecliptic projection | Works |
| Regiomontanus | Space | Equator division, horizon projection | Works |
| Morinus | Space | Equator division, parallel projection | Works |
| Topocentric | Hybrid | Modified Placidus | Works |

### Key Reference Points

All house systems share:
- **MC (Midheaven)**: Ecliptic point on the meridian (10th cusp)
- **ASC (Ascendant)**: Ecliptic point on the eastern horizon (1st cusp)

### Placidus Houses

**Location**: `matrix.cpp:369-409`

The most popular system, based on the time it takes a point to move from horizon to meridian (semi-diurnal arc).

For intermediate cusps, iteratively solve:
```
cos(RAMC + H) = -tan(decl) * tan(lat) / F
```
Where F is a fraction of the semi-arc (1/3 for 11th/3rd, 2/3 for 12th/2nd).

**Limitation**: Fails when `|tan(decl) * tan(lat)| > 1`, which occurs at high latitudes.

### Koch Houses

**Location**: `matrix.cpp:411-428`

Projects the birth moment's horizon onto the ecliptic:
```
A1 = asin(sin(RA) * tan(lat) * tan(obliquity))
A3 = RA + D + A2*A1
cusp = atan2(sin(A3), cos(A3)*cos(obl) - K*tan(lat)*sin(obl))
```

### Campanus Houses

**Location**: `matrix.cpp:430-447`

Divides the prime vertical into 30-degree segments:
```
DN = atan(tan(30*i) * cos(lat))
cusp = atan2(sin(RA+DN), cos(RA+DN)*cos(obl) - sin(DN)*tan(lat)*sin(obl))
```

### Meridian (Axial) Houses

**Location**: `matrix.cpp:449-458`

Simplest space-based system - divides the equator:
```
cusp = atan2(sin(RA + 30*i), cos(RA + 30*i) * cos(obliquity))
```

### Topocentric Houses

**Location**: `matrix.cpp:486-517`

Uses different effective latitudes for different houses to approximate Placidus behavior at extreme latitudes:
- Houses 5, 8: lat' = atan(tan(lat)/3)
- Houses 6, 9: lat' = atan(tan(lat)/1.5)
- Houses 1, 4, 7, 10: actual latitude

---

## References

1. Meeus, Jean. *Astronomical Algorithms*. Willmann-Bell, 1991.
2. Erlewine, Michael. *Manual of Computer Programming for Astrologers*. Matrix Software.
3. Swiss Ephemeris documentation: https://www.astro.com/swisseph/
4. Astronomical Almanac (annual). US Naval Observatory.
