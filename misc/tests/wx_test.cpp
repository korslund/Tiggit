#include "dirfinder.hpp"

#define wxUSE_UNICODE 1
#include <wx/stdpaths.h>
#include <wx/wx.h>

#include <iostream>
using namespace std;
using namespace Misc;

DirFinder fnd("tiggit.net", "tiggit", "finder-test");

void testWrite(const std::string &path)
{
  cout << path << ": ";
  if(DirFinder::isWritable(path))
    cout << "GOOD\n";
  else
    cout << "BAD\n";
}

class MyApp : public wxApp
{
public:
  virtual bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    SetAppName(wxT("tiggit"));

    string wxdata(wxStandardPaths::Get().GetUserLocalDataDir().mb_str());
    cout << "wxW suggestion: ";
    testWrite(wxdata);

    cout << "Our suggestion: ";
    string out;
    fnd.getStandardPath(out);
    testWrite(out);
    return false;
  }
};

IMPLEMENT_APP(MyApp)
