#pragma once

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/window.h>

#include <wx/string.h>

#include <vector>

class wxPanel;
class wxGridSizer;
class ScalableButton;
class StaticBox;
class Label;

namespace Slic3r { namespace GUI {

class NexusTile;

// Nexus: the "Model" card of the Prepare sidebar, after the printer and filament cards (Web-Slicer model card). It gathers what the
// Home page offered — new and open projects, the recent projects as pictured tiles and the wiki — with the
// handy test models as pictured tiles. The Home page itself stays as a page without a tab, opened by the account
// button in the top right corner of the workspace (it has the Orca Cloud account, lists all recent projects and
// can remove them).
class NexusModelCard
{
public:
    // Builds the card's windows on `parent` and adds them to `sizer` with the sidebar's card margin on the sides
    // and `card_spacing` below, to the next card.
    NexusModelCard(wxWindow* parent, wxBoxSizer* sizer, int card_margin, int card_spacing);

    // The windows the card outline is painted around.
    std::vector<wxWindow*> card_windows() const;

    void msw_rescale();
    void sys_color_changed();

private:
    void build_content(wxWindow* parent);
    void toggle();
    void update_recent();
    void update_gallery_height();

    wxWindow*          m_parent;
    wxBoxSizer*        m_sizer;
    StaticBox*         m_title{nullptr};
    Label*             m_title_text{nullptr};
    ScalableButton*    m_icon{nullptr};
    ScalableButton*    m_help_btn{nullptr};
    wxPanel*           m_content{nullptr};
    std::vector<NexusTile*> m_tiles;
    wxScrolledWindow*       m_gallery{nullptr};      // recent projects, then the test models
    wxGridSizer*            m_gallery_grid{nullptr};
    Label*                  m_more{nullptr};         // Show more / Show less
    bool                    m_gallery_open{false};   // all gallery rows shown
    std::vector<NexusTile*> m_recent_tiles;
    std::vector<wxString>   m_recent_files;
    bool                    m_recent_thumbs{false};
};

}} // namespace Slic3r::GUI
