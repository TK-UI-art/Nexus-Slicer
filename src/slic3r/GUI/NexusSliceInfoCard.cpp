#include "NexusSliceInfoCard.hpp"

#include "GUI_App.hpp"
#include "I18N.hpp"
#include "NexusCard.hpp"
#include "NexusTheme.hpp"
#include "PartPlate.hpp"
#include "Plater.hpp"
#include "Widgets/Label.hpp"
#include "Widgets/StateColor.hpp"
#include "Widgets/StaticBox.hpp"
#include "wxExtensions.hpp"
#include "libslic3r/AppConfig.hpp"
#include "libslic3r/Color.hpp"
#include "libslic3r/GCode/GCodeProcessor.hpp"
#include "libslic3r/Print.hpp"
#include "libslic3r/Utils.hpp"

#include <wx/colour.h>
#include <wx/event.h>
#include <wx/panel.h>

#include <cmath>
#include <string>
#include <vector>

namespace Slic3r { namespace GUI {

static const char* FOLDED_KEY = "nexus_slice_info_card_folded";

NexusSliceInfoCard::NexusSliceInfoCard(wxWindow* parent, wxBoxSizer* sizer, int card_margin, int card_spacing)
    : m_parent(parent), m_sizer(sizer)
{
    m_title = new StaticBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE);
    m_title->SetBackgroundColor(wxColour(NEXUS_CARD));
    m_title->SetCornerRadius(0);

    m_icon = new ScalableButton(m_title, wxID_ANY, "nexus_card_slice", wxEmptyString, wxDefaultSize, wxDefaultPosition,
                                wxBU_EXACTFIT | wxNO_BORDER, false, NEXUS_CARD_ICON);
    // TRN Title of the sidebar card with the statistics of the sliced plate (filament use, changes, cost, time).
    m_title_text = new Label(m_title, _L("Slice info"), LB_PROPAGATE_MOUSE_EVENT | wxST_ELLIPSIZE_END);
    m_title_text->SetFont(Label::Body_16);

    auto* title = new wxBoxSizer(wxHORIZONTAL);
    title->Add(m_icon, 0, wxALIGN_CENTER | wxLEFT, parent->FromDIP(NEXUS_CARD_ICON_INSET));
    title->AddSpacer(parent->FromDIP(SidebarProps::ElementSpacing()));
    title->Add(nexus_card_title_text(m_title_text), 1, wxALIGN_CENTER | wxRIGHT, parent->FromDIP(SidebarProps::TitlebarMargin()));
    title->SetMinSize(-1, parent->FromDIP(NEXUS_CARD_TITLE_H));
    m_title->SetSizer(title);
    m_title->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent&) { toggle(); });
    m_title->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent&) { toggle(); });

    m_content = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_content->SetBackgroundColour(nexus_card_colour());

    m_card_sizer = new wxBoxSizer(wxVERTICAL);
    m_card_sizer->AddSpacer(card_spacing);
    m_card_sizer->Add(m_title, 0, wxEXPAND | wxLEFT | wxRIGHT, card_margin);
    m_card_sizer->Add(m_content, 0, wxEXPAND | wxLEFT | wxRIGHT, card_margin);
    sizer->Add(m_card_sizer, 0, wxEXPAND);
    sizer->Show(m_card_sizer, false); // until the plate is sliced

    // The parent is always shown, so it gets the idle events the hidden card would not.
    parent->Bind(wxEVT_IDLE, [this](wxIdleEvent& e) {
        e.Skip();
        update();
    });
}

std::vector<wxWindow*> NexusSliceInfoCard::card_windows() const { return {m_title, m_content}; }

// The current plate's slice result, or null while the plate has none.
static const GCodeProcessorResult* current_slice_result(Plater& plater)
{
    PartPlate* plate = plater.get_partplate_list().get_curr_plate();
    if (plate == nullptr || !plate->is_slice_result_valid())
        return nullptr;
    const GCodeProcessorResult* result = plate->get_slice_result();
    if (result == nullptr || result->print_statistics.modes[size_t(PrintEstimatedStatistics::ETimeMode::Normal)].time <= 0.f)
        return nullptr;
    return result;
}

void NexusSliceInfoCard::update()
{
    Plater* plater = wxGetApp().plater();
    if (plater == nullptr)
        return;
    const GCodeProcessorResult* result = current_slice_result(*plater);

    // What the card would show: the plate, its result and the settings the numbers depend on.
    wxString shown;
    if (result != nullptr) {
        const PrintEstimatedStatistics& stats = result->print_statistics;
        shown << plater->get_partplate_list().get_curr_plate_index() << '|'
              << stats.modes[size_t(PrintEstimatedStatistics::ETimeMode::Normal)].time << '|' << stats.total_filament_changes << '|'
              << stats.total_extruder_changes << '|' << plater->get_partplate_list().get_current_fff_print().print_statistics().total_cost
              << '|' << wxGetApp().app_config->get("use_inches");
        for (const auto& [extruder, volume] : stats.total_volumes_per_extruder)
            shown << '|' << unsigned(extruder) << ':' << volume;
        for (const ColorRGBA& colour : plater->get_extruders_colors())
            shown << '|' << encode_color(colour);
    }
    if (shown == m_shown)
        return;
    m_shown = shown;

    m_sizer->Show(m_card_sizer, result != nullptr);
    if (result != nullptr) {
        rebuild();
        m_content->Show(!wxGetApp().app_config->get_bool(FOLDED_KEY));
    }
    m_sizer->Layout();
    m_parent->Refresh();
}

