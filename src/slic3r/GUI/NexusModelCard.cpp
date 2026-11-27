#include "NexusModelCard.hpp"

#include "GUI_App.hpp"
#include "GUI_Factories.hpp"
#include "I18N.hpp"
#include "MainFrame.hpp"
#include "NexusCard.hpp"
#include "NexusTheme.hpp"
#include "NexusTile.hpp"
#include "Plater.hpp"
#include "Widgets/Label.hpp"
#include "Widgets/StateColor.hpp"
#include "Widgets/StaticBox.hpp"
#include "libslic3r/AppConfig.hpp"
#include "wxExtensions.hpp"

#include <wx/control.h>
#include <wx/cursor.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/utils.h>

#include <algorithm>
#include <functional>
#include <string>

namespace Slic3r { namespace GUI {

static Plater* plater() { return wxGetApp().plater(); }

static constexpr int TILE_GAP     = 6;  // DIP, between the tiles
static constexpr int GALLERY_ICON = 56; // DIP, pictures of the gallery tiles

NexusModelCard::NexusModelCard(wxWindow* parent, wxBoxSizer* sizer, int card_margin, int card_spacing)
    : m_parent(parent), m_sizer(sizer)
{
    m_title = new StaticBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE);
    m_title->SetBackgroundColor(wxColour(NEXUS_CARD));
    m_title->SetCornerRadius(0);

    m_icon = new ScalableButton(m_title, wxID_ANY, "nexus_card_model", wxEmptyString, wxDefaultSize, wxDefaultPosition,
                                wxBU_EXACTFIT | wxNO_BORDER, false, NEXUS_CARD_ICON);
    m_title_text = new Label(m_title, _L("Model"), LB_PROPAGATE_MOUSE_EVENT | wxST_ELLIPSIZE_END);
    m_title_text->SetFont(Label::Body_16);

    m_help_btn = new ScalableButton(m_title, wxID_ANY, "help", wxEmptyString, wxDefaultSize, wxDefaultPosition,
                                    wxBU_EXACTFIT | wxNO_BORDER, false, 18);
    m_help_btn->SetToolTip(_L("Wiki"));
    m_help_btn->Bind(wxEVT_BUTTON, [](wxCommandEvent&) { wxLaunchDefaultBrowser("https://orcaslicer.com/wiki", wxBROWSER_NEW_WINDOW); });

    auto*     title = new wxBoxSizer(wxHORIZONTAL);
    title->Add(m_icon, 0, wxALIGN_CENTER | wxLEFT, m_parent->FromDIP(NEXUS_CARD_ICON_INSET));
    title->AddSpacer(m_parent->FromDIP(SidebarProps::ElementSpacing()));
    title->Add(m_title_text, 1, wxALIGN_CENTER | wxRIGHT, m_parent->FromDIP(SidebarProps::WideSpacing()));
    title->Add(m_help_btn, 0, wxALIGN_CENTER);
    title->AddSpacer(m_parent->FromDIP(SidebarProps::TitlebarMargin()));
    title->SetMinSize(-1, m_parent->FromDIP(NEXUS_CARD_TITLE_H));
    m_title->SetSizer(title);
    m_title->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent&) { toggle(); });
    m_title->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent&) { toggle(); });

    m_content = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_content->SetBackgroundColour(nexus_card_colour());
    build_content(m_content);
    if (wxGetApp().app_config->get_bool("nexus_model_card_folded"))
        m_content->Hide();

    sizer->Add(m_title, 0, wxEXPAND | wxLEFT | wxRIGHT, card_margin);
    sizer->Add(m_content, 0, wxEXPAND | wxLEFT | wxRIGHT, card_margin);
    sizer->AddSpacer(card_spacing);
}

void NexusModelCard::build_content(wxWindow* parent)
{
    auto* content = new wxBoxSizer(wxVERTICAL);
    const int gap = parent->FromDIP(TILE_GAP);

    auto add_tile = [&](wxWindow* tile_parent, wxSizer* row, const wxString& label, const std::string& icon, int icon_px,
                        std::function<void(NexusTile*)> on_click) {
        auto* tile = new NexusTile(tile_parent, label, icon, icon_px, std::move(on_click));
        m_tiles.push_back(tile);
        row->Add(tile, 1, wxEXPAND);
        return tile;
    };

    // Projects. One Open tile for projects and models: Plater::add_file() asks for a 3mf whether to open the
    // project or import its geometry, and imports the other formats into the current project.
    auto* projects = new wxGridSizer(2, gap, gap);
    add_tile(parent, projects, _L("New Project"), "nexus_tile_new", 34, [](NexusTile*) { plater()->new_project(); });
    add_tile(parent, projects, _L("Open"), "nexus_tile_open", 34, [](NexusTile*) { plater()->add_file(); })
        ->SetToolTip(_L("Open a project or import models"));
    content->Add(projects, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, gap);

    // A gallery of the recent projects, pictured by their thumbnails and newest first (update_recent()), then the
    // handy test models (Add Handy models in the plate's context menu). It shows two rows and scrolls; Show more
    // opens it to all tiles at once. The Home page (the account button in the workspace's top right corner) lists
    // all recent projects and can remove them; the built-in shapes stay in the plate's context menu (Add Primitive).
    m_gallery = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_gallery->SetBackgroundColour(nexus_card_colour());
    m_gallery->SetScrollRate(0, parent->FromDIP(10));
    m_gallery_grid = new wxGridSizer(3, gap, gap);
    m_gallery->SetSizer(m_gallery_grid);
    const auto& handy = MenuFactory::handy_models();
    for (size_t i = 0; i < handy.size(); ++i)
        add_tile(m_gallery, m_gallery_grid, _(handy[i].label), std::string("nexus_handy_") + handy[i].key, GALLERY_ICON,
                 [i](NexusTile*) { MenuFactory::load_handy_model(i); });
    content->Add(m_gallery, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, gap);

    m_more = new Label(parent, Label::Body_11, _L("Show more"));
    m_more->SetForegroundColour(StateColor::darkModeColorFor(wxColour(NEXUS_ACCENT_TEXT)));
    m_more->SetCursor(wxCursor(wxCURSOR_HAND));
    m_more->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) {
        m_gallery_open = !m_gallery_open;
        update_gallery_height();
    });
    content->Add(m_more, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, gap);

    parent->SetSizer(content);
    update_recent();
    update_gallery_height(); // update_recent() lays out only a changed list
    parent->Bind(wxEVT_IDLE, [this](wxIdleEvent& e) {
        e.Skip();
        update_recent();
    });
}

