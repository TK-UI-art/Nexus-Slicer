#include "NexusModelCard.hpp"

#include "GUI_App.hpp"
#include "GUI_Factories.hpp"
#include "GUI_ObjectList.hpp"
#include "I18N.hpp"
#include "MainFrame.hpp"
#include "NexusCard.hpp"
#include "NexusTheme.hpp"
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
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/utils.h>
#include <wx/wrapsizer.h>

#include <functional>
#include <string>

namespace Slic3r { namespace GUI {

// A tile of the model card (Web-Slicer tile): a rounded grey field with an icon above its label, or a
// text-only pill when it has no icon.
class NexusTile : public wxWindow
{
public:
    NexusTile(wxWindow* parent, const wxString& label, const std::string& icon, std::function<void(NexusTile*)> on_click)
        : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE)
        , m_label(label)
        , m_icon_name(icon)
        , m_on_click(std::move(on_click))
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetFont(m_icon_name.empty() ? Label::Body_12 : Label::Body_11);
        SetToolTip(label);
        msw_rescale();
        Bind(wxEVT_PAINT, &NexusTile::on_paint, this);
        Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent&) { m_hover = true; Refresh(); });
        Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent&) { m_hover = false; Refresh(); });
        Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) {
            if (m_on_click)
                m_on_click(this);
        });
        SetCursor(wxCursor(wxCURSOR_HAND));
    }

    void msw_rescale()
    {
        if (!m_icon_name.empty())
            m_icon = ScalableBitmap(this, m_icon_name, ICON_PX);
        wxClientDC dc(this);
        dc.SetFont(GetFont());
        const wxSize text = dc.GetTextExtent(m_label);
        const wxSize best = m_icon_name.empty() ? wxSize(text.x + FromDIP(24), FromDIP(28))
                                                : wxSize(FromDIP(64), FromDIP(ICON_PX + 30));
        SetMinSize(best);
        Refresh();
    }

private:
    static constexpr int ICON_PX = 34;

    void on_paint(wxPaintEvent&)
    {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(wxBrush(nexus_card_colour()));
        dc.Clear();
        wxGCDC gdc(dc);

        const bool   dark = StateColor::darkModeColorFor(*wxWHITE) != *wxWHITE;
        const wxRect rc   = GetClientRect();
        const wxColour fill = m_hover ? wxColour(dark ? NEXUS_ACCENT_TINT25_DARK : NEXUS_ACCENT_TINT25)
                                      : wxColour(dark ? "#3A3C41" : "#E4E4E4");
        gdc.SetPen(*wxTRANSPARENT_PEN);
        gdc.SetBrush(wxBrush(fill));
        gdc.DrawRoundedRectangle(rc, m_icon_name.empty() ? rc.height / 2.0 : FromDIP(10));

        gdc.SetFont(GetFont());
        gdc.SetTextForeground(dark ? wxColour("#E8E8E8") : wxColour("#262626"));
        wxString     text = wxControl::Ellipsize(m_label, gdc, wxELLIPSIZE_END, rc.width - FromDIP(6));
        const wxSize ts   = gdc.GetTextExtent(text);
        if (m_icon_name.empty()) {
            gdc.DrawText(text, (rc.width - ts.x) / 2, (rc.height - ts.y) / 2);
            return;
        }
        const wxBitmap& bmp = m_icon.bmp();
        const wxSize    bs  = ScalableBitmap::GetBmpSize(bmp);
        const int       top = (rc.height - bs.y - FromDIP(4) - ts.y) / 2;
        gdc.DrawBitmap(bmp, (rc.width - bs.x) / 2, top, true);
        gdc.DrawText(text, (rc.width - ts.x) / 2, top + bs.y + FromDIP(4));
    }

    wxString                        m_label;
    std::string                     m_icon_name;
    ScalableBitmap                  m_icon;
    std::function<void(NexusTile*)> m_on_click;
    bool                            m_hover{false};
};

static Plater* plater() { return wxGetApp().plater(); }

