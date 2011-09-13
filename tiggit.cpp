#define wxUSE_UNICODE 1

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/listctrl.h>
#include <wx/accel.h>
#include <wx/imaglist.h>

#include <iostream>
#include <assert.h>
#include <set>
#include <time.h>

#include "curl_get.hpp"
#include "decodeurl.hpp"
#include "filegetter.hpp"
#include "data_reader.hpp"
#include "thread_get.hpp"
#include "install.hpp"
#include "json_installed.hpp"
#include "auto_update.hpp"

using namespace std;

DataList data;
JsonInstalled jinst;
TigListReader tig_reader;

void writeConfig()
{
  jinst.write(data);
}

// Update data from all_games.json. If the parameter is true, force
// download. If not, download only if the existing file is too old.
void updateData(bool download)
{
  data.arr.resize(0);

  string lstfile = get.getPath("all_games.json");

  // Do we check file age?
  if(!download && boost::filesystem::exists(lstfile))
    {
      // Yup, check file age
      time_t ft = boost::filesystem::last_write_time(lstfile);
      time_t now = time(0);

      // 24 hour updates should be ok
      if(difftime(now,ft) > 60*60*24)
        download = true;
    }

  if(download)
    {
      // Get the latest list from the net
      string url = "http://tiggit.net/api/all_games.json";
      get.getTo(url, "all_games.json");
    }

  tig_reader.loadData(lstfile, data);
  jinst.read(data);
}

class MyList : public wxListCtrl
{
  wxListItemAttr green, orange;
  wxString textNotInst, textReady;
  wxImageList images;

public:
  MyList(wxWindow *parent, int ID=wxID_ANY)
    : wxListCtrl(parent, ID, wxDefaultPosition, wxDefaultSize,
                 wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
  {
    textNotInst = wxT("Not installed");
    textReady = wxT("Ready to play");

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

    green.SetBackgroundColour(wxColour(180,255,180));
    green.SetTextColour(wxColour(0,0,0));

    orange.SetBackgroundColour(wxColour(255,240,180));
    orange.SetTextColour(wxColour(0,0,0));

    /* For later. Works, but doesn't look good.
    images.Create(80,50,false);
    SetImageList(&images, wxIMAGE_LIST_SMALL);

    wxImage::AddHandler(new wxPNGHandler);
    wxImage img(wxT("tmp.png"));
    wxBitmap bmp(img);
    images.Add(bmp);
    */
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
    static int i = 0;

    const DataList::Entry &e = data.arr[item];

    if(column == 0)
      return e.name;

    if(column == 1)
      return e.desc;

    assert(column == 2);

    int s = e.status;

    if(s == 0)
      return textNotInst;

    if(s == 2)
      return textReady;

    if(s == 1 || s == 3)
      return e.msg;

    assert(0);
  }
};

#define myID_BUTTON1 21
#define myID_BUTTON2 22
#define myID_GAMEPAGE 24
#define myID_LIST 25
#define myID_MENU_REFRESH 30

class MyFrame : public wxFrame
{
  wxButton *b1, *b2;
  MyList *list;
  int select;
  std::string version;

  typedef std::set<int> IntSet;
  IntSet updateList;

public:
  MyFrame(const wxString& title, const std::string &ver)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      version(ver)
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
    SetStatusText(wxString(("Welcome to tiggit - version " + ver).c_str(), wxConvUTF8));

    // Without this the menubar doesn't appear (in GTK at least) until
    // you move the mouse. Probably some weird bug.
    SendSizeEvent();

    wxPanel *panel = new wxPanel(this);

    list = new MyList(panel, myID_LIST);

    wxBoxSizer *leftPane = new wxBoxSizer(wxVERTICAL);
    leftPane->Add(list, 1, wxGROW | wxALL, 10);
    leftPane->Add(new wxStaticText(panel, wxID_ANY, wxT("Mouse: double-click to install / play\nKeyboard: arrow keys + enter, delete")), 0, wxALL, 10);

    wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);

    b1 = new wxButton(panel, myID_BUTTON1, wxT("No action"));
    rightPane->Add(b1, 0, wxBOTTOM | wxRIGHT, 10);

    b2 = new wxButton(panel, myID_BUTTON2, wxT("No action"));
    rightPane->Add(b2, 0, wxBOTTOM | wxRIGHT, 10);

    /*
    wxButton *b3 = new wxButton(panel, myID_GAMEPAGE, wxT("Homepage"));
    rightPane->Add(b3, 0, wxBOTTOM | wxRIGHT, 10);
    */

    wxBoxSizer *panes = new wxBoxSizer(wxHORIZONTAL);
    panes->Add(leftPane, 1, wxGROW);
    panes->Add(rightPane, 0, wxGROW | wxTOP, 35);

    panel->SetSizer(panes);

