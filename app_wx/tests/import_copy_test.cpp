#include "importer_backend.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"

using namespace std;
using namespace Import;

int main()
{
  boost::filesystem::remove_all("_out");
  boost::filesystem::remove_all("_output1");
  boost::filesystem::remove_all("_output2");
  boost::filesystem::remove_all("_output3");

  Spread::SpreadLib spread("_out", "_tmp");
  Spread::JobInfoPtr info;
  Misc::Logger logger("_import1.log");
  logger.print = true;

  copyFiles("input1", "_output1", false, &spread, info, logger);

  if(info)
    cout << "Progress: " << info->getCurrent() << " / " << info->getTotal() << endl;

  cout << "Copying non-existing source:\n";
  try { copyFiles("nothing", "_output2", false, &spread, info, logger); }
  catch(exception &e) { cout << "GOT: " << e.what() << endl; }
  cout << "Copying to non-writable destination:\n";
  try { copyFiles("input1", "/blah", false, &spread, info, logger); }
  catch(exception &e) { cout << "GOT: " << e.what() << endl; }

  copyFiles("input1", "_output3", true, &spread, info, logger);

  printDir("_output1");
  printDir("_output3");

  return 0;
}
