#include <wx/wx.h>

#include <string>
#include <iostream>
using namespace std;

class MyList : public wxListCtrl
{
  wxListItemAttr green, orange, gray;

public:
  MyList(wxWindow *parent)
    : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                 wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
  {
    wxListItem col0;
    col0.SetId(0);
    col0.SetText( wxT("Name") );
    col0.SetWidth(160);
    InsertColumn(0, col0);
        
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( wxT("Description") );
    col1.SetWidth(200);
    InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetText( wxT("Status") );
    col2.SetWidth(250);
    InsertColumn(2, col2);

    SetItemCount(10);

    SetFocus();
    //SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

    /*
    green.SetBackgroundColour(wxColour(100,255,100));
    orange.SetBackgroundColour(wxColour(255,200,100));
    gray.SetBackgroundColour(wxColour(200,200,200));
    */
    green.SetTextColour(wxColour(0,120,0));
    orange.SetTextColour(wxColour(160,100,0));
    gray.SetTextColour(wxColour(60,60,60));
  }

  wxListItemAttr *OnGetItemAttr(long item) const
  {
    if(item < 2)
      return (wxListItemAttr*)&green;
    else if(item < 4)
      return (wxListItemAttr*)&orange;

    return NULL;
  }

  int OnGetItemImage(long item) const
  {
    return -1;
  }

  int OnGetColumnImage(long item, long column) const
  {
    return -1;
  }

  wxString OnGetItemText(long item, long column) const
  {
    if(column == 0)
      if(item == 2)
        return wxT("Desktop Dungeons");
      else if(item == 4)
        return wxT("The Battle for Wesnoth");
      else
        return wxT("My Game");
    else if(column == 1)
      return wxT("This is a game about a crappy-looking game installer");
    else
      if(item < 2)
        return wxT("Ready to play");
      else if(item == 2)
        return wxT("34.7% (21.9Mb / 63Mb, 118kb/s)");
      else if(item == 3)
        return wxT("Unpacking...");
      else
        return wxT("Not installed");
  }
};

class MyFrame : public wxFrame
{
public:
  MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600))
  {
    Centre();

    MyList *list = new MyList(this);

    // Add a 10px border
    wxBoxSizer *border = new wxBoxSizer(wxVERTICAL);
    border->Add(list, 1, wxGROW | wxALL, 10);
    border->Add(new wxStaticText(this, wxID_ANY, wxT("Mouse: click to view, double-click to install / play\nKeyboard: arrow keys + enter")), 0, wxALL, 10);
    //border->Add(new wxButton(this, wxID_ANY, wxT("tmp")), 0, wxGROW | wxALL, 10);

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
