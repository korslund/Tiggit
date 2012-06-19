#include "wx/frame.hpp"
#include "gamedata.hpp"
#include "notifier.hpp"

// Ask the user an OK/Cancel question.
bool ask(const wxString &question)
{
  return wxMessageBox(question, wxT("Please confirm"),
                      wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
}

// Display an error message box
void errorBox(const wxString &msg)
{
  wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
}

bool ask(const std::string &q) { return ask(strToWx(q)); }
void errorBox(const std::string &q) { errorBox(strToWx(q)); }

struct MyTimer : wxTimer
{
  MyTimer(int ms)
  {
    Start(ms);
  }

  void Notify()
  {
    wxTigApp::notify.tick();
  }
};

MyTimer timer(300);

struct TigApp : wxApp
{
  TigLib::Repo rep;
  wxTigApp::GameData *gameData;

  bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    SetAppName(wxT("Tiggit"));
    wxInitAllImageHandlers();

    if(!rep.findRepo())
      errorBox(wxT("Unable to find repository"));

    if(!rep.initRepo())
      {
        if(ask("Failed to lock repository: " + rep.getPath("") + "\n\nThis usually means you are either running two instances of Tiggit, or that another instance has crashed.\n\nAre you SURE you want to continue? If two programs access the repository at the same, data loss may occur!"))
          {
            if(!rep.initRepo(true))
              {
                errorBox("Still unable to lock repository. Aborting.");
                return false;
              }
          }
        else
          return false;
      }

    rep.fetchFiles();
    rep.loadData();

    gameData = new wxTigApp::GameData(rep);
    wxTigApp::notify.data = gameData;

    TigFrame *frame = new TigFrame(wxT("Tiggit"), "1", *gameData);
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(TigApp)
