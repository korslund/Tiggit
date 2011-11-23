#define wxUSE_UNICODE 1

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/listctrl.h>
#include <wx/accel.h>
#include <wx/imaglist.h>
#include <wx/notebook.h>

#include <iostream>
#include <assert.h>
#include <set>
#include <vector>
#include <time.h>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "curl_get.hpp"
#include "decodeurl.hpp"
#include "filegetter.hpp"
#include "data_reader.hpp"
#include "thread_get.hpp"
#include "install.hpp"
#include "json_installed.hpp"
#include "auto_update.hpp"

#include "auth.hpp"

using namespace std;

DataList data;
JsonInstalled jinst;
TigListReader tig_reader;

void writeConfig()
{
  jinst.write(data);
}

class ListKeeper
{
  std::vector<int> selection;
  std::vector<int> indices;

  bool setup;
  bool ready;

  /* 0 - title
     1 - date
   */
  int sortBy;
  bool reverse;

  // Current search string
  std::string search;

  struct SortBase
  {
    virtual bool isLess(DataList::Entry &a, DataList::Entry &b) = 0;

    bool operator()(int a, int b)
    {
      return isLess(data.arr[a], data.arr[b]);
    }
  };

  struct TitleSort : SortBase
  {
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
      return boost::algorithm::ilexicographical_compare(a.tigInfo.title,
                                                        b.tigInfo.title);
    }
  };

  struct DateSort : SortBase
  {
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
      return a.add_time > b.add_time;
    }
  };

  void doSort()
  {
    assert(ready && setup);

    if(sortBy == 0)
      {
        if(reverse)
          sort(indices.rbegin(), indices.rend(), TitleSort());
        else
          sort(indices.begin(), indices.end(), TitleSort());
      }
    else
      {
        // Some repeated code, thanks to C++ actually being quite a
        // sucky language.
        if(reverse)
          sort(indices.rbegin(), indices.rend(), DateSort());
        else
          sort(indices.begin(), indices.end(), DateSort());
      }
  }

  void makeSetup()
  {
    if(!setup)
      {
        // Are we searching?
        if(search == "")
          {
            // Nope. Select all elements.
            selection.resize(data.arr.size());
            for(int i=0; i<selection.size(); i++)
              selection[i] = i;
          }
        else
          {
            selection.resize(0);
            selection.reserve(data.arr.size());

            // Add all games that match the search. This is a dumb,
            // slow and useless algorithm, but at current list sizes
            // it's ok. Never optimize until you get hate mail.
            for(int i=0; i<data.arr.size(); i++)
              if(boost::algorithm::icontains(data.arr[i].tigInfo.title, search))
                selection.push_back(i);
          }

        setup = true;
        ready = false;
      }

    assert(setup);
  }

  void makeReady()
  {
    makeSetup();

    if(!ready)
      {
        // Copy the search selection
        indices.resize(selection.size());
        for(int i=0; i<indices.size(); i++)
          indices[i] = selection[i];

        ready = true;

        // Apply current sorting
        doSort();
      }

    assert(indices.size() == selection.size());
    assert(ready && setup);
  }

  void setSort(int type)
  {
    sortBy = type;
    ready = false;
  }

public:

  ListKeeper() : setup(false), ready(false), sortBy(0), reverse(false) {}

  void sortTitle() { setSort(0); }
  void sortDate() { setSort(1); }

  void setReverse(bool rev)
  {
    reverse = rev;
    ready = false;
  }

  void setSearch(const std::string &str)
  {
    search = str;
    setup = false;
  }

  DataList::Entry &get(int index)
  {
    assert(index >= 0 && index < size());
    makeReady();

    return data.arr[indices[index]];
  }

  // Called whenever the source data list changes. It basically means
  // we have to throw everything out.
  void reset() { ready = false; setup = false; }

  int size()
  {
    makeReady();
    return indices.size();
  }
};

ListKeeper lister;

