#include "importer_backend.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"
#include <spread/spread.hpp>

using namespace std;
namespace bf = boost::filesystem;

int main()
{
  string outdir = "_out_shots";
  string log = "_shots.log";
  string input = "input_full";
  string spreadOut = "_out3";

  Misc::Logger logger(log);
  logger.print = true;

  bf::remove_all(outdir);
  bf::remove_all(spreadOut);

  Spread::SpreadLib spread(spreadOut, "_tmp3");

  cout << "Importing screenshots:\n";
  Spread::JobInfoPtr info = Import::importShots(input, outdir, &spread, logger);
  if(info)
    {
      cout << "Waiting for job to finish...\n";
      info->wait();
      if(info->isError())
        cout << "FAILURE: " << info->getMessage() << endl;
    }
  else
    cout << "ERROR: Info was empty\n";

  if(bf::exists(outdir))
    printDir(outdir);

  return 0;
}
