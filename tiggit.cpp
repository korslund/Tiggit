#include <wx/wx.h>

#include <string>
#include <vector>
#include <iostream>
#include <assert.h>
using namespace std;

struct DataList
{
  struct Entry
  {
    int status;
    wxString idname, name, desc;
  };

  std::vector<Entry> arr;

  void add(int status, const wxString &idname, const wxString &name, const wxString &desc)
  {
    Entry e = { status, idname, name, desc };
    arr.push_back(e);
  }

  void add(int status, const char* idname, const char* name, const char* desc)
  {
    add(status, wxString(idname, wxConvUTF8), wxString(name, wxConvUTF8), wxString(desc, wxConvUTF8));
  }
};

DataList data;

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
    col.SetWidth(160);
    InsertColumn(0, col);
        
    col.SetId(1);
    col.SetText( wxT("Description") );
    col.SetWidth(200);
    InsertColumn(1, col);

    col.SetId(2);
    col.SetText( wxT("Status") );
    col.SetWidth(250);
    InsertColumn(2, col);

    SetFocus();

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

    data.add(2, "col", "Colonization", "A really old one, this");
    data.add(2, "tt", "Tiny Tactics", "A cool tactical RPG in the style of Final Fantasy Tactics and Vandal Hearts");
    data.add(2, "mith", "Mithril", "Underground strategy and colonization simulator");
    data.add(2, "plan", "Planet", "In the biggest exploration game ever created, you are given an entire planet.");
    data.add(1, "port", "Portal", "Eh, not really an indie game, is it?");
    data.add(3, "sc", "SpaceChem", "This one certainly isn't bad");
    data.add(1, "cw2", "Creeper World 2", "I love this one too");
    data.add(0, "mc", "Minecraft", "Exploration mega-hit");
    data.add(0, "aquaria", "Aquaria", "This is a cool underwater game");
    data.add(0, "creeper-world", "Creeper World", "Man, I love this game");
    data.add(0, "dd", "Desktop Dungeons", "Go underground and kill stuff");

    update();
  }

  void update() { list->update(); }

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
    MyFrame *frame = new MyFrame(wxT("Custom list"));
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
