#pragma once

#include "NexusTheme.hpp"
#include "Widgets/StateColor.hpp"

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
// child windows. A card covers the union of its shown windows, inflated by NEXUS_CARD_PAD.
// Call from the parent's wxEVT_PAINT handler.
inline void nexus_paint_cards(wxWindow* parent, wxPaintDC& dc, const std::vector<std::vector<wxWindow*>>& cards)
{
    wxGCDC    gdc(dc);
    const int radius = parent->FromDIP(NEXUS_CARD_RADIUS);
    for (const auto& card : cards) {
        wxRect rc;
        for (const wxWindow* w : card)
            if (w && w->IsShown())
                rc = rc.IsEmpty() ? w->GetRect() : rc.Union(w->GetRect());
        if (rc.IsEmpty())
            continue;
        rc.Inflate(parent->FromDIP(NEXUS_CARD_PAD));

        nexus_paint_shadow(parent, gdc, rc, radius);
        gdc.SetPen(wxPen(StateColor::darkModeColorFor(wxColour(NEXUS_CARD_LINE)), 1));
        gdc.SetBrush(wxBrush(StateColor::darkModeColorFor(*wxWHITE)));
        gdc.DrawRoundedRectangle(rc, radius);
    }
}

}} // namespace Slic3r::GUI
