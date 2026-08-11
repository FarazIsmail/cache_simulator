#include "gui.h"

class App : public wxApp {
public:
    bool OnInit() override {
        auto f = new Frame;
        f->Show();
        return true;
    }
};

wxIMPLEMENT_APP(App);
