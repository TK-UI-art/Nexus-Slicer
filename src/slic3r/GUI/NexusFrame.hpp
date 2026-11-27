#pragma once

#include "NexusTheme.hpp"
#include "OG_CustomCtrl.hpp"
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

#include <algorithm>

namespace Slic3r { namespace GUI {

// The union of the shown windows a sizer lays out, nested sizers included.
inline void nexus_union_shown_windows(const wxSizer* sizer, wxRect& rc)
{
    for (const wxSizerItem* item : sizer->GetChildren()) {
        if (!item->IsShown())
            continue;
        if (const wxWindow* w = item->GetWindow()) {
            if (!w->IsShown())
                continue;
            wxRect wr = w->GetRect();
            // A settings grid stretches over the page's width, while its lines end at their last field: the
            // frame follows the fields, not the empty space right of them.
            if (dynamic_cast<const OG_CustomCtrl*>(w)) {
                int right = 0;
                for (const wxWindow* field : w->GetChildren())
                    if (field->IsShown())
                        right = std::max(right, field->GetRect().GetRight() + 1);
                if (right > 0)
                    wr.width = std::min(wr.width, right);
            }
            rc = rc.IsEmpty() ? wr : rc.Union(wr);
        } else if (const wxSizer* sub = item->GetSizer())
            nexus_union_shown_windows(sub, rc);
    }
}

// The outline of a framed settings group, in the coordinates of the title's parent.
inline wxRect nexus_group_frame_rect(const StaticLine* title)
{
    const wxWindow* parent = title->GetParent();
    wxRect rc;
    nexus_union_shown_windows(title->GetFrameSizer(), rc);
    const wxRect t = title->GetRect();
    const int pad = parent->FromDIP(NEXUS_FRAME_PAD);
    // The title's own line, after the text, runs at its vertical centre.
    const int top = t.y + t.height / 2;
    return wxRect(wxPoint(rc.GetLeft() - pad, top), wxPoint(rc.GetRight() + pad, rc.GetBottom() + pad + parent->FromDIP(2)));
}

// Draws the outline `frame` with `dc`'s window as the painter.
inline void nexus_draw_group_frame(const wxWindow* win, wxGCDC& gdc, const wxRect& frame)
{
    gdc.SetPen(wxPen(StateColor::darkModeColorFor(wxColour(NEXUS_CARD_LINE)), 1));
    gdc.SetBrush(*wxTRANSPARENT_BRUSH);
    gdc.DrawRoundedRectangle(frame, win->FromDIP(NEXUS_FRAME_RADIUS));
}

// Paints the frames of the settings groups on `parent` (Web-Slicer fieldset: a rounded outline around
// the group, its title set into the top edge). A group is framed through its title, a StaticLine
// child of `parent` with a frame sizer (OptionsGroup::nexus_frame). Call from the parent's wxEVT_PAINT
// handler; the titles paint over the top edge themselves. The settings grid stretches under the frame's
// right edge and paints that part itself (nexus_paint_frame_over).
inline void nexus_paint_group_frames(wxWindow* parent, wxPaintDC& dc)
{
    wxGCDC gdc(dc);
    const int gap = parent->FromDIP(5);
    for (wxWindow* child : parent->GetChildren()) {
        auto* title = dynamic_cast<StaticLine*>(child);
        if (title == nullptr || title->GetFrameSizer() == nullptr || !title->IsShown())
            continue;
        const wxRect frame = nexus_group_frame_rect(title);
        nexus_draw_group_frame(parent, gdc, frame);
        // A gap before the title, as a legend's padding.
        const wxRect t = title->GetRect();
        gdc.SetPen(*wxTRANSPARENT_PEN);
        gdc.SetBrush(wxBrush(parent->GetBackgroundColour()));
        gdc.DrawRectangle(t.x - gap, frame.y - 2, gap, 4);
    }
}

// Paints, on `win`, a sibling window lying over the frame of the group titled `title`, the part of the
// frame it covers. Child windows are opaque on macOS and Windows, so the outline painted on the parent
// does not show through them.
inline void nexus_paint_frame_over(wxWindow* win, wxPaintDC& dc, const StaticLine* title)
{
    if (title == nullptr || title->GetFrameSizer() == nullptr || !title->IsShown() || title->GetParent() != win->GetParent())
        return;
    wxRect frame = nexus_group_frame_rect(title);
    frame.Offset(-win->GetPosition());
    wxGCDC gdc(dc);
    nexus_draw_group_frame(win, gdc, frame);
}

}} // namespace Slic3r::GUI
