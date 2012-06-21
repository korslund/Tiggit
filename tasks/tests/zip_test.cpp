#include "../unpack.hpp"

#include <iostream>
using namespace std;

int main()
{
  string zip = "../../unpack/tests/archives/test.zip";
  cout << "Unpacking " << zip << "...\n";

  Tasks::UnpackTask unp(zip, "_outdir1");
  Jobify::JobInfoPtr info = unp.getInfo();

  unp.run();

  if(info->isSuccess())
    cout << "Success!\n";
  else
    cout << "Failure: " << info->message << endl;

  cout << "Progress: " << info->current << "/" << info->total << endl;

  return 0;
}
