#pragma once

#include <wx/sizer.h>
#include <wx/window.h>

#include <vector>

class wxPanel;
class ScalableButton;
class StaticBox;
class Label;

namespace Slic3r { namespace GUI {

class NexusTile;

// Nexus: the "Model" card at the top of the Prepare sidebar (Web-Slicer model card). It gathers what the
// Home page offered — new, open and recent projects, the Orca Cloud account and the wiki — with the handy
// test models as pictured tiles. The Home page itself stays as a page without a tab; the card's account
// button and the recent list's last entry open it.
class NexusModelCard
{
public:
    // Builds the card's windows on `parent` and adds them to `sizer` with the sidebar's card margin.
    NexusModelCard(wxWindow* parent, wxBoxSizer* sizer, int card_margin);

    // The windows the card outline is painted around.
    std::vector<wxWindow*> card_windows() const;

    void msw_rescale();
    void sys_color_changed();

private:
    void build_content(wxWindow* parent);
    void toggle();
    void show_recent_menu(wxWindow* anchor);

    wxWindow*          m_parent;
    wxBoxSizer*        m_sizer;
    StaticBox*         m_title{nullptr};
    Label*             m_title_text{nullptr};
    ScalableButton*    m_icon{nullptr};
    ScalableButton*    m_account_btn{nullptr};
    ScalableButton*    m_help_btn{nullptr};
    wxPanel*           m_content{nullptr};
    std::vector<NexusTile*> m_tiles;
};

}} // namespace Slic3r::GUI
