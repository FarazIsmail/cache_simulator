#include "gui.h"
#include <wx/dcbuffer.h>
#include <wx/filedlg.h>
#include <fstream>
#include <sstream>

CacheView::CacheView(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &CacheView::paint, this);
}

void CacheView::paint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(wxColour(245, 247, 250)));
    dc.Clear();
    if (!cache || !cache->ready()) {
        dc.DrawText("Configure the cache first.", 12, 12);
        return;
    }
    int cellW = 118, cellH = 38, gap = 8, left = 64, top = 32;
    dc.SetFont(wxFontInfo(9));
    for (int w = 0; w < cache->cfg.assoc; w++) dc.DrawText(wxString::Format("Way %d", w), left + w * (cellW + gap) + 8, 8);
    for (int s = 0; s < cache->cfg.sets; s++) {
        int y = top + s * (cellH + gap);
        dc.DrawText(wxString::Format("Set %d", s), 8, y + 11);
        for (int w = 0; w < cache->cfg.assoc; w++) drawCell(dc, s, w, left + w * (cellW + gap), y, cellW, cellH);
    }
}

void CacheView::drawCell(wxDC& dc, int s, int w, int x, int y, int cw, int ch) {
    auto& line = cache->set[s][w];
    wxColour fill = s == lastSet && w == lastWay ? wxColour(93, 140, 198) : line.valid ? wxColour(220, 225, 232) : wxColour(250, 251, 253);
    dc.SetBrush(wxBrush(fill));
    dc.SetPen(wxPen(wxColour(128, 139, 153)));
    dc.DrawRectangle(x, y, cw, ch);
    dc.SetTextForeground(line.valid && s == lastSet && w == lastWay ? *wxWHITE : wxColour(30, 36, 44));
    dc.DrawText(line.valid ? wxString::Format("V:1 T:0x%llX", line.tag) : "V:0", x + 8, y + 11);
    dc.SetTextForeground(*wxBLACK);
}

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Cache Simulator", wxDefaultPosition, wxSize(1150, 720)) {
    auto root = new wxBoxSizer(wxHORIZONTAL);
    auto controls = new wxBoxSizer(wxVERTICAL);
    total = field("1024", controls, "Total Cache Size");
    block = field("64", controls, "Block Size");
    assoc = field("2", controls, "Associativity");
    wxString choices[] = {"LRU", "FIFO", "Random"};
    policy = new wxRadioBox(this, wxID_ANY, "Replacement Policy", wxDefaultPosition, wxDefaultSize, 3, choices);
    controls->Add(policy, 0, wxEXPAND | wxBOTTOM, 8);
    button(controls, "Load Trace", &Frame::loadTrace);
    button(controls, "Run Simulation", &Frame::runAll);
    button(controls, "Step", &Frame::step);
    button(controls, "Reset", &Frame::reset);

    trace = new wxTextCtrl(this, wxID_ANY, sample(), wxDefaultPosition, wxSize(310, -1), wxTE_MULTILINE);
    trace->SetMinSize(wxSize(310, -1));
    auto right = new wxBoxSizer(wxVERTICAL);
    stats = new wxStaticText(this, wxID_ANY, "");
    log = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 170), wxTE_MULTILINE | wxTE_READONLY);
    view = new CacheView(this);
    view->cache = &cache;
    right->Add(stats, 0, wxEXPAND | wxBOTTOM, 6);
    right->Add(new wxStaticText(this, wxID_ANY, "Execution Log"), 0, wxBOTTOM, 2);
    right->Add(log, 0, wxEXPAND | wxBOTTOM, 8);
    right->Add(new wxStaticText(this, wxID_ANY, "Cache Visualizer"), 0, wxBOTTOM, 2);
    right->Add(view, 1, wxEXPAND);

    root->Add(controls, 0, wxEXPAND | wxALL, 10);
    root->Add(trace, 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
    root->Add(right, 1, wxEXPAND | wxALL, 10);
    SetSizer(root);
    updateStats();
}

wxTextCtrl* Frame::field(const wxString& value, wxSizer* s, const wxString& label) {
    s->Add(new wxStaticText(this, wxID_ANY, label), 0, wxBOTTOM, 2);
    auto f = new wxTextCtrl(this, wxID_ANY, value);
    s->Add(f, 0, wxEXPAND | wxBOTTOM, 8);
    return f;
}

void Frame::button(wxSizer* s, const wxString& label, void (Frame::*fn)(wxCommandEvent&)) {
    auto b = new wxButton(this, wxID_ANY, label);
    b->Bind(wxEVT_BUTTON, fn, this);
    s->Add(b, 0, wxEXPAND | wxBOTTOM, 6);
}

Policy Frame::selectedPolicy() const {
    return policy->GetSelection() == 1 ? Policy::FIFO : policy->GetSelection() == 2 ? Policy::RANDOM : Policy::LRU;
}

bool Frame::configure() {
    if (cache.ready()) return true;
    wxString err;
    if (cache.reset(wxAtoi(total->GetValue()), wxAtoi(block->GetValue()), wxAtoi(assoc->GetValue()), selectedPolicy(), err)) {
        next = 0;
        updateStats();
        view->Refresh();
        return true;
    }
    addLog(wxString("Error: ") + err);
    return false;
}

bool Frame::readTrace() {
    entries = parseTrace(trace->GetValue());
    if (!entries.empty()) return true;
    addLog("Error: Please load a trace first.");
    return false;
}

void Frame::accessOne() {
    auto r = cache.access(entries[next++]);
    view->lastSet = r.set;
    view->lastWay = r.way;
    addLog(wxString::FromUTF8(r.text.c_str()));
    updateStats();
    view->Refresh();
}

void Frame::loadTrace(wxCommandEvent&) {
    wxFileDialog dlg(this, "Open trace", "", "", "Trace files (*.txt)|*.txt|All files|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;
    std::ifstream f(dlg.GetPath().ToStdString());
    std::ostringstream s;
    s << f.rdbuf();
    trace->SetValue(wxString::FromUTF8(s.str().c_str()));
    entries.clear();
    next = 0;
}

void Frame::runAll(wxCommandEvent&) {
    if (configure() && readTrace()) while (next < entries.size()) accessOne();
}

void Frame::step(wxCommandEvent&) {
    if (configure() && (next < entries.size() || readTrace()) && next < entries.size()) accessOne();
}

void Frame::reset(wxCommandEvent&) {
    cache.set.clear();
    entries.clear();
    next = 0;
    view->lastSet = view->lastWay = -1;
    log->Clear();
    configure();
}

void Frame::addLog(const wxString& s) {
    log->AppendText(s + "\n");
}

void Frame::updateStats() {
    stats->SetLabel(wxString::Format("Hits: %d   Misses: %d   Evictions: %d", cache.stats.hits, cache.stats.misses, cache.stats.evictions));
}

wxString Frame::sample() {
    return "R 0x0000\nR 0x0040\nR 0x0080\nR 0x00C0\nR 0x0000\nW 0x0040\nR 0x0200\nR 0x0000\nR 0x0400\nR 0x0200\nW 0x0080\nR 0x0000\nR 0x00C0\nR 0x0400\nR 0x0200\nW 0x0000\nR 0x0100\nR 0x0140\n";
}
