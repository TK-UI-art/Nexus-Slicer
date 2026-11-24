#pragma once

#include "NexusTheme.hpp"
#include "Widgets/StateColor.hpp"
#include "Widgets/StaticBox.hpp"

#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>
#include <wx/window.h>

#include <iterator>
#include <vector>

namespace Slic3r { namespace GUI {

// The card fill in the current theme.
inline wxColour nexus_card_colour() { return StateColor::darkModeColorFor(wxColour(NEXUS_CARD)); }

// Gives the windows of a card the card fill. OrcaSlicer's panels, labels and inputs are white (the window
// white of the dark theme in dark mode); every window in `root`'s subtree with that background takes the
// card colour instead, and the boxed widgets (inputs, combo boxes, buttons) get their white states replaced,
// so inputs read as outlined fields on the card, as in the Web-Slicer. Pop-ups and dialogs are skipped.
// Idempotent; call it again after a card builds new windows.
inline void nexus_apply_card_colour(wxWindow* root)
{
    if (root == nullptr)
        return;
    const wxColour card = nexus_card_colour();
    if (auto* box = dynamic_cast<StaticBox*>(root))
        box->ReplaceBackgroundColor(*wxWHITE, wxColour(NEXUS_CARD));
    // Compared as light colours: the window white or card fill of either theme (a theme switch maps them).
    const wxColour bg    = root->GetBackgroundColour();
    const wxColour light = StateColor::lightModeColorFor(bg);
    if ((light == *wxWHITE || light == wxColour(NEXUS_CARD)) && bg != card) {
        root->SetBackgroundColour(card);
        root->Refresh();
    }
    for (wxWindow* child : root->GetChildren())
        if (!child->IsTopLevel())
            nexus_apply_card_colour(child);
}

// The Web-Slicer drop shadow (CSS 0 2px 6px rgba(0,0,0,.2)) behind the rounded rect `rc`, as stacked layers
// spreading 0..6 DIP whose summed opacity falls off like the 6 px blur. ImGuiWrapper::draw_drop_shadow draws
// the same shadow on the 3D view.
inline void nexus_paint_shadow(const wxWindow* win, wxGCDC& gdc, const wxRect& rc, int radius)
{
    static constexpr int alpha[] = {9, 6, 5, 3, 3, 2, 1};
    const bool dark = StateColor::darkModeColorFor(*wxWHITE) != *wxWHITE;
    gdc.SetPen(*wxTRANSPARENT_PEN);
    for (int k = int(std::size(alpha)) - 1; k >= 0; --k) {
        const int spread = win->FromDIP(k);
        wxRect    sh     = rc;
        sh.Offset(0, win->FromDIP(2));
        sh.Inflate(spread);
        gdc.SetBrush(wxBrush(wxColour(0, 0, 0, dark ? alpha[k] * 7 / 4 : alpha[k])));
        gdc.DrawRoundedRectangle(sh, radius + spread);
    }
}

// Paints Web-Slicer cards (rounded, outlined, soft drop shadow) on `parent` behind each group of
// child windows. A card covers the union of its shown windows, inflated by `pad_dip`.
// Call from the parent's wxEVT_PAINT handler.
inline void nexus_paint_cards(wxWindow* parent, wxPaintDC& dc, const std::vector<std::vector<wxWindow*>>& cards,
                              int radius_dip = NEXUS_CARD_RADIUS, int pad_dip = NEXUS_CARD_PAD)
{
    wxGCDC    gdc(dc);
    const int radius = parent->FromDIP(radius_dip);
    for (const auto& card : cards) {
        wxRect rc;
        for (const wxWindow* w : card)
            if (w && w->IsShown())
                rc = rc.IsEmpty() ? w->GetRect() : rc.Union(w->GetRect());
        if (rc.IsEmpty())
            continue;
        rc.Inflate(parent->FromDIP(pad_dip));

        nexus_paint_shadow(parent, gdc, rc, radius);
        gdc.SetPen(wxPen(StateColor::darkModeColorFor(wxColour(NEXUS_CARD_LINE)), 1));
        gdc.SetBrush(wxBrush(nexus_card_colour()));
        gdc.DrawRoundedRectangle(rc, radius);
    }
}

}} // namespace Slic3r::GUI
