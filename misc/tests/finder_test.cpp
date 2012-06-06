#include <iostream>
#include <fstream>
using namespace std;

#include "dirfinder.hpp"

Misc::DirFinder fnd("tiggit.net", "tiggit", "finder-test");

void setTest(const string &dir)
{
  cout << "Attempting to set stored path to '" << dir << "': ";
  if(fnd.setStoredPath(dir))
    cout << "SUCCESS\n";
  else
    cout << "FAILURE\n";
}

int main()
{
  string out;

  if(fnd.getStoredPath(out))
    cout << "Found stored path: ";
  else
    cout << "No stored path found, out = '";
  cout << out << (fnd.isWritable(out)?"' (ok)":"' (not ok)") << endl;

  if(fnd.getStandardPath(out))
    cout << "Standard path acceptible: ";
  else
    cout << "Standard path NOT acceptible: ";
  cout << out << (fnd.isWritable(out)?" (ok)":" (not ok)") << endl;

  setTest("/blah");
  setTest(out);
#ifdef _WIN32
  setTest("c:\\windows\\system\\");
#else
  setTest("/");
#endif

  if(fnd.getStoredPath(out))  
    cout << "Current stored path: " << out << endl;
  else
    cout << "FAILED getting stored path (out=" << out << ")\n";
  return 0;
}
