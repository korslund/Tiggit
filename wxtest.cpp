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

    wxPanel *panel = new wxPanel(this, wxID_ANY);
    wxButton *button = new wxButton(panel, wxID_EXIT, wxT("Quit"),
                                    wxPoint(20, 20));

    Connect(wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(Simple::onQuit));
    button->SetFocus();
  }

  void onQuit(wxCommandEvent &event)
  {
    Close(true);
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
