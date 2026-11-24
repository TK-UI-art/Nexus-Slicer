#pragma once

#include "NexusTheme.hpp"
#include "Widgets/StateColor.hpp"
#include "Widgets/StaticLine.hpp"

#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/sizer.h>
#include <wx/window.h>

namespace Slic3r { namespace GUI {

// The union of the shown windows a sizer lays out, nested sizers included.
inline void nexus_union_shown_windows(const wxSizer* sizer, wxRect& rc)
{
    for (const wxSizerItem* item : sizer->GetChildren()) {
        if (!item->IsShown())
            continue;
        if (const wxWindow* w = item->GetWindow()) {
            if (w->IsShown())
                rc = rc.IsEmpty() ? w->GetRect() : rc.Union(w->GetRect());
        } else if (const wxSizer* sub = item->GetSizer())
            nexus_union_shown_windows(sub, rc);
    }
}

// Paints the frames of the settings groups on `parent` (Web-Slicer fieldset: a rounded outline around
// the group, its title set into the top edge). A group is framed through its title, a StaticLine
// child of `parent` with a frame sizer (OptionsGroup::nexus_frame). Call from the parent's wxEVT_PAINT
// handler; the titles paint over the top edge themselves.
inline void nexus_paint_group_frames(wxWindow* parent, wxPaintDC& dc)
{
    wxGCDC gdc(dc);
    const wxPen line(StateColor::darkModeColorFor(wxColour(NEXUS_CARD_LINE)), 1);
    const int   gap = parent->FromDIP(5);
    const int pad    = parent->FromDIP(NEXUS_FRAME_PAD);
    const int radius = parent->FromDIP(NEXUS_FRAME_RADIUS);
    for (wxWindow* child : parent->GetChildren()) {
        auto* title = dynamic_cast<StaticLine*>(child);
        if (title == nullptr || title->GetFrameSizer() == nullptr || !title->IsShown())
            continue;
        wxRect rc;
        nexus_union_shown_windows(title->GetFrameSizer(), rc);
        const wxRect t = title->GetRect();
        // The title's own line, after the text, runs at its vertical centre.
        const int top = t.y + t.height / 2;
        const wxRect frame(wxPoint(rc.GetLeft() - pad, top), wxPoint(rc.GetRight() + pad, rc.GetBottom() + pad + parent->FromDIP(2)));
        gdc.SetPen(line);
        gdc.SetBrush(*wxTRANSPARENT_BRUSH);
        gdc.DrawRoundedRectangle(frame, radius);
        // A gap before the title, as a legend's padding.
        gdc.SetPen(*wxTRANSPARENT_PEN);
        gdc.SetBrush(wxBrush(parent->GetBackgroundColour()));
        gdc.DrawRectangle(t.x - gap, top - 2, gap, 4);
    }
}

}} // namespace Slic3r::GUI
