#pragma once

#include "simulator.h"
#include <wx/wx.h>

class CacheView : public wxPanel {
public:
    Cache* cache = nullptr;
    int lastSet = -1, lastWay = -1;

    CacheView(wxWindow* parent);

private:
    void paint(wxPaintEvent&);
    void drawCell(wxDC& dc, int s, int w, int x, int y, int cw, int ch);
};

class Frame : public wxFrame {
    wxTextCtrl *total, *block, *assoc, *trace, *log;
    wxRadioBox* policy;
    wxStaticText* stats;
    CacheView* view;
    Cache cache;
    std::vector<Trace> entries;
    size_t next = 0;

public:
    Frame();

private:
    wxTextCtrl* field(const wxString& value, wxSizer* s, const wxString& label);
    void button(wxSizer* s, const wxString& label, void (Frame::*fn)(wxCommandEvent&));
    Policy selectedPolicy() const;
    bool configure();
    bool readTrace();
    void accessOne();
    void loadTrace(wxCommandEvent&);
    void runAll(wxCommandEvent&);
    void step(wxCommandEvent&);
    void reset(wxCommandEvent&);
    void addLog(const wxString& s);
    void updateStats();
    static wxString sample();
};
