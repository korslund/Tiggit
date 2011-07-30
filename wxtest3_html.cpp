#include <wx/wx.h>
#include <wx/htmllbox.h>

#include <string>
#include <iostream>
using namespace std;

class MyList : public wxSimpleHtmlListBox
{
public:
  MyList(wxWindow *parent)
    : wxSimpleHtmlListBox(parent, wxID_ANY)
  {
    Append(wxT("Hello"));
    Append(wxT("How are you?"));
    Append(wxT("I am <a href=\"http://tiggit.net\">fine</a>!"));
  }
};

class MyFrame : public wxFrame
{
public:
  MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(600, 300))
  {
    Centre();

    MyList *list = new MyList(this);

    // Add a 10px border
    wxBoxSizer *border = new wxBoxSizer(wxVERTICAL);
    border->Add(list, 1, wxGROW | wxALL, 10);

    SetSizer(border);
  }
};

class MyApp : public wxApp
{
public:
  virtual bool OnInit()
  {
    MyFrame *frame = new MyFrame(wxT("Custom list"));
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
