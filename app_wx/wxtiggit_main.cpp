#include "wx/frame.hpp"
#include "gamedata.hpp"

struct TigApp : wxApp
{
  wxTigApp::GameData gameData;

  bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    SetAppName(wxT("Tiggit"));
    wxInitAllImageHandlers();

    TigFrame *frame = new TigFrame(wxT("Tiggit"), "1", gameData);
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(TigApp)
