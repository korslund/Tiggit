#include <wx/wx.h>

#include <iostream>
using namespace std;

class Simple : public wxFrame
{
public:
    Simple(const wxString& title)
      : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 150))
  {
    Centre();
  }
};

class MyApp : public wxApp
{
public:
  virtual bool OnInit()
  {
    Simple *simple = new Simple(wxT("Hello world"));
    simple->Show(true);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
