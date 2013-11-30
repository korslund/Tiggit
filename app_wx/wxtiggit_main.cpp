#include "wx/frame.hpp"
#include "gamedata.hpp"
#include "notifier.hpp"
#include "wx/boxes.hpp"
#include "importer_gui.hpp"
#include "jobprogress.hpp"
#include "wx/dialogs.hpp"
#include <wx/cmdline.h>
#include "version.hpp"
#include "misc/dirfinder.hpp"

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

/* Command line options
 */
static const wxCmdLineEntryDesc cmdLineDesc[] =
  {
    { wxCMD_LINE_SWITCH, wxT("o"), wxT("offline"), wxT("Offline mode") },
    { wxCMD_LINE_OPTION, wxT("r"), wxT("repo"), wxT("Use repository dir. Will not use or set the default repository location."), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_SWITCH, wxT("s"), wxT("reset"), wxT("Reset the repository location. Will act as if this is an initial install, and will let you select the repository location") },
    { wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("Display this help"),
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_NONE }
  };

struct TigApp : wxApp
{
  TigLib::Repo rep;
  wxTigApp::GameData *gameData;

  std::string param_repo;
  bool param_offline, param_reset;

  void OnInitCmdLine(wxCmdLineParser& parser)
  {
    parser.SetDesc (cmdLineDesc);
    parser.SetSwitchChars(wxT("-"));
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser)
  {
    param_offline = parser.Found(wxT("o"));

    param_reset = parser.Found(wxT("s"));

    wxString str;
    if(parser.Found(wxT("r"), &str))
      param_repo = wxToStr(str);

    return true;
  }

  int OnExit()
  {
    // Make sure we clean up the notifier on exit
    wxTigApp::notify.cleanup();
    return wxApp::OnExit();
  }

