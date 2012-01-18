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
#include <time.h>

#include "image_viewer.hpp"
#include "curl_get.hpp"
#include "decodeurl.hpp"
#include "filegetter.hpp"
#include "data_reader.hpp"
#include "thread_get.hpp"
#include "install.hpp"
#include "json_installed.hpp"
#include "auto_update.hpp"
#include "listkeeper.hpp"
#include "auth.hpp"

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
  else
    // If the file didn't exist, download it.
    download = true;

  if(download)
    {
      try
        {
          // Get the latest list from the net
          string url = "http://tiggit.net/api/all_games.json";
          get.getTo(url, "all_games.json");
        }
      catch(...) {}
    }

  try
    {
      tig_reader.loadData(lstfile, data);
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

struct ColumnHandler
{
  virtual wxString getText(const DataList::Entry &e) = 0;
};

class MyList : public wxListCtrl
{
  wxListItemAttr green, orange;
  wxImageList images;
  ListKeeper &lister;

  std::vector<ColumnHandler*> colHands;

  // Number of columns
  int colNum;

public:
  MyList(wxWindow *parent, int ID, ListKeeper &lst)
    : wxListCtrl(parent, ID, wxDefaultPosition, wxDefaultSize,
                 wxBORDER_SUNKEN | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL),
      lister(lst), colNum(0)
  {
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

  void addColumn(const wxString &name, int width, ColumnHandler *ch)
  {
    colHands.push_back(ch);

    wxListItem col;
    col.SetId(colNum);
    col.SetText(name);
    col.SetWidth(width);
    InsertColumn(colNum, col);
    colNum++;
  }

  void setSelect(int index)
  {
    SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  }

  // Refresh the list based on new data. If stay=true, then try to
  // stay at or near the old list position.
  void update(bool stay=false)
  {
    int newItem = 0;

    if(stay)
      {
        newItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if(newItem < 0) newItem = 0;
      }

    SetItemCount(lister.size());
    if(newItem >= lister.size())
      newItem = lister.size() - 1;
    setSelect(newItem);

    // Doesn't seem to be needed on Linux. We could remove it there to
    // make it a slightly less flickering experience.
    Refresh();
  }

  wxListItemAttr *OnGetItemAttr(long item) const
  {
    if(item < 0 || item >= lister.size())
      return NULL;

    int s = lister.get(item).status;
    if(s == 0) return NULL;
    //if(s == 2) return (wxListItemAttr*)&green;
    if(s == 2) return NULL; // Disable green
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
    if(column < 0 || column >= colHands.size() ||
       item < 0 || item >= lister.size())
      return wxT("No info");

    assert(column >= 0 && column < colHands.size());
    ColumnHandler *h = colHands[column];
    assert(h);

    return h->getText(lister.get(item));
  }
};

// Ask the user an OK/Cancel question.
bool ask(const wxString &question)
{
  return wxMessageBox(question, wxT("Please confirm"),
                      wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
}

struct TabBase : wxPanel
{
  wxNotebook *book;

  // Current tab placement in the wxNotebook. MUST be set to -1 when
  // not inserted!
  int tabNum;

  TabBase(wxNotebook *parent)
    : wxPanel(parent), book(parent), tabNum(-1)
  {}

  // Called when the tab is selected, letting the tab to direct focus
  // to a sub-element.
  virtual void takeFocus() = 0;

  // Called regularly to update information. Currently called for all
  // tabs, will soon only be called for the visible tab. Default is to
  // do nothing.
  virtual void tick() {}

  // Called whenever the root data table has changed, basically
  // instructing a complete reset on everything that depends on the
  // game database.
  virtual void dataChanged() = 0;

  // Insert this tab into the wxNotebook, if it has any content
  virtual void insertMe()
  {
    assert(book);
    book->AddPage(this, wxT(""));
    tabNum = book->GetPageCount() - 1;
  }

  // Select this tab, if it is inserted into the book. Returns true if
  // selection was successful.
  bool selectMe()
  {
    assert(book);
    if(tabNum >= 0)
      {
        book->ChangeSelection(tabNum);
        return true;
      }
    return false;
  }
};

// Not currently in use, will be expanded later
struct NewsTab : TabBase
{
  NewsTab(wxNotebook *parent)
    : TabBase(parent)
  {
    new wxStaticText(this, wxID_ANY, wxT("This is a test tab"));
  }

  void takeFocus()
  {
    cout << "Test tab got focus!\n";
  }

  void dataChanged()
  {
    cout << "The world is changing.\n";
  }

  // Currently doesn't insert itself at all
  void insertMe() { tabNum = -1; }
};

#define myID_BUTTON1 21
#define myID_BUTTON2 22
#define myID_SUPPORT 23
#define myID_GAMEPAGE 24
#define myID_LIST 25
#define myID_TEXTVIEW 27
#define myID_SCREENSHOT 28
#define myID_SORT_TITLE 41
#define myID_SORT_DATE 42
#define myID_SORT_REVERSE 43
#define myID_SEARCH_BOX 45

struct TitleCol : ColumnHandler
{
  wxString getText(const DataList::Entry &e)
  {
    return e.name;
  }
};

struct AddDateCol : ColumnHandler
{
  wxString getText(const DataList::Entry &e)
  {
    return e.timeString;
  }
};

struct StatusCol : ColumnHandler
{
  wxString textNotInst, textReady;

  StatusCol()
  {
    textNotInst = wxT("Not installed");
    textReady = wxT("Ready to play");
  }

  wxString getText(const DataList::Entry &e)
  {
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

struct SortOptions
{
  virtual void addSortOptions(wxWindow *parent, wxBoxSizer *pane) const = 0;
};

// Callback to update all lists when an object moves. I don't like
// this AT ALL, and it should be done using wx events instead, but fix
// it later.
struct StatusNotify
{
  virtual void onStatusChanged() = 0;
  virtual void switchToInstalled() = 0;
};

struct ListTab : TabBase
{
  wxButton *b1, *b2, *supportButton;
  MyList *list;
  wxTextCtrl *textView;
  ImageViewer *screenshot;
  int select;
  time_t last_launch;
  ListKeeper lister;

  wxString tabName;
  StatusNotify *stat;

  ListTab(wxNotebook *parent, const wxString &name, int listType,
          StatusNotify *s, const SortOptions *sop = NULL)
    : TabBase(parent), select(-1), last_launch(0),
      lister(data, listType), tabName(name), stat(s)
  {
    list = new MyList(this, myID_LIST, lister);

    wxBoxSizer *searchBox = new wxBoxSizer(wxHORIZONTAL);
    searchBox->Add(new wxStaticText(this, wxID_ANY, wxT("Search:")), 0);
    searchBox->Add(new wxTextCtrl(this, myID_SEARCH_BOX, wxT(""), wxDefaultPosition,
                                  wxSize(260,22)),1, wxGROW);

    /*
    wxBoxSizer *sortBox = new wxBoxSizer(wxHORIZONTAL);
    if(sop)
      sop->addSortOptions(this, sortBox);
    */

    wxBoxSizer *leftPane = new wxBoxSizer(wxVERTICAL);
    leftPane->Add(list, 1, wxGROW | wxRIGHT | wxBOTTOM, 10);
    leftPane->Add(searchBox, 0, wxBOTTOM, 2);
    /*
    leftPane->Add(sortBox, 0);
    leftPane->Add(new wxCheckBox(this, myID_SORT_REVERSE, wxT("Reverse order")),
                  0);
    */
    leftPane->Add(new wxStaticText(this, wxID_ANY, wxString(wxT("Mouse: double-click to ")) +
                                   ((listType == ListKeeper::SL_INSTALL)?
                                    wxT("play"):wxT("install")) +
                                   wxT("\nKeyboard: arrow keys + enter, delete")),
                  0, wxLEFT | wxBOTTOM, 4);

    textView = new wxTextCtrl
      (this, myID_TEXTVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
       wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

    screenshot = new ImageViewer(this, myID_SCREENSHOT, wxDefaultPosition,
                                  wxSize(300,200));

    b1 = new wxButton(this, myID_BUTTON1, wxT("No action"));
    b2 = new wxButton(this, myID_BUTTON2, wxT("No action"));

    supportButton = new wxButton(this, myID_SUPPORT, wxT("Support Game (Donate)"));

    wxBoxSizer *buttonBar = new wxBoxSizer(wxHORIZONTAL);
    buttonBar->Add(b1, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);
    buttonBar->Add(b2, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);
    buttonBar->Add(new wxButton(this, myID_GAMEPAGE, wxT("Game Website")),
                   0, wxTOP | wxBOTTOM, 3);

    wxBoxSizer *buttonHolder = new wxBoxSizer(wxVERTICAL);
    buttonHolder->Add(supportButton, 0, wxALIGN_RIGHT);
    buttonHolder->Add(buttonBar, 0);

    wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);
    rightPane->Add(textView, 1, wxGROW | wxALL, 5);
    rightPane->Add(screenshot, 0, wxLEFT | wxTOP, 5);
    rightPane->Add(buttonHolder, 0);
    /*
    rightPane->Add(supportButton, 0, wxALIGN_RIGHT);
    rightPane->Add(buttonBar, 0);
    */

    wxBoxSizer *panes = new wxBoxSizer(wxHORIZONTAL);
    panes->Add(leftPane, 100, wxGROW);
    panes->Add(rightPane, 60, wxGROW);

    SetSizer(panes);

    // Add delete = 2nd button accelerator
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, myID_BUTTON2);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    Connect(myID_GAMEPAGE, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(ListTab::onGamePage));

    Connect(myID_SUPPORT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(ListTab::onGamePage));

    Connect(myID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(ListTab::onButton));
    Connect(myID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(ListTab::onButton));

    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_SELECTED,
            wxListEventHandler(ListTab::onListSelect));
    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_DESELECTED,
            wxListEventHandler(ListTab::onListDeselect));

    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
            wxListEventHandler(ListTab::onListRightClick));
    Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
            wxListEventHandler(ListTab::onListActivate));

    Connect(myID_SORT_TITLE, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
            wxCommandEventHandler(ListTab::onSortChange));
    Connect(myID_SORT_DATE, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
            wxCommandEventHandler(ListTab::onSortChange));
    Connect(myID_SORT_REVERSE, wxEVT_COMMAND_CHECKBOX_CLICKED,
            wxCommandEventHandler(ListTab::onSortChange));

    Connect(myID_SEARCH_BOX, wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler(ListTab::onSearch));

    list->update();
    takeFocus();
  }

  void insertMe()
  {
    TabBase::insertMe();
    wxString name = tabName + wxString::Format(wxT(" (%d)"), lister.baseSize());
    book->SetPageText(tabNum, name);
  }

  void takeFocus()
  {
    list->SetFocus();
    updateSelection();
  }

  // Called on "soft" list changes, such as search queries. If
  // stay=true, try to stay at the same position.
  void listHasChanged(bool stay=false)
  {
    list->update(stay);
    updateSelection();
  }

  void updateGameInfo()
  {
    if(select < 0 || select >= lister.size())
      {
        textView->Clear();
        //screenshot->clear();
        return;
      }

    const DataList::Entry &e = lister.get(select);

    // Update the text view
    textView->ChangeValue(wxString(e.tigInfo.desc.c_str(), wxConvUTF8));

    // And the screenshot
    //screenshot->loadImage("filename.jpg");
  }

  // Called whenever there is a chance that a new game has been
  // selected. This updates the available action buttons as well as
  // the displayed game info to match the current selection.
  void updateSelection()
  {
    fixButtons();
    updateGameInfo();
  }

  // Fix buttons for the current selected item (if any)
  void fixButtons()
  {
    if(select < 0 || select >= lister.size())
      {
        b1->Disable();
        b2->Disable();
        supportButton->Disable();
        b1->SetLabel(wxT("No action"));
        b2->SetLabel(wxT("No action"));
        return;
      }

    const DataList::Entry &e = lister.get(select);

    if(e.tigInfo.hasPaypal)
      supportButton->Enable();
    else
      supportButton->Disable();

    b1->Enable();
    b2->Enable();

    int s = e.status;
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
    listHasChanged();
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

    listHasChanged();
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
            statusChanged();
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
                statusChanged();
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

    /* Update lists and moved to the Installed tab.

       NOTE: this will switch tabs even if download immediately
       fails. This isn't entirely optimal, but is rare enough that it
       doesn't matter.
     */
    statusChanged(true);
  }

  /* Called whenever an item switches lists. This is a cludge and it
     should die. But we will clean up all this mess later (or so I
     keep telling myself), and then everything will be much dandier.

     Set newInst to true if we should switch to the "Installed" tab.
  */
  void statusChanged(bool newInst=false)
  {
    stat->onStatusChanged();
    if(newInst) stat->switchToInstalled();
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
              return;

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

                statusChanged();
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

    const DataList::Entry &e = lister.get(select);

    wxString url;

    if(event.GetId() == myID_GAMEPAGE)
      {
        if(e.tigInfo.homepage != "")
          // Launch game homepage if it exists
          url = wxString(e.tigInfo.homepage.c_str(), wxConvUTF8);
        else
          // Otherwise, just redirect to the tiggit page
          url = wxT("http://tiggit.net/game/") + e.urlname;
      }

    else if(event.GetId() == myID_SUPPORT)
      {
        // Redirect to our special support page
        url = wxT("http://tiggit.net/game/") + e.urlname + wxT("&donate");
      }

    wxLaunchDefaultBrowser(url);
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
    updateSelection();
  }

  void onListSelect(wxListEvent &event)
  {
    select = event.GetIndex();
    updateSelection();
  }

  void onListRightClick(wxListEvent &event)
  {
    cout << "Context menu on " << event.GetIndex() << endl;
  }

  void onListActivate(wxListEvent &event)
  {
    doAction(event.GetIndex(), 3);
  }

  void dataChanged()
  {
    lister.reset();
    listHasChanged(true);
  }
};

