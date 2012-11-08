#include "importer_backend.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"
#include <spread/spread.hpp>

using namespace std;
namespace bf = boost::filesystem;

int main()
{
  string outdir = "_out_games";
  string log = "_games.log";
  string input = "input_full";
  string spreadOut = "_out2";

  Misc::Logger logger(log);
  logger.print = true;

  bf::remove_all(outdir);
  bf::remove_all(spreadOut);

  cout << "Scanning " << input << " for games:\n";
  vector<string> games;
  Import::getGameList(games, input, logger);
  cout << "Found:\n";
  for(int i=0; i<games.size(); i++)
    cout << "  " << games[i] << endl;

  Spread::SpreadLib spread(spreadOut, "_tmp2");

  bf::create_directories(outdir + "/gamedata/tiggit.net/already_exists");

  for(int i=0; i<games.size(); i++)
    {
      const string &game = games[i];
      cout << "\nImporting '" << game << "':\n";
      Spread::JobInfoPtr info = Import::importGame(game, input, outdir, &spread, logger);
      if(info)
        {
          cout << "Waiting for job to finish...\n";
          info->wait();
          if(info->isError())
            cout << "FAILURE: " << info->getMessage() << endl;
        }
    }

  printDir(outdir);

  return 0;
}
