#include "importer_backend.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"

using namespace std;
namespace bf = boost::filesystem;

void impConf(string input, bool kill=true)
{
  string output = "_out_conf/" + input;
  string log = output + ".log";
  input = "input_conf/" + input;

  if(kill)
    bf::remove_all(output);
  Misc::Logger logger(log);
  logger.print = true;
  Import::importConfig(input, output, logger);
  if(bf::exists(output))
    printDir(output);
}

int main()
{
  impConf("noconf");
  impConf("invalid_conf");
  impConf("normal");
  impConf("normal", false);

  cout << "Program done!\n";
  return 0;
}
