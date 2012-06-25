#include "wx/frame.hpp"
#include "gamedata.hpp"
#include "notifier.hpp"
#include "wx/boxes.hpp"
#include "jobprogress.hpp"
#include "wx/dialogs.hpp"

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

    // Use to test offline mode
    //rep.offline = true;

    SetAppName(wxT("Tiggit"));
    wxInitAllImageHandlers();

    try
      {
        if(!rep.findRepo())
          {
            std::string dir = rep.defaultPath();
            bool failed = false;

            // Unable to find a repository. Ask the user.
            while(true)
              {
                wxTiggit::OutputDirDialog dlg(NULL, dir, failed, true);

                // Exit when the user has had enough
                if(!dlg.ok)
                  return false;

                dir = dlg.path;

                // We're done when the given path is an acceptible
                // repo path.
                if(rep.findRepo(dir))
                  break;

                // If not, continue asking, and tell the user why.
                failed = true;
              }
          }

        if(!rep.initRepo())
          {
            if(Boxes::ask("Failed to lock repository: " + rep.getPath("") + "\n\nThis usually means that a previous instance of Tiggit crashed. But it MIGHT also mean you are running two instances of Tiggit at once.\n\nAre you SURE you want to continue? If two programs access the repository at the same, data loss may occur!"))
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
            prog.start("Downloading initial data set...");
            if(info->isError())
              Boxes::error("Download failed: " + info->message);
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
