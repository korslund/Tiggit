#include <wx/wx.h>

#include <iostream>
#include <assert.h>

#include "curl_get.hpp"
#include "decodeurl.hpp"
#include "filegetter.hpp"
#include "data_reader.hpp"

using namespace std;

DataList data;

// Update data from stored all_games.json
void updateData()
{
  data.arr.resize(0);
  string lstfile = (get.base / "all_games.json").string();
  TigListReader lst;
  lst.loadData(lstfile, data);
}

// Refresh all_games.json from the net. Call updateData() after
// calling this.
void refreshData()
{
  string url = "http://tiggit.net/api/all_games.json";
  string tmpfile = get.getFile(url);
  get.copyTo(tmpfile, "all_games.json");
}

class MyList : public wxListCtrl
{
  wxListItemAttr green, orange, gray;

public:
  MyList(wxWindow *parent, int ID=wxID_ANY)
    : wxListCtrl(parent, ID, wxDefaultPosition, wxDefaultSize,
                 wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
  {
    wxListItem col;

    col.SetId(0);
    col.SetText( wxT("Name") );
    col.SetWidth(200);
    InsertColumn(0, col);
        
    col.SetId(1);
    col.SetText( wxT("Description") );
    col.SetWidth(230);
    InsertColumn(1, col);

    col.SetId(2);
    col.SetText( wxT("Status") );
    col.SetWidth(240);
    InsertColumn(2, col);

    /*
    green.SetBackgroundColour(wxColour(100,255,100));
    orange.SetBackgroundColour(wxColour(255,200,100));
    gray.SetBackgroundColour(wxColour(200,200,200));
    */
    green.SetTextColour(wxColour(0,120,0));
    orange.SetTextColour(wxColour(160,100,0));
    gray.SetTextColour(wxColour(60,60,60));
  }

  void setSelect(int index)
  {
    SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  }

  void update()
  {
    updateData();
    SetItemCount(data.arr.size());
    setSelect(0);
  }

  wxListItemAttr *OnGetItemAttr(long item) const
  {
    int s = data.arr[item].status;
    if(s == 0) return NULL;
    if(s == 2) return (wxListItemAttr*)&green;
    return (wxListItemAttr*)&orange;
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
    const DataList::Entry &e = data.arr[item];

    if(column == 0)
      return e.name;

    if(column == 1)
      return e.desc;

    int s = e.status;

    if(s == 0)
      return wxT("Not installed");

    if(s == 2)
      return wxT("Ready to play");

    if(s == 1)
      return wxT("34.7% (21.9Mb / 63Mb, 118kb/s)");

    if(s == 3)
      return wxT("Unpacking...");

    assert(0);
  }
};

#define myID_BUTTON1 21
#define myID_BUTTON2 22
#define myID_LIST 25
#define myID_MENU_REFRESH 30

class MyFrame : public wxFrame
{
  wxButton *b1, *b2;
  MyList *list;
  int select;

public:
  MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600))
  {
    Centre();

    wxMenu *menuFile = new wxMenu;

    menuFile->Append(wxID_ABOUT, _("&About..."));
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, _("E&xit"));

    wxMenu *menuList = new wxMenu;
    menuList->Append(myID_MENU_REFRESH, wxT("&Reload list"));

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, _("&App"));
    menuBar->Append(menuList, _("&List"));

    SetMenuBar( menuBar );

    CreateStatusBar();
    SetStatusText(_("Welcome to tiggit!"));

    // Without this the menubar doesn't appear (in GTK at least) until
    // you move the mouse. Probably some weird bug.
    SendSizeEvent();

    list = new MyList(this, myID_LIST);

    wxBoxSizer *leftPane = new wxBoxSizer(wxVERTICAL);
    leftPane->Add(list, 1, wxGROW | wxALL, 10);
    leftPane->Add(new wxStaticText(this, wxID_ANY, wxT("Mouse: double-click to install / play\nKeyboard: arrow keys + enter")), 0, wxALL, 10);

    wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);
    b1 = new wxButton(this, myID_BUTTON1, wxT("No action"));
    b2 = new wxButton(this, myID_BUTTON2, wxT("No action"));
    b2->Show(false);
    rightPane->Add(b1, 0, wxBOTTOM | wxRIGHT, 10);
    rightPane->Add(b2, 0, wxBOTTOM | wxRIGHT, 10);

    wxBoxSizer *panes = new wxBoxSizer(wxHORIZONTAL);
    panes->Add(leftPane, 1, wxGROW);
    panes->Add(rightPane, 0, wxGROW | wxTOP, 35);

    SetSizer(panes);

    Connect(myID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyFrame::onButton));
    Connect(myID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyFrame::onButton));

    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(MyFrame::onListSelect));
    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(MyFrame::onListDeselect));

    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
            wxListEventHandler(MyFrame::onListRightClick));
    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
            wxListEventHandler(MyFrame::onListActivate));

    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onAbout));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onExit));

    Connect(myID_MENU_REFRESH, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onRefresh));

    updateData();
    list->SetFocus();
  }

  void onRefresh(wxCommandEvent &event)
  {
    refreshData();
    updateData();
  }

  void onAbout(wxCommandEvent &event)
  {
    cout << "About tiggit\n";
  }

  void onExit(wxCommandEvent &event)
  {
    Close();
  }

  void updateData() { list->update(); }

  void doAction(int index, int b)
  {
    if(index < 0 || index >= data.arr.size())
      return;

    const DataList::Entry &e = data.arr[index];

    // True when called as an 'activate' command.
    bool activate = false;

    if(b == 3)
      {
        activate = true;
        b = 1;
      }

    if(e.status == 0 && b == 1)
      {
        cout << "Installing";
      }
    else if(e.status == 1 || e.status == 3)
      {
        if(activate)
          cout << "No action on";
        else if(b == 1)
          cout << "Pausing";
        else if(b == 2)
          cout << "Aborting";
      }
    else if(e.status == 2)
      {
        if(b == 1)
          cout << "Playing";
        else if(b == 2)
          cout << "Uninstalling";
      }
    cout << " " << e.name.mb_str() << "\n";
  }

  void onButton(wxCommandEvent &event)
  {
    int b = event.GetId()-20;
    doAction(select, b);
    list->SetFocus();
  }

  void onListDeselect(wxListEvent &event)
  {
    select = -1;
    b1->Disable();
    b2->Disable();
  }

  void onListSelect(wxListEvent &event)
  {
    select = event.GetIndex();
    //cout << "Selection " << select << endl;
    int s = data.arr[select].status;

    b1->Enable();
    b2->Enable();
    b2->Show(true);

    if(s == 0)
      {
        b1->SetLabel(wxT("Install"));
        b2->Show(false);
      }
    else if(s == 1 || s == 3)
      {
        b1->SetLabel(wxT("Pause"));
        b2->SetLabel(wxT("Abort"));
      }
    else if(s == 2)
      {
        b1->SetLabel(wxT("Play Now"));
        b2->SetLabel(wxT("Uninstall"));
      }
  }

  void onListRightClick(wxListEvent &event)
  {
    cout << "Context menu on " << event.GetIndex() << endl;
  }

  void onListActivate(wxListEvent &event)
  {
    doAction(event.GetIndex(), 3);
  }
};

class MyApp : public wxApp
{
public:
  virtual bool OnInit()
  {
    try
      {
        MyFrame *frame = new MyFrame(wxT("Tiggit - the indie game installer"));
        frame->Show(true);
        return true;
      }
    catch(std::exception &e)
      {
        wxString msg(e.what(), wxConvUTF8);
        wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
      }
    return false;
  }
};

IMPLEMENT_APP(MyApp)
