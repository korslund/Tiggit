#include "dirfinder.hpp"

#include <iostream>
using namespace std;

DirFinder::Finder fnd("tiggit.net", "tiggit", "finder-test");

void setTest(const string &dir)
{
  cout << "Attempting to set standard path to '" << dir << "': ";
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
    cout << "No stored path found, out = ";
  cout << out << endl;

  if(fnd.getStandardPath(out))
    cout << "Standard path acceptible: ";
  else
    cout << "Standard path NOT acceptible: ";
  cout << out << endl;

  setTest("/blah");
  setTest(out);
  setTest("/");

  if(fnd.getStoredPath(out))  
    cout << "Current stored path: " << out << endl;
  else
    cout << "FAILED getting stored path (out=" << out << ")\n";

  return 0;
}