/* A cludge to work around C++'s crappy handling of virtual
   functions. This function should be virtual in ListTab, but then the
   constructor doesn't call the right one.
 */
struct GameSortOptions : SortOptions
{
  void addSortOptions(wxWindow *parent, wxBoxSizer *pane) const
  {
    pane->Add(new wxRadioButton(parent, myID_SORT_TITLE, wxT("Sort by title"),
                                wxDefaultPosition, wxDefaultSize, wxRB_GROUP),
              0, wxRIGHT, 10);
    pane->Add(new wxRadioButton(parent, myID_SORT_DATE, wxT("Sort by date")),
              0, wxRIGHT, 10);
  }
};

// List tab that only displays itself when non-empty
struct ListTabNonEmpty : ListTab
{
  ListTabNonEmpty(wxNotebook *parent, const wxString &name, int listType,
                  StatusNotify *s, const SortOptions *sop = NULL)
    : ListTab(parent, name, listType, s, sop)
  {}

  void insertMe()
  {
    tabNum = -1;

    // Only show the "New" tab if there are new games to show.
    if(lister.baseSize() > 0)
      ListTab::insertMe();
  }
};

// Lists newly added games
struct NewGameListTab : ListTabNonEmpty
{
  NewGameListTab(wxNotebook *parent, StatusNotify *s)
    : ListTabNonEmpty(parent, wxT("New"), ListKeeper::SL_NEW, s, new GameSortOptions)
  {
    list->addColumn(wxT("Name"), 380, new TitleCol);
    //list->addColumn(wxT("Date added"), 160, new AddDateCol);
  }
};

