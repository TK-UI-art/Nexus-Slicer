#pragma once

#include "NexusCard.hpp"
#include "NexusTheme.hpp"
#include "Widgets/Label.hpp"
#include "Widgets/StateColor.hpp"
#include "wxExtensions.hpp"

#include <wx/bitmap.h>
#include <wx/brush.h>
#include <wx/control.h>
#include <wx/cursor.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/image.h>
#include <wx/window.h>

#include <algorithm>
#include <functional>
#include <string>

namespace Slic3r { namespace GUI {

// The fill of a tile on a sidebar card, in the current theme.
inline wxColour nexus_tile_colour()
{
    const bool dark = StateColor::darkModeColorFor(*wxWHITE) != *wxWHITE;
    return wxColour(dark ? "#3A3C41" : "#E4E4E4");
}

// A tile of the sidebar cards (Web-Slicer tile): a rounded grey field with an icon or picture above its label.
// A disabled tile shows muted and takes no clicks.
class NexusTile : public wxWindow
{
public:
    NexusTile(wxWindow* parent, const wxString& label, const std::string& icon, int icon_px, std::function<void(NexusTile*)> on_click,
              const wxImage& picture = wxImage())
        : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE)
        , m_label(label)
        , m_icon_name(icon)
        , m_icon_px(icon_px)
        , m_picture(picture)
        , m_on_click(std::move(on_click))
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetFont(Label::Body_11);
        SetToolTip(label);
        msw_rescale();
        Bind(wxEVT_PAINT, &NexusTile::on_paint, this);
        Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent&) { m_hover = true; Refresh(); });
        Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent&) { m_hover = false; Refresh(); });
        Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) {
            if (m_on_click && IsEnabled())
                m_on_click(this);
        });
        SetCursor(wxCursor(wxCURSOR_HAND));
    }

    // The height of a tile with an icon of `icon_px`, both in DIP.
    static int min_height(int icon_px) { return icon_px + 30; }

    // A tile showing a picture (a project thumbnail) instead of an icon resource.
    NexusTile(wxWindow* parent, const wxString& label, const wxImage& picture, int icon_px, std::function<void(NexusTile*)> on_click)
        : NexusTile(parent, label, std::string(), icon_px, std::move(on_click), picture)
    {}

    void msw_rescale()
    {
        if (m_picture.IsOk()) {
            const int    px = FromDIP(m_icon_px);
            const double k  = std::min(double(px) / m_picture.GetWidth(), double(px) / m_picture.GetHeight());
            m_icon_bmp      = wxBitmap(m_picture.Scale(std::max(1, int(m_picture.GetWidth() * k)), std::max(1, int(m_picture.GetHeight() * k)),
                                                       wxIMAGE_QUALITY_HIGH));
        } else
            m_icon_bmp = ScalableBitmap(this, m_icon_name, m_icon_px).bmp();
        SetMinSize(wxSize(FromDIP(64), FromDIP(min_height(m_icon_px))));
        Refresh();
    }

private:
    void on_paint(wxPaintEvent&)
    {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(wxBrush(nexus_card_colour()));
        dc.Clear();
        wxGCDC gdc(dc);

        const bool   dark = StateColor::darkModeColorFor(*wxWHITE) != *wxWHITE;
        const wxRect rc   = GetClientRect();
        const wxColour fill = m_hover && IsEnabled() ? wxColour(dark ? NEXUS_ACCENT_TINT25_DARK : NEXUS_ACCENT_TINT25) : nexus_tile_colour();
        gdc.SetPen(*wxTRANSPARENT_PEN);
        gdc.SetBrush(wxBrush(fill));
        gdc.DrawRoundedRectangle(rc, FromDIP(10));

        gdc.SetFont(GetFont());
        gdc.SetTextForeground(!IsEnabled() ? wxColour(dark ? "#77797E" : "#A0A0A0") : dark ? wxColour("#E8E8E8") : wxColour("#262626"));
        wxString     text = wxControl::Ellipsize(m_label, gdc, wxELLIPSIZE_END, rc.width - FromDIP(6));
        const wxSize ts   = gdc.GetTextExtent(text);
        const wxBitmap& bmp = m_icon_bmp;
        const wxSize    bs  = ScalableBitmap::GetBmpSize(bmp);
        const int       top = (rc.height - bs.y - FromDIP(4) - ts.y) / 2;
        gdc.DrawBitmap(IsEnabled() ? bmp : bmp.ConvertToDisabled(), (rc.width - bs.x) / 2, top, true);
        gdc.DrawText(text, (rc.width - ts.x) / 2, top + bs.y + FromDIP(4));
    }

    wxString                        m_label;
    std::string                     m_icon_name;
    int                             m_icon_px;
    wxImage                         m_picture;
    wxBitmap                        m_icon_bmp;
    std::function<void(NexusTile*)> m_on_click;
    bool                            m_hover{false};
};

}} // namespace Slic3r::GUI
