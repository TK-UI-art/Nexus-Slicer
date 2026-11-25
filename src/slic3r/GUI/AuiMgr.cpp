#include "AuiMgr.hpp"

#include "GUI_App.hpp"
#include "NexusTheme.hpp"
#include "Widgets/StateColor.hpp"
#ifdef __WXGTK__
#include "LinuxDisplayBackend.hpp"
#endif

#include <wx/aui/dockart.h>
#include <wx/aui/floatpane.h>
#include <wx/aui/framemanager.h>
#include <wx/colour.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/settings.h>

#include <algorithm>

namespace Slic3r { namespace GUI {

namespace {

// TODO: listen on dark ui change
class FloatFrame : public wxAuiFloatingFrame
{
public:
    FloatFrame(wxWindow* parent, wxAuiManager* ownerMgr, const wxAuiPaneInfo& pane) : wxAuiFloatingFrame(parent, ownerMgr, pane)
    {
        wxGetApp().UpdateFrameDarkUI(this);
    }
};

} // namespace

void AuiMgr::init(wxWindow* window)
{
    SetManagedWindow(window);
    SetDockSizeConstraint(1, 1);
#ifdef __WXGTK__
    // Floating and its docking hints need global pointer positions and window moves, which Wayland does not provide.
    if (is_running_on_wayland())
        SetFlags(GetFlags() & ~wxAUI_MGR_ALLOW_FLOATING);
#endif
    GetArtProvider()->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, 18);
    GetArtProvider()->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_NONE);
    apply_color_mode();
}

void AuiMgr::apply_color_mode()
{
    // Nexus: caption strip, sash and border take the sidebar background, which is also the 3D view
    // background, so the sidebar cards appear to float over the scene (Web-Slicer layout).
    const bool     is_dark    = wxGetApp().dark_mode();
    const wxColour sash_color = StateColor::darkModeColorFor(wxColour(NEXUS_SIDEBAR_BG));
    GetArtProvider()->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, sash_color);
    GetArtProvider()->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, is_dark ? wxColour(0xB2, 0xB3, 0xB5) : wxColour(0x8A, 0x8A, 0x8A));
    GetArtProvider()->SetColour(wxAUI_DOCKART_SASH_COLOUR, sash_color);
    GetArtProvider()->SetColour(wxAUI_DOCKART_BORDER_COLOUR, sash_color);
}

void AuiMgr::track_docked_size(wxWindow* window)
{
    window->Bind(wxEVT_IDLE, [this, window](wxIdleEvent& evt) {
        wxAuiPaneInfo& pane = GetPane(window);
        if (pane.IsOk() && pane.IsShown() && pane.IsDocked() && pane.rect.GetWidth() > 0 && pane.rect.GetHeight() > 0) {
            const bool horizontal = pane.dock_direction == wxAUI_DOCK_TOP || pane.dock_direction == wxAUI_DOCK_BOTTOM;
            pane.BestSize(horizontal ? pane.best_size.GetWidth() : pane.rect.GetWidth(),
                          horizontal ? pane.rect.GetHeight() : pane.best_size.GetHeight());
        }
        evt.Skip();
    });
}

wxAuiPaneInfo AuiMgr::sidebar_pane_info()
{
    return wxAuiPaneInfo()
        .Name("sidebar")
        .Left()
        .CloseButton(false)
        .TopDockable(false)
        .BottomDockable(false)
        .MinSize(wxSize(sidebar_min_width(), -1))
        .BestSize(wxSize(sidebar_min_width(), 90 * wxGetApp().em_unit()));
}

int AuiMgr::sidebar_min_width()
{
    // Nexus: a settings row takes 34 em (2 em title inset, 20 em label column, 12 em field), and the cards the
    // settings sit in add their insets and the undo buttons (about 64 DIP) and the page may show a vertical
    // scroll bar. At OrcaSlicer's 39 em the fields stuck out past the right edge of their card.
    const int em = wxGetApp().em_unit();
    return std::max(39 * em, 41 * em + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X));
}

wxAuiFloatingFrame* AuiMgr::CreateFloatingFrame(wxWindow* parent, const wxAuiPaneInfo& pane)
{
    return new FloatFrame(parent, this, pane);
}

}} // namespace Slic3r::GUI
