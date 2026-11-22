#pragma once

// Nexus Slicer colour theme.
//
// Single source of truth for the accent colour family that replaces OrcaSlicer's teal (#009688).
// The values follow the "Web-Slicer 2.0" design: a bright green used for filled surfaces with dark
// text on top, and a darker green for accent-coloured text and links so it stays readable on white.
//
// The colours are macros so that they compose with every form the GUI code uses:
//   string literals      wxColour(NEXUS_ACCENT), StateColor("..."), "color:" NEXUS_ACCENT ";"
//   0xRRGGBB integers    std::make_pair(NEXUS_ACCENT_RGB, StateColor::Hovered)   (StateColor order)
//   byte triplets        wxColour(NEXUS_ACCENT_I), ImColor(NEXUS_ACCENT_I, alpha)
//   float triplets       ImVec4(NEXUS_ACCENT_F, alpha)
// Keep all four spellings of a colour in sync when changing it.
//
// Dark mode: the fill colours keep the same bright green in both themes (as in Web-Slicer 2.0) and
// have no entry in StateColor's gDarkColors map. The text colour and the tints are mapped there.

// Accent fill: buttons, toggles, selection borders, progress bars, icons.
#define NEXUS_ACCENT            "#69E84A"
#define NEXUS_ACCENT_RGB        0x69E84A
#define NEXUS_ACCENT_I          105, 232, 74
#define NEXUS_ACCENT_F          0.412f, 0.910f, 0.290f

// Accent fill, hovered.
#define NEXUS_ACCENT_HOVER      "#55D636"
#define NEXUS_ACCENT_HOVER_RGB  0x55D636
#define NEXUS_ACCENT_HOVER_I    85, 214, 54
#define NEXUS_ACCENT_HOVER_F    0.333f, 0.839f, 0.212f

// Accent fill, pressed.
#define NEXUS_ACCENT_PRESSED    "#4BC72E"
#define NEXUS_ACCENT_PRESSED_I  75, 199, 46

// Lighter accent: hovered checked buttons, the "#00FF00" placeholder in SVG icons.
#define NEXUS_ACCENT_LIGHT      "#A5F293"
#define NEXUS_ACCENT_LIGHT_RGB  0xA5F293

// Text drawn on top of an accent fill (both themes).
#define NEXUS_ON_ACCENT         "#101010"
#define NEXUS_ON_ACCENT_F       0.063f, 0.063f, 0.063f

// Accent-coloured text, links and thin lines on light backgrounds (contrast 4.4:1 on white).
// Mapped to NEXUS_ACCENT_TEXT_DARK in dark mode.
#define NEXUS_ACCENT_TEXT       "#278A1A"
#define NEXUS_ACCENT_TEXT_I     39, 138, 26
#define NEXUS_ACCENT_TEXT_DARK  "#7AEB5E"

// Accent tints for selected / focused backgrounds (accent at 10 % and 25 % over white).
#define NEXUS_ACCENT_TINT10     "#F0FDED"
#define NEXUS_ACCENT_TINT10_RGB 0xF0FDED
#define NEXUS_ACCENT_TINT25     "#D9F9D2"
#define NEXUS_ACCENT_TINT25_RGB 0xD9F9D2
#define NEXUS_ACCENT_TINT10_DARK "#2F3D2D"
#define NEXUS_ACCENT_TINT25_DARK "#3A5A35"
