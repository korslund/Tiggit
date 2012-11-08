#include "importer_gui.hpp"
#include "importer_backend.hpp"
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "wx/boxes.hpp"
#include "jobprogress.hpp"

using namespace std;
using namespace Spread;
using namespace wxTiggit;

bool ImportGui::importRepoGui(const string &from, const string &to, SpreadLib *spread)
{
  Misc::Logger log((boost::filesystem::path(to) / "import.log").string());
  log("Importing repository from '" + from + "' to '" + to + "'");

  assert(spread);

  // This is quick so don't bother the user with it
  Import::importConfig(from, to, log);

  // Get game list. The names are idnames ("tiggit.net/urlname")
  vector<string> games, success;
  Import::getGameList(games, from, log);

  // Import games one by one
  for(int i=0; i<games.size(); i++)
    {
      const string &game = games[i];
      log("Importing game " + game);
      JobInfoPtr info = Import::importGame(game, from, to, spread, log);

      if(info)
        {
          wxTigApp::JobProgress prog(info);
          if(!prog.start("Importing game " + game))
            {
              if(info->isError())
                {
                  log("Failure: " + info->getMessage());
                  if(!Boxes::ask("Import failed: " + info->getMessage() + "\n\nContinue?"))
                    return false;
                }
              else
                {
                  log("Aborted by user");
                  if(!Boxes::ask("Do you want to continue importing?"))
                    return false;
                }
            }
          else
            {
              // Add successful games to a special list
              success.push_back(game);
              log("Successfully imported " + game);
            }
        }
      else
        log("No import needed");
    }

  // Import screenshots
  log("Importing screenshots");
  JobInfoPtr info = Import::importShots(from, to, spread, log);
  if(info)
    {
      wxTigApp::JobProgress prog(info);
      if(prog.start("Copying existing screenshot data to save bandwidth"))
        log("Screenshot success");
      else
        {
          if(info->isError())
            log("Screenshot failure: " + info->getMessage());
          else
            log("Screenshot aborted");
        }
    }

  if(Boxes::ask("Do you want to delete successfully imported data from " + from + "?"))
    Import::cleanup(from, success, log);

  return true;
}