NexusModelCard::NexusModelCard(wxWindow* parent, wxBoxSizer* sizer, int card_margin) : m_parent(parent), m_sizer(sizer)
{
    m_title = new StaticBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE);
    m_title->SetBackgroundColor(wxColour(NEXUS_CARD));
    m_title->SetCornerRadius(0);

    m_icon = new ScalableButton(m_title, wxID_ANY, "nexus_card_model", wxEmptyString, wxDefaultSize, wxDefaultPosition,
                                wxBU_EXACTFIT | wxNO_BORDER, false, NEXUS_CARD_ICON);
    m_title_text = new Label(m_title, _L("Model"), LB_PROPAGATE_MOUSE_EVENT | wxST_ELLIPSIZE_END);
    m_title_text->SetFont(Label::Body_16);

    // The Orca Cloud and Bambu Cloud accounts, OrcaCloud and the full recent list live on the Home page.
    m_account_btn = new ScalableButton(m_title, wxID_ANY, "nexus_account", wxEmptyString, wxDefaultSize, wxDefaultPosition,
                                       wxBU_EXACTFIT | wxNO_BORDER, false, 18);
    m_account_btn->SetToolTip(_L("Account, cloud and recent projects"));
    m_account_btn->Bind(wxEVT_BUTTON, [](wxCommandEvent&) { wxGetApp().mainframe->select_tab(TAB_ID_HOME); });

    m_help_btn = new ScalableButton(m_title, wxID_ANY, "help", wxEmptyString, wxDefaultSize, wxDefaultPosition,
                                    wxBU_EXACTFIT | wxNO_BORDER, false, 18);
    m_help_btn->SetToolTip(_L("Wiki"));
    m_help_btn->Bind(wxEVT_BUTTON, [](wxCommandEvent&) { wxLaunchDefaultBrowser("https://orcaslicer.com/wiki", wxBROWSER_NEW_WINDOW); });

    auto*     title = new wxBoxSizer(wxHORIZONTAL);
    title->Add(m_icon, 0, wxALIGN_CENTER | wxLEFT, m_parent->FromDIP(NEXUS_CARD_ICON_INSET));
    title->AddSpacer(m_parent->FromDIP(SidebarProps::ElementSpacing()));
    title->Add(m_title_text, 1, wxALIGN_CENTER | wxRIGHT, m_parent->FromDIP(SidebarProps::WideSpacing()));
    title->Add(m_account_btn, 0, wxALIGN_CENTER | wxRIGHT, m_parent->FromDIP(SidebarProps::WideSpacing()));
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

    sizer->AddSpacer(card_margin);
    sizer->Add(m_title, 0, wxEXPAND | wxLEFT | wxRIGHT, card_margin);
    sizer->Add(m_content, 0, wxEXPAND | wxLEFT | wxRIGHT, card_margin);
    sizer->AddSpacer(m_parent->FromDIP(NEXUS_CARD_GAP + 2 * NEXUS_CARD_PAD) - card_margin);
}

