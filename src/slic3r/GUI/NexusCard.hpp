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

#include <vector>

namespace Slic3r { namespace GUI {

// Paints Web-Slicer cards (rounded, outlined, soft drop shadow) on `parent` behind each group of
// child windows. A card covers the union of its shown windows, inflated by NEXUS_CARD_PAD.
// Call from the parent's wxEVT_PAINT handler.
inline void nexus_paint_cards(wxWindow* parent, wxPaintDC& dc, const std::vector<std::vector<wxWindow*>>& cards)
{
    wxGCDC     gdc(dc);
    const int  radius = parent->FromDIP(NEXUS_CARD_RADIUS);
    const int  dy     = parent->FromDIP(2);
    const bool dark   = StateColor::darkModeColorFor(*wxWHITE) != *wxWHITE;
    for (const auto& card : cards) {
        wxRect rc;
        for (const wxWindow* w : card)
            if (w && w->IsShown())
                rc = rc.IsEmpty() ? w->GetRect() : rc.Union(w->GetRect());
        if (rc.IsEmpty())
            continue;
        rc.Inflate(parent->FromDIP(NEXUS_CARD_PAD));

        // Shadow (CSS 0 2px 6px rgba(0,0,0,.2)): a few widening, fading layers shifted down.
        gdc.SetPen(*wxTRANSPARENT_PEN);
        for (int i = 3; i >= 1; --i) {
            wxRect sh = rc;
            sh.Offset(0, dy);
            sh.Inflate(parent->FromDIP(i));
            gdc.SetBrush(wxBrush(wxColour(0, 0, 0, dark ? 40 : 18)));
            gdc.DrawRoundedRectangle(sh, radius + parent->FromDIP(i));
        }

        gdc.SetPen(wxPen(StateColor::darkModeColorFor(wxColour(NEXUS_CARD_LINE)), 1));
        gdc.SetBrush(wxBrush(StateColor::darkModeColorFor(*wxWHITE)));
        gdc.DrawRoundedRectangle(rc, radius);
    }
}

}} // namespace Slic3r::GUI
