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

#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/stopwatch.h>
#include <wx/timer.h>
#include <wx/weakref.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <map>
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
// child windows. A card covers the union of its shown windows, inflated by `pad_dip`. A negative `radius_dip`
// rounds each card into a pill (half its height). Call from the parent's wxEVT_PAINT handler.
inline void nexus_paint_cards(wxWindow* parent, wxPaintDC& dc, const std::vector<std::vector<wxWindow*>>& cards,
                              int radius_dip = NEXUS_CARD_RADIUS, int pad_dip = NEXUS_CARD_PAD)
{
    wxGCDC    gdc(dc);
    for (const auto& card : cards) {
        wxRect rc;
        for (const wxWindow* w : card)
            if (w && w->IsShown())
                rc = rc.IsEmpty() ? w->GetRect() : rc.Union(w->GetRect());
        if (rc.IsEmpty())
            continue;
        rc.Inflate(parent->FromDIP(pad_dip));
        const int radius = radius_dip < 0 ? std::min(rc.width, rc.height) / 2 : parent->FromDIP(radius_dip);

        nexus_paint_shadow(parent, gdc, rc, radius);
        gdc.SetPen(wxPen(StateColor::darkModeColorFor(wxColour(NEXUS_CARD_LINE)), 1));
        gdc.SetBrush(wxBrush(nexus_card_colour()));
        gdc.DrawRoundedRectangle(rc, radius);
    }
}

// A card title's text in a sizer that sets it optically centred on the title row. Centred by its text box,
// macOS draws the glyphs a pixel high, which shows in a folded card, where the title row is the whole card.
inline wxSizer* nexus_card_title_text(wxWindow* label)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
#ifdef __WXOSX__
    sizer->Add(label, 0, wxEXPAND | wxTOP, label->FromDIP(1));
#else
    sizer->Add(label, 0, wxEXPAND);
#endif
    return sizer;
}

// Folds or unfolds the content window of a sidebar card with a short height animation (Web-Slicer card). The
// content keeps its layout while its window grows or shrinks, so it slides in and out instead of squeezing.
// `relayout` lays out the card's parent; it runs on every step. The content must sit in a vertical box sizer
// without a stretch proportion, where its minimum height sets its height.
class NexusFoldAnimation : public wxTimer
{
public:
    static void run(wxWindow* content, bool show, std::function<void()> relayout)
    {
        auto& running = active();
        if (auto it = running.find(content); it != running.end())
            it->second->finish();
        if (show == content->IsShown())
            return;
        new NexusFoldAnimation(content, show, std::move(relayout));
    }

private:
    static constexpr int DURATION_MS = 200;

    NexusFoldAnimation(wxWindow* content, bool show, std::function<void()> relayout)
        : m_content(content), m_show(show), m_relayout(std::move(relayout)), m_min(content->GetMinSize())
    {
        active()[content] = this;
        m_full = content->GetSizer() != nullptr ? content->GetSizer()->GetMinSize().y : content->GetBestSize().y;
        // Laid out at the full height once, then frozen while the window resizes.
        content->SetSize(content->GetSize().x, m_full);
        content->Layout();
        content->Bind(wxEVT_SIZE, &NexusFoldAnimation::on_size, this);
        if (m_show) {
            content->SetMinSize(wxSize(m_min.x, 0));
            content->Show();
        }
        m_clock.Start();
        step();
        Start(15);
    }

    static std::map<wxWindow*, NexusFoldAnimation*>& active()
    {
        static std::map<wxWindow*, NexusFoldAnimation*> animations;
        return animations;
    }

    void on_size(wxSizeEvent&) { m_content->Refresh(); } // no layout: the content slides

    void Notify() override { step(); }

    void step()
    {
        if (m_finished)
            return;
        if (!m_content) {
            finish();
            return;
        }
        const double t = std::min(1.0, m_clock.Time() / double(DURATION_MS));
        if (t >= 1.0) {
            finish();
            return;
        }
        const double eased = 1.0 - std::pow(1.0 - t, 3.0);
        const int    h     = int(std::lround(m_full * (m_show ? eased : 1.0 - eased)));
        m_content->SetMinSize(wxSize(m_min.x, h));
        m_relayout();
    }

    void finish()
    {
        if (m_finished)
            return;
        m_finished = true;
        Stop();
        active().erase(m_content_key);
        if (m_content) {
            m_content->Unbind(wxEVT_SIZE, &NexusFoldAnimation::on_size, this);
            m_content->SetMinSize(m_min);
            if (!m_show)
                m_content->Hide();
            m_content->Layout();
            m_relayout();
        }
        m_content = nullptr;
        wxTheApp->CallAfter([this] { delete this; });
    }

    wxWeakRef<wxWindow>   m_content;
    wxWindow*             m_content_key{m_content.get()};
    bool                  m_show;
    std::function<void()> m_relayout;
    wxSize                m_min;
    int                   m_full{0};
    bool                  m_finished{false};
    wxStopWatch           m_clock;
};

}} // namespace Slic3r::GUI
