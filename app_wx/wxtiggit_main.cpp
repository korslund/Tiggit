#include "wx/frame.hpp"
#include "gamedata.hpp"
#include "notifier.hpp"
#include "wx/boxes.hpp"
#include "jobprogress.hpp"

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

    try
      {
        if(!rep.findRepo())
          Boxes::error("Unable to find repository");

        if(!rep.initRepo())
          {
            if(Boxes::ask("Failed to lock repository: " + rep.getPath("") + "\n\nThis usually means you are either running two instances of Tiggit, or that another instance has crashed.\n\nAre you SURE you want to continue? If two programs access the repository at the same, data loss may occur!"))
              {
                if(!rep.initRepo(true))
                  {
                    Boxes::error("Still unable to lock repository. Aborting.");
                    return false;
                  }
              }
            else
              return false;
          }

        Jobify::JobInfoPtr info = rep.fetchFiles();

        // Are we doing a larger download job here?
        if(info)
          {
            // If so, keep the user informed
            wxTigApp::JobProgress prog(this, info);
            prog.start("Downloading data set...");
          }

        rep.loadData();

        gameData = new wxTigApp::GameData(rep);
        wxTigApp::notify.data = gameData;

        TigFrame *frame = new TigFrame(wxT("Tiggit"), "1", *gameData);
        frame->Show(true);
        return true;
      }
    catch(std::exception &e)
      {
        Boxes::error(std::string(e.what()));
      }
    catch(...)
      {
        Boxes::error("An unknown error occured");
      }

    return false;
  }
};

IMPLEMENT_APP(TigApp)
