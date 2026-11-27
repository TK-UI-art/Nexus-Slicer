#include "Notebook.hpp"
#include "slic3r/GUI/NexusTheme.hpp"

//#ifdef _WIN32

#include "GUI_App.hpp"
#include "wxExtensions.hpp"
#include "Widgets/Button.hpp"
#include "Widgets/StateColor.hpp"
#include "NexusCard.hpp"

//BBS set font size

#include <wx/bookctrl.h>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <wx/brush.h>
#include <wx/button.h>
#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/window.h>

wxDEFINE_EVENT(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED, wxCommandEvent);

// Nexus: the pill around the main tabs (Web-Slicer mode switch), rounded to half its height; its padding in DIP.
static constexpr int TAB_PILL_RADIUS = -1;
static constexpr int TAB_PILL_PAD    = 5;

ButtonsListCtrl::ButtonsListCtrl(wxWindow *parent, wxBoxSizer* side_tools) :
    wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTAB_TRAVERSAL)
    , m_owner(parent)
{
#ifdef __WINDOWS__
    SetDoubleBuffered(true);
#endif //__WINDOWS__

    // Nexus: the tabs float as a white pill on the app background (Web-Slicer mode switch).
    SetBackgroundColour(StateColor::darkModeColorFor(wxColour(NEXUS_SIDEBAR_BG)));

    int em = em_unit(this);// Slic3r::GUI::wxGetApp().em_unit();
    // Nexus: tabs are pills with a small gap between them and around the row
    m_btn_margin = std::lround(0.4 * em);
    m_line_margin = std::lround(0.1 * em);

    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(m_sizer);

    m_buttons_sizer = new wxFlexGridSizer(1, m_btn_margin, m_btn_margin);
    // Nexus: the main tab bar centres its tabs between the left edge and the side tools (Web-Slicer
    // centred mode switch). Both stretch spacers shrink to nothing when the bar runs out of room.
    if (side_tools != NULL)
        m_sizer->AddStretchSpacer(1);
    // Room for the pill's padding and its drop shadow around the tabs.
    m_sizer->Add(m_buttons_sizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(TAB_PILL_PAD + 6));

    if (side_tools != NULL) {
        m_sizer->AddStretchSpacer(1);
        for (size_t idx = 0; idx < side_tools->GetItemCount(); idx++) {
            wxSizerItem* item = side_tools->GetItem(idx);
            wxWindow* item_win = item->GetWindow();
            if (item_win) {
                item_win->Reparent(this);
            }
        }
        m_sizer->Add(side_tools, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, m_btn_margin);
    }

    Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxPaintDC dc(this);
        // macOS leaves an unpainted control transparent, showing the window background behind the bar.
        dc.SetBackground(wxBrush(GetBackgroundColour()));
        dc.Clear();
        Slic3r::GUI::nexus_paint_cards(this, dc, {std::vector<wxWindow*>(m_pageButtons.begin(), m_pageButtons.end())}, TAB_PILL_RADIUS,
                                       TAB_PILL_PAD);
    });
    // The pill is painted from the tabs' positions, which move with the bar's size: paint it anew.
    Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
        e.Skip();
        Refresh();
    });
    Bind(wxEVT_SYS_COLOUR_CHANGED, [this](wxSysColourChangedEvent& e) {
        e.Skip();
        SetBackgroundColour(StateColor::darkModeColorFor(wxColour(NEXUS_SIDEBAR_BG)));
        for (Button* btn : m_pageButtons)
            btn->SetBackgroundColour(Slic3r::GUI::nexus_card_colour());
        Refresh();
    });
}

void ButtonsListCtrl::OnPaint(wxPaintEvent&)
{
    //Slic3r::GUI::wxGetApp().UpdateDarkUI(this);
    const wxSize sz = GetSize();
    wxPaintDC dc(this);

    if (m_selection < 0 || m_selection >= (int)m_pageButtons.size())
        return;

    wxColour selected_btn_bg("#1F8EEA");
    wxColour default_btn_bg(NEXUS_BAR);
    const wxColour& btn_marker_color = Slic3r::GUI::wxGetApp().get_color_hovered_btn_label();

    // highlight selected notebook button

    for (int idx = 0; idx < int(m_pageButtons.size()); idx++) {
        Button* btn = m_pageButtons[idx];

        btn->SetBackgroundColor(idx == m_selection ? selected_btn_bg : default_btn_bg);

        wxPoint pos = btn->GetPosition();
        wxSize size = btn->GetSize();
        const wxColour& clr = idx == m_selection ? btn_marker_color : default_btn_bg;
        dc.SetPen(clr);
        dc.SetBrush(clr);
        dc.DrawRectangle(pos.x, pos.y + size.y, size.x, sz.y - size.y);
    }
    // Draw orange bottom line

    dc.SetPen(btn_marker_color);
    dc.SetBrush(btn_marker_color);
    dc.DrawRectangle(1, sz.y - m_line_margin, sz.x, m_line_margin);
}