void NexusModelCard::update_gallery_height()
{
    static constexpr int CLOSED_ROWS = 2;
    const int gap    = m_parent->FromDIP(TILE_GAP);
    const int row_h  = m_parent->FromDIP(NexusTile::min_height(GALLERY_ICON));
    const int tiles  = int(m_gallery_grid->GetItemCount());
    const int all    = (tiles + 2) / 3;
    const int rows   = m_gallery_open ? all : std::min(all, CLOSED_ROWS);
    m_more->Show(all > CLOSED_ROWS);
    m_more->SetLabel(m_gallery_open ? _L("Show less") : _L("Show more"));
    m_gallery->SetMinSize(wxSize(-1, rows * row_h + std::max(0, rows - 1) * gap));
    m_gallery->FitInside();
    m_content->Layout();
    m_sizer->Layout();
    m_parent->Layout();
    m_parent->Refresh();
}

void NexusModelCard::update_recent()
{
    MainFrame* frame = wxGetApp().mainframe;
    if (frame == nullptr)
        return;
    const auto&           history = frame->recent_projects();
    std::vector<wxString> files;
    for (size_t i = 0; i < history.GetCount(); ++i)
        files.push_back(history.GetHistoryFile(i));
    if (files == m_recent_files && m_recent_thumbs == (history.GetCount() > 0 && !history.GetThumbnail(0).empty()))
        return;
    m_recent_files  = files;
    m_recent_thumbs = history.GetCount() > 0 && !history.GetThumbnail(0).empty(); // thumbnails load after startup

    for (NexusTile* tile : m_recent_tiles) {
        m_tiles.erase(std::remove(m_tiles.begin(), m_tiles.end(), tile), m_tiles.end());
        m_gallery_grid->Detach(tile);
        tile->Destroy();
    }
    m_recent_tiles.clear();

    for (size_t i = 0; i < files.size(); ++i) {
        const std::string& png = history.GetThumbnail(i);
        wxImage            picture;
        if (!png.empty()) {
            wxMemoryInputStream stream(png.data(), png.size());
            picture.LoadFile(stream, wxBITMAP_TYPE_PNG);
        }
        const wxString path = files[i];
        auto on_click = [frame, i, path](NexusTile*) { frame->open_recent_project(i, path); };
        auto* tile = picture.IsOk() ? new NexusTile(m_gallery, wxFileName(path).GetName(), picture, GALLERY_ICON, on_click)
                                    : new NexusTile(m_gallery, wxFileName(path).GetName(), "nexus_tile_open", GALLERY_ICON, on_click);
        tile->SetToolTip(path);
        m_gallery_grid->Insert(i, tile, 1, wxEXPAND);
        m_recent_tiles.push_back(tile);
        m_tiles.push_back(tile);
    }
    update_gallery_height();
}

void NexusModelCard::toggle()
{
    const bool show = !m_content->IsShown();
    wxGetApp().app_config->set_bool("nexus_model_card_folded", !show);
    NexusFoldAnimation::run(m_content, show, [this] {
        m_sizer->Layout();
        m_parent->Refresh();
    });
}

std::vector<wxWindow*> NexusModelCard::card_windows() const { return {m_title, m_content}; }

void NexusModelCard::msw_rescale()
{
    m_title->GetSizer()->SetMinSize(-1, m_parent->FromDIP(NEXUS_CARD_TITLE_H));
    for (ScalableButton* btn : {m_icon, m_help_btn})
        btn->msw_rescale();
    for (NexusTile* tile : m_tiles)
        tile->msw_rescale();
}

void NexusModelCard::sys_color_changed()
{
    m_content->SetBackgroundColour(nexus_card_colour());
    msw_rescale();
    m_content->Refresh();
}

}} // namespace Slic3r::GUI