void NexusModelCard::build_content(wxWindow* parent)
{
    auto* content = new wxBoxSizer(wxVERTICAL);
    const int gap = parent->FromDIP(6);

    auto add_heading = [&](const wxString& text) {
        auto* label = new Label(parent, Label::Body_12, text);
        label->SetForegroundColour(StateColor::darkModeColorFor(wxColour("#6B6B6B")));
        content->Add(label, 0, wxLEFT | wxTOP | wxBOTTOM, gap);
    };
    auto add_tile = [&](wxSizer* row, const wxString& label, const std::string& icon, std::function<void(NexusTile*)> on_click) {
        auto* tile = new NexusTile(parent, label, icon, std::move(on_click));
        m_tiles.push_back(tile);
        row->Add(tile, 1, wxEXPAND);
        return tile;
    };

    // Projects
    auto* projects = new wxGridSizer(4, gap, gap);
    add_tile(projects, _L("New Project"), "nexus_tile_new", [](NexusTile*) { plater()->new_project(); });
    add_tile(projects, _L("Open Project"), "nexus_tile_open", [](NexusTile*) { plater()->load_project(); });
    add_tile(projects, _L("Import Model"), "nexus_tile_import", [](NexusTile*) {
#ifdef __APPLE__
        plater()->add_model();
#else
        plater()->add_file();
#endif
    });
    add_tile(projects, _L("Recent"), "nexus_tile_recent", [this](NexusTile* tile) { show_recent_menu(tile); });
    content->Add(projects, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, gap);

    // Built-in shapes, added as objects at their default size (Add Primitive in the plate's context menu).
    add_heading(_L("Shapes"));
    auto* shapes = new wxGridSizer(3, gap, gap);
    const std::pair<const char*, const char*> shape_list[] = {
        {L("Cube"), "nexus_shape_cube"}, {L("Cylinder"), "nexus_shape_cylinder"}, {L("Sphere"), "nexus_shape_sphere"},
        {L("Cone"), "nexus_shape_cone"}, {L("Disc"), "nexus_shape_disc"},         {L("Torus"), "nexus_shape_torus"}};
    for (const auto& [name, icon] : shape_list) {
        const std::string type_name = name;
        add_tile(shapes, _(name), icon, [type_name](NexusTile*) {
            plater()->deselect_all();
            wxGetApp().obj_list()->load_shape_object(type_name, false);
        });
    }
    content->Add(shapes, 0, wxEXPAND | wxLEFT | wxRIGHT, gap);

    // Handy test models (Add Handy models in the plate's context menu).
    add_heading(_L("Test models"));
    auto* models = new wxWrapSizer(wxHORIZONTAL);
    const auto& handy = MenuFactory::handy_models();
    for (size_t i = 0; i < handy.size(); ++i) {
        auto* chip = new NexusTile(parent, _(handy[i].label), "", [i](NexusTile*) { MenuFactory::load_handy_model(i); });
        m_tiles.push_back(chip);
        models->Add(chip, 0, wxRIGHT | wxBOTTOM, gap);
    }
    content->Add(models, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, gap);

    parent->SetSizer(content);
}

void NexusModelCard::show_recent_menu(wxWindow* anchor)
{
    MainFrame* frame = wxGetApp().mainframe;
    wxMenu     menu;
    const auto&    history = frame->recent_projects();
    for (size_t i = 0; i < history.GetCount() && i < 12; ++i) {
        const wxString path = history.GetHistoryFile(i);
        menu.Append(wxID_FILE1 + int(i), wxFileName(path).GetFullName(), path);
    }
    if (history.GetCount() == 0) {
        menu.Append(wxID_ANY, _L("No recent projects"))->Enable(false);
    }
    menu.AppendSeparator();
    // The Home page lists them with previews and can remove them from the list.
    menu.Append(wxID_HIGHEST + 1, _L("Show all recent projects"));

    menu.Bind(wxEVT_MENU, [frame](wxCommandEvent& evt) {
        if (evt.GetId() == wxID_HIGHEST + 1) {
            frame->select_tab(TAB_ID_HOME);
            return;
        }
        const size_t file_id = size_t(evt.GetId() - wxID_FILE1);
        frame->open_recent_project(file_id, frame->recent_projects().GetHistoryFile(file_id));
    });
    anchor->PopupMenu(&menu, 0, anchor->GetSize().y);
}

void NexusModelCard::toggle()
{
    const bool show = !m_content->IsShown();
    m_content->Show(show);
    wxGetApp().app_config->set_bool("nexus_model_card_folded", !show);
    m_sizer->Layout();
    m_parent->Refresh();
}

std::vector<wxWindow*> NexusModelCard::card_windows() const { return {m_title, m_content}; }

void NexusModelCard::msw_rescale()
{
    m_title->GetSizer()->SetMinSize(-1, m_parent->FromDIP(NEXUS_CARD_TITLE_H));
    for (ScalableButton* btn : {m_icon, m_account_btn, m_help_btn})
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