// Update data from all_games.json. If the parameter is true, force
// download. If not, download only if the existing file is too old.
void updateData(bool download)
{
  data.arr.resize(0);
  lister.reset();

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
  else
    // If the file didn't exist, download it.
    download = true;

  try
    {
      if(download)
        {
          // Get the latest list from the net
          string url = "http://tiggit.net/api/all_games.json";
          get.getTo(url, "all_games.json");
        }

      tig_reader.loadData(lstfile, data);
      lister.reset();
      jinst.read(data);
    }
  catch(std::exception &e)
    {
      // Did we already try downloading?
      if(download)
        {
          // Then fail, nothing more to do
          wxString msg(e.what(), wxConvUTF8);
          wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
        }
      else
        // Nope. Try again, this time force a download.
        updateData(true);
    }
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
    col.SetWidth(300);
    InsertColumn(0, col);

    col.SetId(1);
    col.SetText( wxT("Date added") );
    col.SetWidth(115);
    InsertColumn(1, col);

    col.SetId(2);
    col.SetText( wxT("Status") );
    col.SetWidth(235);
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
    SetItemCount(lister.size());
    setSelect(0);

    // Doesn't seem to be needed on Linux. We could remove it there to
    // make it a slightly less flickering experience.
    Refresh();
  }

  wxListItemAttr *OnGetItemAttr(long item) const
  {
    int s = lister.get(item).status;
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

    const DataList::Entry &e = lister.get(item);

    if(column == 0)
      return e.name;

    if(column == 1)
      return e.timeString;

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

// Ask the user an OK/Cancel question.
bool ask(const wxString &question)
{
  return wxMessageBox(question, wxT("Please confirm"),
                      wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
}

#define myID_BUTTON1 21
#define myID_BUTTON2 22
#define myID_GAMEPAGE 24
#define myID_LIST 25
#define myID_SORT_TITLE 41
#define myID_SORT_DATE 42
#define myID_SORT_REVERSE 43
#define myID_SEARCH_BOX 45

struct MyPane : wxPanel
{
  wxButton *b1, *b2;
  MyList *list;
  int select;
  time_t last_launch;

  MyPane(wxWindow *parent)
    : wxPanel(parent), select(-1), last_launch(0)
  {
    list = new MyList(this, myID_LIST);

    wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);

    b1 = new wxButton(this, myID_BUTTON1, wxT("No action"));
    rightPane->Add(b1, 0, wxBOTTOM | wxRIGHT, 10);

    b2 = new wxButton(this, myID_BUTTON2, wxT("No action"));
    rightPane->Add(b2, 0, wxBOTTOM | wxRIGHT, 10);

    rightPane->Add(new wxRadioButton(this, myID_SORT_TITLE, wxT("Sort by title"),
                                     wxDefaultPosition, wxDefaultSize, wxRB_GROUP),
                   0, wxRIGHT, 10);
    rightPane->Add(new wxRadioButton(this, myID_SORT_DATE, wxT("Sort by date")),
                   0, wxRIGHT, 10);
    rightPane->Add(new wxCheckBox(this, myID_SORT_REVERSE, wxT("Reverse order")),
                   0, wxRIGHT, 10);

    rightPane->Add(new wxStaticText(this, wxID_ANY, wxT("\nSearch:")), 0, wxRIGHT, 0);
    rightPane->Add(new wxTextCtrl(this, myID_SEARCH_BOX),
                   0, wxRIGHT, 10);

    wxBoxSizer *panes = new wxBoxSizer(wxHORIZONTAL);
    panes->Add(list, 1, wxGROW);
    panes->Add(rightPane, 0, wxGROW | wxLEFT | wxTOP, 18);

    SetSizer(panes);

    // Add delete = 2nd button accelerator
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, myID_BUTTON2);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    Connect(myID_GAMEPAGE, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyPane::onGamePage));

    Connect(myID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyPane::onButton));
    Connect(myID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyPane::onButton));

    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(MyPane::onListSelect));
    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(MyPane::onListDeselect));

    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
            wxListEventHandler(MyPane::onListRightClick));
    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
            wxListEventHandler(MyPane::onListActivate));

    Connect(myID_SORT_TITLE, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
            wxCommandEventHandler(MyPane::onSortChange));
    Connect(myID_SORT_DATE, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
            wxCommandEventHandler(MyPane::onSortChange));
    Connect(myID_SORT_REVERSE, wxEVT_COMMAND_CHECKBOX_CLICKED,
            wxCommandEventHandler(MyPane::onSortChange));

    Connect(myID_SEARCH_BOX, wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler(MyPane::onSearch));

    list->update();
    takeFocus();
  }

  void takeFocus()
  {
    list->SetFocus();
  }

  // Fix buttons for the current selected item (if any)
  void fixButtons()
  {
    if(select < 0 || select >= lister.size())
      {
        b1->Disable();
        b2->Disable();
        b1->SetLabel(wxT("No action"));
        b2->SetLabel(wxT("No action"));
        return;
      }

    int s = lister.get(select).status;

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

  void onSearch(wxCommandEvent &event)
  {
    string src = string(event.GetString().mb_str());
    lister.setSearch(src);
    list->update();
  }

  // I'm short on change, can you help a fella out?
  void onSortChange(wxCommandEvent &event)
  {
    int id = event.GetId();

    if(id == myID_SORT_REVERSE)
      lister.setReverse(event.IsChecked());

    else if(id == myID_SORT_TITLE)
      lister.sortTitle();

    else if(id == myID_SORT_DATE)
      lister.sortDate();

    else assert(0);

    list->update();
  }

  // Create a nice size string
  static wxString sizify(int size)
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
    DataList::Entry &e = lister.get(index);

    // If this is the current item, make sure the buttons are set
    // correctly.
    if(index == select) fixButtons();

    // Neither downloading or unpacking
    if(e.status != 1 && e.status != 3)
      {
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

  void startDownload(int index)
  {
    DataList::Entry &e = lister.get(index);

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

    // Are we allowed to download games?
    if(auth.isAdmin())
      {
        wxMessageBox(wxT("Administrators may not install games"),
                     wxT("Error"), wxOK | wxICON_ERROR);
        return;
      }

    {
      // Update the .tig info
      DataList::TigInfo ti;

      // TODO: In future versions, this will be outsourced to a worker
      // thread to keep from blocking the app.
      if(tig_reader.decodeTigUrl
         (string(e.urlname.mb_str()), // url-name
          string(e.tigurl.mb_str()),  // url to tigfile
          ti,                         // where to store result
          false))                     // ONLY use cache if fetch fails
        {
          // Copy new data if the fetch was successful
          if(ti.launch != "")
            e.tigInfo = ti;
        }
    }

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

    // Finally update this entry now.
    handleDownload(index);
  }

  void doAction(int index, int b)
  {
    if(index < 0 || index >= lister.size())
      return;

    const DataList::Entry &e = lister.get(index);

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

            // Check if we double-tapped
            time_t now = time(0);

            // Did we just start something?
            if(difftime(now,last_launch) <= 10)
              {
                if(!ask(wxT("You just started a game. Are you sure you want to launch again?\n\nSome games may take a few seconds to start.")))
                  return;
              }

            // Program to launch
            boost::filesystem::path program = dir / e.tigInfo.launch;

            if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
              cout << "WARNING: Launching will probably not work on your platform.\n";

            // Change the working directory before running. Since none
            // of our own code uses relative paths, this should not
            // affect our own operation.
            boost::filesystem::path workDir;

            // Calculate full working directory path
            if(e.tigInfo.subdir != "")
              // Use user-requested sub-directory
              workDir = dir / e.tigInfo.subdir;
            else
              // Use base path of the executable
              workDir = program.branch_path();

            wxSetWorkingDirectory(wxString(workDir.string().c_str(), wxConvUTF8));

            int res = wxExecute(wxString(program.string().c_str(), wxConvUTF8));
            if(res == -1)
              cout << "Failed to launch " << program << endl;

            // Update the last launch time
            last_launch = now;

            /* wxExecute allows a callback for when the program exists
               as well, letting us do things like (optionally)
               disabling downloads while running the program. I assume
               this needs to be handled in a thread-like reentrant
               manner, and we also need an intuitive backup plan in
               case the callback is never called, or if it's called
               several times.
            */
          }
        else if(b == 2)
          {
            // Button2 == Uninstall

            // Are you sure?
            if(!ask(wxT("Are you sure you want to uninstall ") + e.name +
                    wxT("? All savegames and configuration will be lost.")))
              return;;

	    try
	      {
                // First, make sure we are not killing our own working
                // directory.
                wxSetWorkingDirectory(wxString(get.base.string().c_str(), wxConvUTF8));

                // Kill all the files
		boost::filesystem::remove_all(dir);

		// Revert status
		lister.get(index).status = 0;
		if(index == select) fixButtons();
		list->RefreshItem(index);

		// Make sure we update the config file
		writeConfig();
	      }
	    catch(exception &ex)
	      {
		wxString msg = wxT("Could not uninstall ") + e.name +
		  wxT(": ") + wxString(string(ex.what()).c_str(), wxConvUTF8)
		  + wxT("\nPerhaps the game is still running?");
		wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
	      }
          }
      }
  }

  void onGamePage(wxCommandEvent &event)
  {
    if(select < 0 || select >= lister.size())
      return;

    wxLaunchDefaultBrowser(wxT("http://tiggit.net/game/") + lister.get(select).urlname);
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

  void onListRightClick(wxListEvent &event)
  {
    cout << "Context menu on " << event.GetIndex() << endl;
  }

  void onListActivate(wxListEvent &event)
  {
    doAction(event.GetIndex(), 3);
  }
};

