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

// Neutral palette taken from the Web-Slicer design.
// Top tab bar (and the slice/print strip next to it). Mapped to NEXUS_BAR_DARK in dark mode.
#define NEXUS_BAR               "#2B2C2F"
#define NEXUS_BAR_DARK          "#18191B"
// Radius, in DIP, of the main tab-bar pills.
#define NEXUS_TAB_RADIUS        14
// Height of the sidebar's slice button and of the 3D view's toolbar row, in DIP.
#define NEXUS_BAR_HEIGHT        44
// The filament spools of the filament card's tiles, in DIP.
#define NEXUS_SPOOL_PX          40

// Sidebar cards (Web-Slicer step cards). The sidebar background shows between the cards; the cards
// are NEXUS_CARD (mapped to NEXUS_CARD_DARK in dark mode) with a thin outline.
#define NEXUS_SIDEBAR_BG        "#E0E0E0"
#define NEXUS_SIDEBAR_BG_DARK   "#18191B"
// Card fill (Web-Slicer --card). Its dark value differs from the dark window white (#2A2B2E) by one step
// so that StateColor's reverse lookup maps each back to its own light colour.
#define NEXUS_CARD              "#F4F4F4"
#define NEXUS_CARD_RGB          0xF4F4F4
#define NEXUS_CARD_DARK         "#2A2B2F"
#define NEXUS_CARD_LINE         "#B0B0B0"
#define NEXUS_CARD_LINE_DARK    "#55575C"
#define NEXUS_CARD_RADIUS       26 // DIP, as the Web-Slicer's cards
#define NEXUS_CARD_PAD          8  // DIP, inset of the card content from the card edge: at most a third of the
                                   // radius, or the square corners of the content stick out of the rounding
#define NEXUS_CARD_GAP          8  // DIP, sidebar background visible around and between cards
// Card title icon (the green dot): centred on the centre of the card's corner rounding, so that it sits
// concentric with the corner. Title height and left inset follow from the radius, pad and icon size.
#define NEXUS_CARD_ICON         28 // DIP
#define NEXUS_CARD_TITLE_H      (2 * (NEXUS_CARD_RADIUS - NEXUS_CARD_PAD))                   // DIP
#define NEXUS_CARD_ICON_INSET   (NEXUS_CARD_RADIUS - NEXUS_CARD_PAD - NEXUS_CARD_ICON / 2)   // DIP

// Settings group frames (Web-Slicer fieldset): a rounded NEXUS_CARD_LINE outline around each group of a
// settings page, the group title set into its top edge. DIP.
#define NEXUS_FRAME_RADIUS      12
#define NEXUS_FRAME_PAD         4  // outline outside the group's controls, left, right and below
#define NEXUS_FRAME_TITLE_INSET 8  // title from the group's left edge

// Device page background (Web-Slicer device view, a little darker than the sidebar).
#define NEXUS_DEVICE_BG         "#CDD0D5"
#define NEXUS_DEVICE_BG_DARK    "#0F1012"

// Outline of text inputs, combo boxes and spin inputs (Web-Slicer uses stronger lines than Orca's #DBDBDB).
#define NEXUS_INPUT_LINE_RGB    0xBDBDBD
#define NEXUS_INPUT_LINE        "#BDBDBD"