void ButtonsListCtrl::UpdateMode()
{
    //m_mode_sizer->SetMode(Slic3r::GUI::wxGetApp().get_mode());
}

void ButtonsListCtrl::Rescale()
{
    //m_mode_sizer->msw_rescale();
    int em = em_unit(this);
    for (Button* btn : m_pageButtons) {
        //BBS
        btn->SetMinSize({(btn->GetLabel().empty() ? 40 : 132) * em / 10, 28 * em / 10});
        btn->SetCornerRadius(btn->FromDIP(NEXUS_TAB_RADIUS));
        btn->Rescale();
    }

    // BBS: no gap
    //m_btn_margin = std::lround(0.3 * em);
    //m_line_margin = std::lround(0.1 * em);
    //m_buttons_sizer->SetVGap(m_btn_margin);
    //m_buttons_sizer->SetHGap(m_btn_margin);

    m_sizer->Layout();
}

void ButtonsListCtrl::SetSelection(int sel)
{
    if (m_selection == sel && sel >= 0 && sel < static_cast<int>(m_pageButtons.size()))
        return;
    // BBS: change button color
    wxColour selected_btn_bg(NEXUS_ACCENT);
    if (m_selection >= 0 && m_selection < static_cast<int>(m_pageButtons.size())) {
        StateColor bg_color = StateColor(
        std::pair{wxColour(NEXUS_SIDEBAR_BG), (int) StateColor::Hovered}, // Nexus: Web-Slicer pill, hover shows the page grey
        std::pair{wxColour(NEXUS_CARD), (int) StateColor::Normal});
        m_pageButtons[m_selection]->SetBackgroundColor(bg_color);
        StateColor text_color = StateColor(
        std::pair{wxColour("#262E30"), (int) StateColor::Normal}
        );
        m_pageButtons[m_selection]->SetTextColor(text_color);
    }

    if (sel < 0 || sel >= static_cast<int>(m_pageButtons.size())) {
        m_selection = -1;
        Refresh();
        return;
    }

    m_selection = sel;

    StateColor bg_color = StateColor(
        std::pair{wxColour(NEXUS_ACCENT), (int) StateColor::Hovered},
        std::pair{wxColour(NEXUS_ACCENT), (int) StateColor::Normal});
    m_pageButtons[m_selection]->SetBackgroundColor(bg_color);

    // Nexus: dark text on the green selected tab (white on the accent is unreadable)
    StateColor text_color = StateColor(
        std::pair{wxColour(NEXUS_ON_ACCENT), (int) StateColor::Normal}
        );
    m_pageButtons[m_selection]->SetTextColor(text_color);
    
    Refresh();
}

bool ButtonsListCtrl::InsertPage(size_t n, const wxString &text, bool bSelect /* = false*/, const std::string &bmp_name /* = ""*/, const wxBitmap &bmp /* = wxNullBitmap */)
{
    Button * btn = new Button(this, text, bmp_name, wxNO_BORDER);
    btn->SetCornerRadius(btn->FromDIP(NEXUS_TAB_RADIUS));

    if (bmp_name.empty() && bmp.IsOk())
        btn->SetIcon(bmp);

    // The label no longer carries a leading space, so widen the icon<->text gap to keep the
    // original spacing between a tab's icon and its caption.
    {
        wxClientDC dc(btn);
        dc.SetFont(btn->GetFont());
        int space_w = 0;
        dc.GetTextExtent(" ", &space_w, nullptr);
        btn->SetIconSpacing(5 + space_w);
    }

    int em = em_unit(this);
    //BBS set size for button
    btn->SetMinSize({(text.empty() ? 40 : 136) * em / 10, 28 * em / 10});

    StateColor bg_color = StateColor(
        std::pair{wxColour(NEXUS_SIDEBAR_BG), (int) StateColor::Hovered}, // Nexus: Web-Slicer pill, hover shows the page grey
        std::pair{wxColour(NEXUS_CARD), (int) StateColor::Normal});

    btn->SetBackgroundColor(bg_color);
    StateColor text_color = StateColor(
        std::pair{wxColour("#262E30"), (int) StateColor::Normal});
    btn->SetTextColor(text_color);
    btn->Bind(wxEVT_BUTTON, [this, btn](wxCommandEvent& event) {
        if (auto it = std::find(m_pageButtons.begin(), m_pageButtons.end(), btn); it != m_pageButtons.end()) {
            auto sel = it - m_pageButtons.begin();
            //do it later
            //SetSelection(sel);
            
            wxCommandEvent evt = wxCommandEvent(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED);
            evt.SetId(sel);
            wxPostEvent(m_owner, evt);
        }
    });
    Slic3r::GUI::wxGetApp().UpdateDarkUI(btn);
    // The corners outside the tab's rounding show the pill, not the bar.
    btn->SetBackgroundColour(Slic3r::GUI::nexus_card_colour());
    m_pageButtons.insert(m_pageButtons.begin() + n, btn);
    m_pageLabels.insert(m_pageLabels.begin() + n, text); // ORCA
    m_pageIcons.insert(m_pageIcons.begin() + n, bmp_name);
    m_buttons_sizer->Insert(sizer_index(n), new wxSizerItem(btn));
    m_buttons_sizer->SetCols(m_buttons_sizer->GetCols() + 1);
    m_sizer->Layout();
    return true;
}