  bool OnInit()
  {
    PRINT("OnInit()");
    if(!wxApp::OnInit())
      return false;

    /* TODO: Use this to test the dialog boxes
    {
      wxTiggit::OutputDirDialog dlg1(NULL, "c:\\default\\location", "", false, true);
    }
    {
      wxTiggit::OutputDirDialog dlg1(NULL, "c:\\default\\location", "c:\\user\\you\\appdata\\old\\location", false, true);
    }
    return false;
    */

    rep.offline = param_offline;

    PRINT("Offline mode: " << (rep.offline?"YES":"NO"));

    SetAppName(wxT("Tiggit"));
    wxInitAllImageHandlers();

    try
      {
        // This is set if we found a legacy directory
        std::string legacy_dir;

        if(param_repo != "")
          {
            PRINT("Setting repo dir=" << param_repo);
            rep.setRepo(param_repo);
          }
        else if(param_reset || !rep.findRepo())
          {
            // We were unable to find a repository (or the user wanted
            // to reset the location.) Ask the user.

            // Check if there are any legacy locations to import
            legacy_dir = rep.findLegacyDir();
            PRINT("Legacy directory: " << legacy_dir);

            // Use the legacy dir as the initial suggestion as well,
            // if it is set. If not, use the default location.
            std::string dir;
            if(legacy_dir != "") dir = legacy_dir;
            else dir = rep.defaultPath();

            PRINT("Suggested default dir: " << dir);

            bool failed = false;
            while(true)
              {
                wxTiggit::OutputDirDialog dlg(NULL, dir, legacy_dir, failed, true);

                // Exit when the user presses Cancel
                if(!dlg.ok)
                  return false;

                dir = dlg.path;

                // We're done when the given path is an acceptible
                // repo path.
                if(rep.findRepo(dir))
                  {
                    // Blank out the legacy location if the user
                    // doesn't want us to use it
                    if(!dlg.doImport) legacy_dir = "";
                    break;
                  }

                // If not, continue asking, and tell the user why.
                failed = true;
              }

            PRINT("Set repo dir " << dir);
          }

        PRINT("Final repo dir: " << rep.getPath());

        // Check if there is a newer version installed in the repo. If
        // there is, lauch it and exit.
        {
          wxTigApp::AppUpdater upd(rep);
          PRINT("Checking for newer EXE");
          if(upd.launchCorrectExe())
            {
              PRINT("New EXE launched. Exiting.");
              return false;
            }
          PRINT("No new EXE found.");
        }

        // If there was a legacy location, import it now
        if(legacy_dir != "")
          {
            PRINT("Importing from " << legacy_dir);
            if(!ImportGui::importRepoGui(legacy_dir, rep.getPath(), &rep.getSpread(), true))
              return false;
          }

        PRINT("Initializing repository");
        {
          // Try locking the repository
          bool repOk = rep.initRepo();

          if(!repOk)
            {
              // Try waiting a short while. The lock could be the
              // remains of an exiting process that is still shutting
              // down, as is typical when we are restarting from
              // within the client itself.
              wxSleep(2);
              repOk = rep.initRepo();
            }

          if(!repOk)
            {
              // Ask the user
              if(Boxes::ask("Failed to lock repository: " + rep.getPath() + "\n\nThis usually means that a previous instance of Tiggit crashed. But it MIGHT also mean you are running two instances of Tiggit at once.\n\nAre you SURE you want to continue? If two programs access the repository at the same, data loss may occur!"))
                // The 'true' means override lock
                repOk = rep.initRepo(true);
              else
                return false;
            }

          if(!repOk)
            {
              Boxes::error("Still unable to lock repository. Aborting.");
              return false;
            }

          assert(repOk);
        }

        // Double-check that the repository is writable
        if(!Misc::DirFinder::isWritable(rep.getPath()))
          {
            if(!Boxes::ask("Repo path " + rep.getPath() + " does not seem to be writable. Do you want to continue anyway?\n\nIf this fails, try rerunning tiggit.exe with the --reset parameter, and pick another repo location."))
              return false;
          }

        // Set up the GameData struct
        gameData = new wxTigApp::GameData(rep);

        // Try loading existing data
        try
          {
            PRINT("Trying to load data");

            // This throws on error
            gameData->loadData();

            // Check for updates in the background. The StatusNotifier
            // in wxTigApp::notify will make sure the rest of the
            // system is informed when the data has finished loading.
            PRINT("Success. Starting background update job");
            wxTigApp::notify.updateJob = gameData->updater.startJob();
          }
        catch(...)
          {
            PRINT("Load failed. Doing foreground update.");

            /* Getting here is the normal case for fresh installs.

               A fresh install will put the exe and dlls into a
               separate directory, then proceed to install the updated
               version from the net. However, since in most cases the
               DLLs will not have changed, we can save some download
               time by adding the local dll files (we add all the
               files in the exe dir) to the Spread cache. The Spread
               library will then find and automatically use these
               instead of re- downloading.
             */
            gameData->updater.cacheLocalExeDir();

            /* If there were any errors, assume this means the data
               has either not been downloaded yet, or that the data
               has changed format and we need to update the client
               itself.
             */
            Spread::JobInfoPtr info = gameData->updater.startJob();

            if(info)
              {
                // Keep the user informed about what we're doing
                wxTigApp::JobProgress prog(info);
                if(!prog.start("Updating data...\nDestination directory: " + rep.getPath()))
                  {
                    if(info->isError())
                      Boxes::error("Download failed: " + info->getMessage());
                    return false;
                  }
              }

            PRINT("Checking for new EXE");
            if(gameData->updater.launchIfNew())
              {
                PRINT("Found and launched. Exit.");
                return false;
              }

            // If there was no new app version, then try loading the
            // data again
            try { gameData->loadData(); }
            catch(std::exception &e)
              {
                Boxes::error("Failed to load data: " + std::string(e.what()));
                return false;
              }
          }

        /* Start the notifier system. The notifier is a cleanup
           procedure that polls threads regularly for status, and acts
           on status changes. It is run in the MAIN thread at regular
           intervals, through wxTimer.

           However, it doesn't start doing anything until we have set
           the data member.
         */
        PRINT("Starting notification loop");
        wxTigApp::notify.data = gameData;

        TigFrame *frame = new TigFrame(wxT("Tiggit"), TIGGIT_VERSION, *gameData);
        frame->Show(true);
        gameData->frame = frame;

        PRINT("last_time: " << rep.getLastTime());

        // Check for and act on cleanup instructions.
        ImportGui::doUserCleanup(rep.getPath());

        // TODO: After a repo move, we have copied files into
        // spread/cache/, but these might not actually be visible to
        // the system at this point. We might need to cache them
        // manually.

        // TODO: Reaffirm the stored path. This might be needed on
        // Linux, where the path is stored in a file. In some cases
        // the user cleanup above can delete that file.
        //rep.setStoredPath(rep.getPath());

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
