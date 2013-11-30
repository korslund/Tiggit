#include "importer_gui.hpp"
#include "importer_backend.hpp"
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "wx/boxes.hpp"
#include "jobprogress.hpp"
#include <spread/misc/readjson.hpp>
#include "tiglib/repo.hpp"

using namespace std;
using namespace Spread;
using namespace wxTiggit;
namespace bf = boost::filesystem;

void ImportGui::doUserCleanup(const std::string &repoDir)
{
  assert(repoDir != "");

  bf::path file = repoDir;
  file /= "cleanup.json";

  // No file, no action
  if(!bf::exists(file)) return;

  try
    {
      Json::Value root = ReadJson::readJson(file.string());
      bf::remove(file);

      // Only instructions we can currently use is a single string,
      // representing the path to clean up. We have the option to
      // expand this later.
      if(root.isString())
        {
          std::string path = root.asString();

          if(Boxes::ask("Remove files from\n" + path + "?"))
            TigLib::Repo::killRepo(path);
        }
    }
  // Ignore errors
  catch(...) {}
}

bool ImportGui::copyFilesGui(const std::string &from, const std::string &to,
                             Spread::SpreadLib *spread, const std::string &text)
{
  assert(spread);
  if(bf::equivalent(from, to))
    return false;

  JobInfoPtr info = Import::copyFiles(from, to, spread);
  if(info)
    {
      wxTigApp::JobProgress prog(info);
      if(!prog.start(text))
        {
          if(info->isError())
            Boxes::error("Copy failure: " + info->getMessage());
          return false;
        }
      return true;
    }
  return false;
}

bool ImportGui::importRepoGui(const string &from, const string &to, SpreadLib *spread,
                              bool doCleanup)
{
  Misc::Logger log((boost::filesystem::path(to) / "import.log").string());
  log("Importing repository from '" + from + "' to '" + to + "'");

  if(bf::equivalent(from, to))
    {
      log("ERROR: from and to paths are equivalent");
      return false;
    }

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
        log("No import needed or game not found");
    }

  // Import screenshots
  log("Importing screenshots");
  JobInfoPtr info = Import::importShots(from, to, spread, log);
  if(info)
    {
      wxTigApp::JobProgress prog(info);
      if(prog.start("Import screenshot data"))
        log("Screenshot success");
      else
        {
          if(info->isError())
            log("Screenshot failure: " + info->getMessage());
          else
            log("Screenshot aborted");
        }
    }

  if(doCleanup && Boxes::ask("Do you want to delete successfully imported data from " + from + "?"))
    Import::cleanup(from, success, log);

  return true;
}
