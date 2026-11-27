#pragma once

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/window.h>

#include <vector>

class wxPanel;
class wxFlexGridSizer;
class ScalableButton;
class StaticBox;
class Label;

namespace Slic3r { namespace GUI {

// Nexus: the "Slice info" card below the Process card of the sidebar. Once the current plate is sliced it shows
// what the preview legend showed of the result: the filament each slot uses, the filament and tool changes, the
// cost and the time estimate. It hides while the plate has no valid slice result; the standalone G-code viewer,
// which has no sidebar, keeps them in its legend.
class NexusSliceInfoCard
{
public:
    // Builds the card's windows on `parent` and adds them to `sizer`, after `card_spacing` from the card above,
    // with the sidebar's card margin on the sides.
    NexusSliceInfoCard(wxWindow* parent, wxBoxSizer* sizer, int card_margin, int card_spacing);

    // The windows the card outline is painted around.
    std::vector<wxWindow*> card_windows() const;

    void msw_rescale();
    void sys_color_changed();

private:
    void update();
    void rebuild();
    void toggle();

    wxWindow*        m_parent;
    wxBoxSizer*      m_sizer;
    wxBoxSizer*      m_card_sizer{nullptr}; // the card with the gap above it, shown while there is a result
    StaticBox*       m_title{nullptr};
    Label*           m_title_text{nullptr};
    ScalableButton*  m_icon{nullptr};
    wxPanel*         m_content{nullptr};
    wxString         m_shown; // what the content shows, to rebuild it only when the result changes
};

}} // namespace Slic3r::GUI
