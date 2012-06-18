#include "wx/frame.hpp"
#include "gamedata.hpp"

struct TigApp : wxApp
{
  TigLib::Repo rep;
  wxTigApp::GameData gameData;

  TigApp() : gameData(rep) {}

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
