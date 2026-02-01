#include <wx/wx.h>
#include "gui/MainFrame.h"

class CppConomyApp : public wxApp
{
public:
    bool OnInit() override
    {
// Enable high DPI support
#ifdef __WXMSW__
        SetProcessDPIAware();
#endif

        MainFrame *frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(CppConomyApp);
