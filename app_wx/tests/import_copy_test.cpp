#include "importer.hpp"
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace wxTigApp;

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

  copyTest("input1", "_output1", false, &spread, info, &logger);

  if(info)
    cout << "Progress: " << info->getCurrent() << " / " << info->getTotal() << endl;

  cout << "Copying non-existing source:\n";
  try { copyTest("nothing", "_output2", false, &spread, info, &logger); }
  catch(exception &e) { cout << "GOT: " << e.what() << endl; }
  cout << "Copying to non-writable destination:\n";
  try { copyTest("input1", "/blah", false, &spread, info, &logger); }
  catch(exception &e) { cout << "GOT: " << e.what() << endl; }

  copyTest("input1", "_output3", true, &spread, info, &logger);

  return 0;
}
