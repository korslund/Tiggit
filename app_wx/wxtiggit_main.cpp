#include "wx/frame.hpp"
#include "gamedata.hpp"
#include "notifier.hpp"
#include "wx/boxes.hpp"
#include "jobprogress.hpp"
#include "wx/dialogs.hpp"
#include <wx/cmdline.h>

/* Command line options
 */

static const wxCmdLineEntryDesc cmdLineDesc[] =
  {
    { wxCMD_LINE_SWITCH, wxT("o"), wxT("offline"), wxT("Offline mode") },
    { wxCMD_LINE_OPTION, wxT("r"), wxT("repo"), wxT("Use repository dir. Will not use or set the default repository location."), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("Display this help"),
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_NONE }
  };

struct TigApp : wxApp
{
  TigLib::Repo rep;
  wxTigApp::GameData *gameData;

  std::string param_repo;
  bool param_offline;

  void OnInitCmdLine(wxCmdLineParser& parser)
  {
    parser.SetDesc (cmdLineDesc);
    parser.SetSwitchChars(wxT("-"));
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser)
  {
    param_offline = parser.Found(wxT("o"));

    wxString str;
    if(parser.Found(wxT("r"), &str))
      param_repo = wxToStr(str);

    return true;
  }

  bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    // Use to test offline mode
    rep.offline = param_offline;

    SetAppName(wxT("Tiggit"));
    wxInitAllImageHandlers();

    try
      {
        if(param_repo != "")
          rep.setRepo(param_repo);
        else if(!rep.findRepo())
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

        Spread::JobInfoPtr info = rep.fetchFiles();

        // Are we doing a larger download job here?
        if(info)
          {
            // If so, keep the user informed
            wxTigApp::JobProgress prog(this, info);
            if(!prog.start("Downloading initial data set...\nDestination directory: " + rep.getPath("")))
              {
                if(info->isError())
                  Boxes::error("Download failed: " + info->getMessage());
                return false;
              }
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