#define myID_MENU_REFRESH 30
#define myID_GOLEFT 31
#define myID_GORIGHT 32

class MyFrame : public wxFrame
{
  std::string version;
  MyPane *pane;
  wxNotebook *book;

public:
  MyFrame(const wxString& title, const std::string &ver)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(845, 700)),
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

    // Without this the menubar doesn't appear (in GTK on Linux) until
    // you move the mouse. Probably some weird bug.
    SendSizeEvent();

    wxPanel *panel = new wxPanel(this);
    book = new wxNotebook(panel, wxID_ANY);
    pane = new MyPane(book);

    book->AddPage(pane, wxT("All"), true);
    book->AddPage(new wxStaticText(book, wxID_ANY, wxT("This is a test")),
                  wxT("Test"));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(book, 1, wxGROW | wxALL, 10);
    mainSizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Mouse: double-click to install / play\nKeyboard: arrow keys + enter, delete")), 0, wxALL, 10);

    panel->SetSizer(mainSizer);

    // Add left/right => control tabs
    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_NORMAL, WXK_LEFT, myID_GOLEFT);
    entries[1].Set(wxACCEL_NORMAL, WXK_RIGHT, myID_GORIGHT);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);

    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onAbout));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onExit));
    Connect(myID_MENU_REFRESH, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onRefresh));

    Connect(myID_GOLEFT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyFrame::onLeftRight));
    Connect(myID_GORIGHT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(MyFrame::onLeftRight));

    pane->takeFocus();
  }

  void onLeftRight(wxCommandEvent &event)
  {
    if(event.GetId() == myID_GOLEFT)
      cout << "Going left\n";
    else
      cout << "Going right\n";

    // TODO: Use book->AdvanceSelection here

    // TODO: Focus control. We should call pane->takeFocus() on the
    // appropriate pane. It doesn't do this itself, because that
    // disturbs normal keyboard usage of the tabs. It should only
    // steal focus when using these arrow key accelerators.
  }

  void onRefresh(wxCommandEvent &event)
  {
    updateData(true);

    // TODO: Should update ALL panes
    pane->list->update();
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

  // Called regularly by an external timer, used to update
  // thread-dependent data
  void tick()
  {
    // This too has to be sent to all panes. Well actually, I'm not
    // sure. We have mixed up display stuff AND actual thread stuff in
    // here. BUT I think the status updates only happen once, we are
    // after all refering to the same Element, so I think it won't
    // cause any trouble to update all of them. This entire thing will
    // probably change a lot after we've rewritten the thread system
    // though.

    for(int i=0; i<lister.size(); i++)
      pane->handleDownload(i);
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

  void test(const std::string &inp)
  {
    cout << inp << "  =>  " << tig_reader.URL(inp) << endl;
  }

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
        }

        conf.load(get.base);
        auth.load();

        updateData(conf.updateList);

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