void NexusSliceInfoCard::rebuild()
{
    Plater&                         plater = *wxGetApp().plater();
    const GCodeProcessorResult&     result = *current_slice_result(plater);
    const PrintEstimatedStatistics& stats  = result.print_statistics;
    const PrintEstimatedStatistics::Mode& time_mode = stats.modes[size_t(PrintEstimatedStatistics::ETimeMode::Normal)];
    const Print&                    print  = plater.get_partplate_list().get_current_fff_print();
    const PrintConfig&              config = print.config();
    const bool imperial = wxGetApp().app_config->get("use_inches") == "1";

    m_content->DestroyChildren();
    const int gap  = m_parent->FromDIP(4);
    const int pad  = m_parent->FromDIP(SidebarProps::ContentMargin());
    const wxColour ink  = StateColor::darkModeColorFor(wxColour("#262626"));
    const wxColour mute = StateColor::darkModeColorFor(wxColour("#6B6B6B"));

    auto* content = new wxBoxSizer(wxVERTICAL);
    auto heading = [&](const wxString& text) {
        auto* label = new Label(m_content, Label::Head_13, text);
        label->SetForegroundColour(ink);
        content->Add(label, 0, wxLEFT | wxRIGHT | wxTOP, pad);
    };
    auto text = [&](const wxString& value, bool muted) {
        auto* label = new Label(m_content, Label::Body_12, value);
        label->SetForegroundColour(muted ? mute : ink);
        return label;
    };

    // The filament each slot uses, model, supports, prime tower and flushing together.
    heading(_L("Filament"));
    auto* filaments = new wxFlexGridSizer(4, gap, 3 * gap);
    filaments->AddGrowableCol(2);
    const std::vector<ColorRGBA> colours = plater.get_extruders_colors();
    for (const auto& [extruder, volume] : stats.total_volumes_per_extruder) {
        if (volume <= 0.)
            continue;
        const double diameter = config.filament_diameter.get_at(extruder);
        const double length   = diameter > 0. ? volume / (PI * 0.25 * diameter * diameter) : 0.; // mm
        const double weight   = volume * config.filament_density.get_at(extruder) * 0.001;     // g

        auto* swatch = new wxPanel(m_content, wxID_ANY, wxDefaultPosition, wxSize(m_parent->FromDIP(12), m_parent->FromDIP(12)));
        if (extruder < colours.size()) {
            const ColorRGBA& c = colours[extruder];
            swatch->SetBackgroundColour(wxColour(int(std::lround(c.r() * 255)), int(std::lround(c.g() * 255)), int(std::lround(c.b() * 255))));
        }
        filaments->Add(swatch, 0, wxALIGN_CENTER_VERTICAL);
        filaments->Add(text(wxString::Format("%d", int(extruder) + 1), false), 0, wxALIGN_CENTER_VERTICAL);
        filaments->Add(text(imperial ? wxString::Format("%.2f in", length / 25.4) : wxString::Format("%.2f m", length / 1000.), false), 0,
                       wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
        filaments->Add(text(wxString::Format("%.2f g", weight), false), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    }
    content->Add(filaments, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, pad);

    auto add_lines = [&](const std::vector<std::pair<wxString, wxString>>& lines) {
        auto* grid = new wxFlexGridSizer(2, gap, 3 * gap);
        grid->AddGrowableCol(1);
        for (const auto& [name, value] : lines) {
            grid->Add(text(name + ":", true), 0, wxALIGN_CENTER_VERTICAL);
            grid->Add(text(value, false), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
        }
        content->Add(grid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, pad);
    };
    add_lines({{_L("Filament change times"), wxString::Format("%u", stats.total_filament_changes)},
               {_L("Tool changes"), wxString::Format("%u", stats.total_extruder_changes)},
               {_L("Cost"), wxString::Format("%.2f", print.print_statistics().total_cost)}});

    heading(_L("Time Estimation"));
    std::vector<std::pair<wxString, wxString>> times;
    if (time_mode.prepare_time != 0.f)
        times.push_back({_L("Prepare time"), wxString::FromUTF8(short_time(get_time_dhms(time_mode.prepare_time)))});
    times.push_back({_L("Model printing time"), wxString::FromUTF8(short_time(get_time_dhms(float(time_mode.time) - time_mode.prepare_time)))});
    times.push_back({_L("Total time"), wxString::FromUTF8(short_time(get_time_dhms(float(time_mode.time))))});
    add_lines(times);
    content->AddSpacer(pad);

    m_content->SetSizer(content, true);
    nexus_apply_card_colour(m_content);
    m_content->Layout();
}

void NexusSliceInfoCard::toggle()
{
    const bool show = !m_content->IsShown();
    wxGetApp().app_config->set_bool(FOLDED_KEY, !show);
    NexusFoldAnimation::run(m_content, show, [this] {
        m_sizer->Layout();
        m_parent->Refresh();
    });
}

void NexusSliceInfoCard::msw_rescale()
{
    m_title->GetSizer()->SetMinSize(-1, m_parent->FromDIP(NEXUS_CARD_TITLE_H));
    m_icon->msw_rescale();
    m_shown.clear(); // rebuilt at the new scale
}

void NexusSliceInfoCard::sys_color_changed()
{
    m_content->SetBackgroundColour(nexus_card_colour());
    m_icon->msw_rescale();
    m_shown.clear(); // rebuilt in the new colours
}

}} // namespace Slic3r::GUI