// Lists freeware games
struct FreewareListTab : ListTab
{
  FreewareListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Browse"), ListKeeper::SL_FREEWARE, s, new GameSortOptions)
  {
    list->addColumn(wxT("Name"), 380, new TitleCol);
    //list->addColumn(wxT("Date added"), 160, new AddDateCol);
  }

};

struct DemoListTab : ListTabNonEmpty
{
  DemoListTab(wxNotebook *parent, StatusNotify *s)
    : ListTabNonEmpty(parent, wxT("Demos"), ListKeeper::SL_DEMOS, s, new GameSortOptions)
  {
    list->addColumn(wxT("Name"), 380, new TitleCol);
    //list->addColumn(wxT("Date added"), 160, new AddDateCol);
  }
};

struct InstalledListTab : ListTab
{
  InstalledListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Installed"), ListKeeper::SL_INSTALL, s)
  {
    list->addColumn(wxT("Name"), 300, new TitleCol);
    list->addColumn(wxT("Status"), 170, new StatusCol);
  }

  void tick()
  {
    for(int i=0; i<lister.size(); i++)
      handleDownload(i);
  }
};

#define myID_MENU_REFRESH 30
#define myID_GOLEFT 31
#define myID_GORIGHT 32
#define myID_BOOK 33

class MyFrame : public wxFrame, public StatusNotify
{
  std::string version;
  wxNotebook *book;