void ButtonsListCtrl::RemovePage(size_t n)
{
    if (n >= m_pageButtons.size())
        return;

    if (m_selection == static_cast<int>(n))
        m_selection = -1;
    else if (m_selection > static_cast<int>(n))
        --m_selection;

    Button* btn = m_pageButtons[n];
    if (m_detached.erase(btn) == 0)
        m_buttons_sizer->Detach(btn);
    m_pageButtons.erase(m_pageButtons.begin() + n);
    m_pageLabels.erase(m_pageLabels.begin() + n); // ORCA
    m_pageIcons.erase(m_pageIcons.begin() + n);
#if __WXOSX__
    RemoveChild(btn);
#else
    btn->Reparent(nullptr);
#endif
    btn->Destroy();
    m_sizer->Layout();
}

bool ButtonsListCtrl::SetPageImage(size_t n, const std::string& bmp_name) const
{
    if (n >= m_pageButtons.size())
        return false;
     
    // BBS
    //return m_pageButtons[n]->SetBitmap_(bmp_name);
    ScalableBitmap bitmap(NULL, bmp_name);
    //m_pageButtons[n]->SetBitmap_(bitmap);
    return true;
}

void ButtonsListCtrl::SetPageText(size_t n, const wxString& strText)
{
    Button* btn = m_pageButtons[n];
    btn->SetLabel(strText);
    if(!strText.empty())  // ORCA
        m_pageLabels[n] = strText;
}

// ORCA
void ButtonsListCtrl::ShowPageButton(size_t n, bool show)
{
    if (n >= m_pageButtons.size())
        return;
    // Taken out of the sizer, not only hidden: a hidden item of the flex grid still shifted the row.
    Button* btn = m_pageButtons[n];
    if (show && m_detached.erase(btn) > 0)
        m_buttons_sizer->Insert(sizer_index(n), btn);
    else if (!show && m_detached.insert(btn).second)
        m_buttons_sizer->Detach(btn);
    btn->Show(show);
    m_sizer->Layout();
    Refresh();
}

size_t ButtonsListCtrl::sizer_index(size_t n) const
{
    size_t idx = 0;
    for (size_t i = 0; i < n && i < m_pageButtons.size(); ++i)
        if (m_detached.count(m_pageButtons[i]) == 0)
            ++idx;
    return idx;
}

void ButtonsListCtrl::SetCompact(size_t n, bool compact)
{
    int em = em_unit(this);
    Button* btn = m_pageButtons[n];
    btn->SetMinSize({(compact ? 40 : 136) * em / 10, 28 * em / 10});
    btn->SetLabel(compact ? "" : m_pageLabels[n]);
}

wxString ButtonsListCtrl::GetPageText(size_t n) const
{
    Button* btn = m_pageButtons[n];
    return btn->GetLabel();
}

// ORCA
wxString ButtonsListCtrl::GetPageLabel(size_t n) const
{
    return n < m_pageLabels.size() ? m_pageLabels[n] : wxString();
}

// ORCA
void ButtonsListCtrl::SetOverflowButton(wxWindow* button)
{
    if (m_overflow_button == button)
        return;

    if (m_overflow_button != nullptr)
        m_sizer->Detach(m_overflow_button);

    m_overflow_button = button;

    if (m_overflow_button != nullptr) {
        // Right after the tab buttons (which may follow a leading spacer), ahead of side_tools
        size_t idx = 0;
        while (idx < m_sizer->GetItemCount() && m_sizer->GetItem(idx)->GetSizer() != m_buttons_sizer)
            ++idx;
        m_sizer->Insert(idx + 1, m_overflow_button, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, m_btn_margin);
    }

    m_sizer->Layout();
}

//#endif // _WIN32

void Notebook::Init()
{
    // We don't need any border as we don't have anything to separate the
    // page contents from.
    SetInternalBorder(0);

    // No effects by default.
    m_showEffect = m_hideEffect = wxSHOW_EFFECT_NONE;

    m_showTimeout = m_hideTimeout = 0;

    m_pageNames.clear();

    /* On Linux, Gstreamer wxMediaCtrl does not seem to get along well with
     * 32-bit X11 visuals (the overlay does not work).  Is this a wxWindows
     * bug?  Is this a Gstreamer bug?  No idea, but it is our problem ... 
     * and anyway, this transparency thing just isn't all that interesting,
     * so we just don't do it on Linux. 
     */
#ifndef __WXGTK__
    SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
#endif
}
