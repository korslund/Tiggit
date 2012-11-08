#include "importer_backend.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"
#include <spread/spread.hpp>

using namespace std;
namespace bf = boost::filesystem;

int main()
{
  string outdir = "_out_clean";
  string log = "_clean.log";
  string input = "input_full";
  string spreadOut = "_out4";

  Misc::Logger logger(log);
  logger.print = true;

  bf::remove_all(outdir);
  bf::remove_all(spreadOut);

  Spread::SpreadLib spread(spreadOut, "_tmp4");

  cout << "Copying input directory:\n";
  Spread::JobInfoPtr info;
  Import::copyFiles(input, outdir, false, &spread, info, logger);

  cout << "\nBEFORE:\n";
  printDir(outdir);

  vector<string> games;
  games.push_back("dummy/dir/subdir");
  games.push_back("tiggit.net/game1");
  games.push_back("tiggit.net/backslash");
  Import::cleanup(outdir, games, logger);

  cout << "\nAFTER:\n";
  printDir(outdir);

  return 0;
}