    // Add delete = 2nd button accelerator
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, myID_BUTTON2);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    Connect(myID_GAMEPAGE, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyFrame::onGamePage));

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

    list->update();
    list->SetFocus();
  }

  void onRefresh(wxCommandEvent &event)
  {
    updateData(true);
    list->update();
  }

  void onAbout(wxCommandEvent &event)
  {
    std::string str = "TIGGIT - The Indie Game Installer\nVersion " + version;
    wxMessageBox(wxString(str.c_str(), wxConvUTF8), wxT("About"), wxOK);
  }

  void onExit(wxCommandEvent &event)
  {
    Close();
  }

  // Create a nice size string
  wxString sizify(int size)
  {
    wxString tmp;
    if(size > 1024*1024)
      tmp << wxString::Format(wxT("%.1f"), size/(1024.0*1024)) << wxT("Mb");
    else if(size > 1024)
      tmp << wxString::Format(wxT("%.1f"), size/1024.0) << wxT("Kb");
    else
      tmp << size;

    return tmp;
  }

  void handleDownload(int index)
  {
    DataList::Entry &e = data.arr[index];

    // If this is the current item, make sure the buttons are set
    // correctly.
    if(index == select) fixButtons();

    // Neither downloading or unpacking
    if(e.status != 1 && e.status != 3)
      {
        // We're done, remove ourselves from the list
        updateList.erase(index);

        // Make sure config is updated
        writeConfig();
        return;
      }

    // Are we currently unpacking?
    if(e.status == 3)
      {
        // Check the installer status
        int res = inst.check(e.extra);

        if(res == 2)
          {
            // Success. Move to "playable" status.
            e.status = 2;
            e.extra = NULL;
          }
        else if(res > 2)
          {
            // Install was aborted. Revert to "not installed" status.
            e.status = 0;
            e.extra = NULL;

            // If there was an error, report it to the user
            if(res == 3)
              wxMessageBox(wxT("Unpacking failed for ") + e.name, wxT("Error"), wxOK | wxICON_ERROR);
          }
        /*
        else if(res == 0)
          e.msg = wxT("Queued for installation");
        */
        else if(res == 1 || res == 0)
          e.msg = wxT("Unpacking...");

        else assert(0);
      }
    else
      {
        // If we get here, we are currently downloading something.
        assert(e.status == 1);
        assert(e.extra);
        const ThreadGet *g = (ThreadGet*)e.extra;

        // Has the download finished?
        if(g->status >= 2)
          {
            if(g->status == 2)
              {
                // Success. Move to unpacking stage.
                e.status = 3;

                // Construct the install path
                boost::filesystem::path dir = "data";
                dir /= string(e.idname.mb_str());

                // Set up installer, store the reference struct.
                e.extra = inst.queue(g->file, get.getPath(dir.string()));
              }
            else
              {
                // Abort.
                e.status = 0;
                e.extra = NULL;

                if(g->status == 3)
                  wxMessageBox(wxT("Download failed for ") + e.name +
                               wxT(":\n") + wxString(g->errMsg.c_str(), wxConvUTF8),
                               wxT("Error"), wxOK | wxICON_ERROR);
              }

            // We don't need the downloader anymore
            delete g;
          }
        else if(g->status == 1)
          {
            // Download is still in progress

            // Aw, fixed a div-by-zero bug, how cute :)
            int percent = 0;
            if(g->total > 0)
              percent = (int)((100.0*g->current)/g->total);

            // Update the visible progress message.
            wxString status;
            status << percent << wxT("% (");
            status << sizify(g->current) << wxT(" / ");
            status << sizify(g->total) << wxT(")");

            e.msg = status;
          }
      }

    // In any case, update the display
    list->RefreshItem(index);
  }

  // Called regularly by an external timer, used to update
  // thread-dependent
  void tick()
  {
    for(IntSet::const_iterator it = updateList.begin();
        it != updateList.end();)
      {
        int i = *it;

        // Update iterator first, handleDownload() might remove the
        // item from the list.
        it++;

        handleDownload(i);
      }
  }

  void startDownload(int index)
  {
    assert(index >= 0 && index < data.arr.size());
    DataList::Entry &e = data.arr[index];

    if(e.extra)
      {
        cout << "FAIL: e.extra not NULL (internal error)\n";
        return;
      }
    if(e.status != 0)
      {
        cout << "FAIL: wrong status " << e.status << "(internal error)\n";
        return;
      }

    /* TODO: We should update the tigfile before downloading.
     */

    // Start theaded downloading
    ThreadGet *tg = new ThreadGet;

    string url = e.tigInfo.url;

    // Construct the file name
    boost::filesystem::path dir = "incoming";
    dir /= string(e.idname.mb_str()) + ".zip";
    string out = get.getPath(dir.string());

    //cout << url << " => " << out << endl;
    tg->start(url, out);

    // Update list status
    e.status = 1;
    e.extra = tg;

    // Finally, remember to update this entry later, and update it
    // now.
    updateList.insert(index);
    handleDownload(index);
  }

  void doAction(int index, int b)
  {
    if(index < 0 || index >= data.arr.size())
      return;

    const DataList::Entry &e = data.arr[index];

    // True called as an 'activate' command, meaning that the list
    // told us the item was activated (it was double clicked or we
    // pressed 'enter'.)
    bool activate = false;

    if(b == 3)
      {
        activate = true;
        b = 1;
      }

    if(e.status == 0 && b == 1)
      {
        startDownload(index);
        return;
      }
    else if(e.status == 1 || e.status == 3)
      {
        if(e.status == 1) // Downloading
          {
            ThreadGet *g = (ThreadGet*)e.extra;
            if(b == 2) // Abort
              g->status = 4;
          }
        else if(e.status == 3) // Installing
          {
            if(b == 2) // Abort
              inst.abort(e.extra);
          }
        /*
        if(activate) // Disable enter/double-click?
          cout << "No action";
        else if(b == 1)
          cout << "Pausing";
        else if(b == 2)
          cout << "Aborting";
        */
      }
    else if(e.status == 2)
      {
        // TODO: All these path and file operations could be out-
        // sourced to an external module. One "repository" module that
        // doesn't know about the rest of the program is the best bet.

        // Construct the install path
        boost::filesystem::path dir = "data";
        dir /= string(e.idname.mb_str());
        dir = get.getPath(dir.string());

        if(b == 1)
          {
            // Button1 == Launching
            if((wxGetOsVersion() & wxOS_WINDOWS) != 0)
              {
                string program = (dir / e.tigInfo.launch).string();

                // Change the working directory before running. Since
                // none of our own code uses relative paths, this
                // shouldn't affect our own operation.
                wxSetWorkingDirectory(wxString(dir.string().c_str(), wxConvUTF8));

                int res = wxExecute(wxString(program.c_str(), wxConvUTF8));
                if(res == -1)
                  cout << "Failed to launch " << program << endl;
              }
            else
              cout << "Launching currently only available on Windows\n";

            /* wxExecute allows a callback for when the program exists
               as well, letting us do things like (optionally)
               disabling downloads while running the program. I assume
               this needs to be handled in a thread-like manner, and
               we also need an intuitive backup plan in case the
               callback is never called.
            */
          }
        else if(b == 2)
          {
            // Button2 == Uninstall

            // Kill all the files
            boost::filesystem::remove_all(dir);

            // Revert status
            data.arr[index].status = 0;
            if(index == select) fixButtons();
            list->RefreshItem(index);

            // Make sure we update the config file
            writeConfig();
          }
      }
  }

  void onGamePage(wxCommandEvent &event)
  {
    if(select < 0 || select >= data.arr.size())
      return;

    wxLaunchDefaultBrowser(wxT("http://tiggit.net/game/") + data.arr[select].urlname);
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
    fixButtons();
  }

  void onListSelect(wxListEvent &event)
  {
    select = event.GetIndex();
    fixButtons();
  }

  // Fix buttons for the current selected item (if any)
  void fixButtons()
  {
    if(select < 0 || select >= data.arr.size())
      {
        b1->Disable();
        b2->Disable();
        b1->SetLabel(wxT("No action"));
        b2->SetLabel(wxT("No action"));
        return;
      }

    int s = data.arr[select].status;

    b1->Enable();
    b2->Enable();

    if(s == 0)
      {
        b1->SetLabel(wxT("Install"));
        b2->SetLabel(wxT("No action"));
        b2->Disable();
      }
    else if(s == 1 || s == 3)
      {
        b1->SetLabel(wxT("Pause"));
        b2->SetLabel(wxT("Abort"));

        // Pausing is not implemented yet
        b1->Disable();
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

struct MyTimer : wxTimer
{
  MyFrame *frame;

  MyTimer(MyFrame *f)
    : frame(f)
  {
    Start(500);
  }

  void Notify()
  {
    frame->tick();
  }
};

class MyApp : public wxApp
{
  MyTimer *time;

public:
  virtual bool OnInit()
  {
    if (!wxApp::OnInit())
      return false;

    SetAppName(wxT("tiggit"));

    // Set up the config directory
    wxString dataDir = wxStandardPaths::Get().GetUserLocalDataDir();
    get.setBase(string(dataDir.mb_str()));

    try
      {
        string version;
        {
          // Do auto update step. This requires us to immediately exit
          // in some cases.
          Updater upd(this);
          if(upd.doAutoUpdate())
            return false;

          version = upd.version;

          upd.setMsg(wxT("Loading game data"));
          updateData(false);
        }

        MyFrame *frame = new MyFrame(wxT("Tiggit - The Indie Game Installer"),
                                     version);
        frame->Show(true);
        time = new MyTimer(frame);
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
