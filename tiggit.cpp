#include <wx/wx.h>
#include <json/json.h>

#include <string>
#include <vector>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <stdexcept>

using namespace std;

struct DataList
{
  struct Entry
  {
    int status;
    wxString idname, name, desc, fpshot, tigurl;
  };

  std::vector<Entry> arr;

  void add(int status, const wxString &idname, const wxString &name,
           const wxString &desc, const wxString &fpshot,
           const wxString &tigurl)
  {
    Entry e = { status, idname, name, desc, fpshot, tigurl };
    arr.push_back(e);
  }

  void add(int status,
           const std::string &idname,
           const std::string &name,
           const std::string &desc,
           const std::string &fpshot,
           const std::string &tigurl)
  {
    add(status,
        wxString(idname.c_str(), wxConvUTF8),
        wxString(name.c_str(), wxConvUTF8),
        wxString(desc.c_str(), wxConvUTF8),
        wxString(fpshot.c_str(), wxConvUTF8),
        wxString(tigurl.c_str(), wxConvUTF8));
  }
};

DataList data;

struct TigList
{
  std::string filename, channel, desc, location, homepage;

  void fail(const std::string &msg)
  {
    throw std::runtime_error("ERROR in '" + filename + "': " + msg);
  }

  void loadData(const std::string &file, DataList &data)
  {
    using namespace Json;
    filename = file;

    Value root;

    {
      std::ifstream inf(file.c_str());
      if(!inf)
        fail("Cannot read file");

      Reader reader;
      if(!reader.parse(inf, root))
        fail(reader.getFormatedErrorMessages());
    }

    // Check file type
    if(root["type"] != "tiglist 1.0")
      fail("Not a valid tiglist");

    channel = root["name"].asString();
    desc = root["desc"].asString();
    location = root["location"].asString();
    homepage = root["homepage"].asString();

    // This must be present, the rest are optional
    if(channel == "") fail("Missing or invalid channel name");

    // Traverse the list
    root = root["list"];

    // We have to do it this way because the jsoncpp iterators are
    // b0rked.
    Value::Members keys = root.getMemberNames();
    Value::Members::iterator it;
    for(it = keys.begin(); it != keys.end(); it++)
      {
        const std::string &key = *it;
        Value game = root[key];

        // Push the game into the list
        data.add(0, key,
                 game["title"].asString(), game["desc"].asString(),
                 game["fpshot"].asString(), game["tigurl"].asString());
      }
  }
};

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

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, _("&App"));

    SetMenuBar( menuBar );

    CreateStatusBar();
    SetStatusText(_("Welcome to tiggit!"));

    list = new MyList(this, myID_LIST);

    wxBoxSizer *leftPane = new wxBoxSizer(wxVERTICAL);
    leftPane->Add(list, 1, wxGROW | wxALL, 10);
    leftPane->Add(new wxStaticText(this, wxID_ANY, wxT("Mouse: click to view, double-click to install / play\nKeyboard: arrow keys + enter")), 0, wxALL, 10);

    wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);
    b1 = new wxButton(this, myID_BUTTON1, wxT("Button1"));
    b2 = new wxButton(this, myID_BUTTON2, wxT("Button2"));
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

    updateData();
    list->SetFocus();
  }

  void updateData() { list->update(); }

  void doAction(int index, int b)
  {
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
    TigList lst;
    lst.loadData("all_games.json", data);

    MyFrame *frame = new MyFrame(wxT("Custom list"));
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