  NewGameListTab *newTab;
  FreewareListTab *freewareTab;
  DemoListTab *demoTab;
  InstalledListTab *installedTab;
  NewsTab *newsTab;

public:
  MyFrame(const wxString& title, const std::string &ver)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(850, 700)),
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
    book = new wxNotebook(panel, myID_BOOK);

    // Set up the tabs. They are inserted further down through
    // setupTabs()
    newTab = new NewGameListTab(book, this);
    freewareTab = new FreewareListTab(book, this);
    demoTab = new DemoListTab(book, this);
    installedTab = new InstalledListTab(book, this);
    newsTab = new NewsTab(book);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(book, 1, wxGROW | wxALL, 10);

    panel->SetSizer(mainSizer);

    /* Add left/right => control tabs

       This works on Linux but unfortunately not on Windows, and I
       don't know why.
     */
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

    setTabFocus();
    setupTabs();
  }

  TabBase *getTab(int i)
  {
    assert(book);
    TabBase *res = dynamic_cast<TabBase*>(book->GetPage(i));
    assert(res);
    return res;
  }

  /* Get current tab. Note: Do NOT use this in tab selection event
     handling! Because it may refer to EITHER the new or the old tab,
     depending on platform. Use wxNotebookEvent::GetSelection instead.

     May return NULL if no tab is selected.
   */
  TabBase *getCurrentTab()
  {
    int sel = book->GetSelection();
    if(sel >= 0) return getTab(sel);
    return NULL;
  }

  void setTabFocus()
  {
    TabBase *t = getCurrentTab();
    if(t) t->takeFocus();
  }

  void setupTabs()
  {
    // Remember which tab was selected
    TabBase *sel = getCurrentTab();

    // Remove all tabs
    for(int i=book->GetPageCount()-1; i >= 0; i--)
      book->RemovePage(i);

    // Re-add them, if they want to be re-added
    newTab->insertMe();
    freewareTab->insertMe();
    demoTab->insertMe();
    installedTab->insertMe();
    newsTab->insertMe();

    // Re-select the previously selected tab, if it exists
    if((sel == NULL) || !sel->selectMe())
      {
        // No previously selected tab available.

        // Check if there are installed games, and if so, start by
        // selecting the Installed tab.
        if(installedTab->lister.baseSize() != 0)
          installedTab->selectMe();
        else
          freewareTab->selectMe();
      }
  }

  void onLeftRight(wxCommandEvent &event)
  {
    book->AdvanceSelection(event.GetId() == myID_GORIGHT);
    setTabFocus();
  }

  // "Event" created internally when a list element moves from one
  // list to another. This is called as a virtual function from
  // StatusNotify.
  void onStatusChanged()
  {
    // Notify all tabs that data has changed, but keep current position.
    for(int i=0; i<book->GetPageCount(); i++)
      getTab(i)->dataChanged();

    // Update tab status and titles
    setupTabs();
  }

  // Switches the selected tab to the "Installed" tab. This is a
  // virtual function from StatusNotify.
  void switchToInstalled()
  {
    installedTab->selectMe();
    setTabFocus();
  }

  void onRefresh(wxCommandEvent &event)
  {
    updateData(true);

    // Notify all tabs that data has changed
    for(int i=0; i<book->GetPageCount(); i++)
      getTab(i)->dataChanged();

    setupTabs();
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
    /* TODO: We have mixed up display stuff AND actual thread handling
       stuff in tick, so at the moment we have to update all tabs.

       After we've rewritten the thread system to handle itself more
       autonomously though, we'll only need to update the displayed
       tab since the ticks will be cosmetic.
    */

    /*
    TabBase *t = getCurrentTab();
    if(t) t->tick();
    */
    for(int i=0; i<book->GetPageCount(); i++)
      getTab(i)->tick();
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

        wxInitAllImageHandlers();

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
