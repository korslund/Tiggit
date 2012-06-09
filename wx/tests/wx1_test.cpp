#include "frame.hpp"

using namespace std;
using namespace wxTiggit;

struct TigApp : wxApp
{
  virtual bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    SetAppName(wxT("Test App"));
    wxInitAllImageHandlers();

    TigFrame *frame = new TigFrame(wxT("Test App"), "1");
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(TigApp)
