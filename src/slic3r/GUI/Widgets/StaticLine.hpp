#ifndef slic3r_GUI_StaticLine_hpp_
#define slic3r_GUI_StaticLine_hpp_

#include "../wxExtensions.hpp"
#include "wx/window.h"
#include <wx/string.h>
#include <wx/colour.h>
#include <wx/event.h>

class wxSizer;

class StaticLine : public wxWindow
{
public:
    StaticLine(wxWindow *parent, bool vertical = false, const wxString &label = {}, const wxString &icon = {});

public:
    void SetLabel(const wxString& label) override;

    void SetIcon(const wxString& icon);

    void SetLineColour(wxColour color);
    
    void Rescale();

    // Nexus: the title of a settings group drawn in a frame (NexusFrame.hpp); `sizer` holds the group.
    void     SetFrameSizer(wxSizer* sizer) { m_frame_sizer = sizer; }
    wxSizer* GetFrameSizer() const { return m_frame_sizer; }

private:
    wxSizer*       m_frame_sizer{nullptr};
    wxColour       lineColor;
    bool vertical;
    ScalableBitmap icon;

private:
    void paintEvent(wxPaintEvent& evt);

    void messureSize();

    void render(wxDC &dc);

    DECLARE_EVENT_TABLE()
};

#endif // !slic3r_GUI_StaticLine_hpp_
