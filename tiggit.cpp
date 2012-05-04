#define wxUSE_UNICODE 1

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/accel.h>
#include <wx/imaglist.h>
#include <wx/cmdline.h>
#include <wx/notebook.h>
#include <wx/stdpaths.h>

#include <iostream>
#include <assert.h>
#include <time.h>

#include "image_viewer.hpp"
#include "decodeurl.hpp"
#include "repo.hpp"
#include "data_reader.hpp"
#include "downloadjob.hpp"
#include "zipjob.hpp"
#include "jobqueue.hpp"
#include "json_installed.hpp"
#include "auto_update.hpp"
#include "listkeeper.hpp"
#include "auth.hpp"
#include "json_rated.hpp"
#include "gameinfo.hpp"
#include "cache_fetcher.hpp"
#include "tag_sorter.hpp"
#include "tabbase.hpp"
#include "newstab.hpp"

using namespace std;

DataList data;
JsonInstalled jinst;
TigListReader tig_reader;
TagSorter tagSorter;

// Temporary hack
bool gHasDemos = false;

// Ask the user an OK/Cancel question.
bool ask(const wxString &question)
{
  return wxMessageBox(question, wxT("Please confirm"),
                      wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
}

// Display an error message box
void errorBox(const wxString &msg)
{
  wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
}

// Update data from all_games.json. If the parameter is true, force
// download. If not, download only if the existing file is too old.
void updateData(bool download)
{
  // This is bad and kinda leaks memory like hell (because of all the
  // GameInfo instances.) But refreshes are kinda not an integral part
  // of the application at the moment, so just ignore it for now.
  data.arr.resize(0);

  string lstfile = get.getPath("all_games.json");

  // Do we check file age?
  if(!download && boost::filesystem::exists(lstfile))
    {
      // Yup, check file age
      time_t ft = boost::filesystem::last_write_time(lstfile);
      time_t now = time(0);

      // update game list once an hour
      if(difftime(now,ft) > 60*60*1)
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
      tagSorter.process(data);
    }
  catch(std::exception &e)
    {
      // Did we already try downloading?
      if(download)
        {
          // Then fail, nothing more to do
          wxString msg(e.what(), wxConvUTF8);

          // Make sure we delete the file - no point in keeping an
          // invalid cache.
          boost::filesystem::remove(lstfile);

          errorBox(msg);
        }
      else
        // Nope. Try again, this time force a download.
        updateData(true);
    }
}

struct ColumnHandler
{
  virtual wxString getText(GameInfo &e) = 0;
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

    GameInfo &g = GameInfo::conv(lister.get(item));
    if(g.isNone()) return NULL;
    if(g.isInstalled()) return NULL;
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

    return h->getText(GameInfo::conv(lister.get(item)));
  }
};

#define myID_BUTTON1 21
#define myID_BUTTON2 22
#define myID_SUPPORT 23
#define myID_GAMEPAGE 24
#define myID_LIST 25
#define myID_RATE 26
#define myID_TEXTVIEW 27
#define myID_SCREENSHOT 28
#define myID_SORT_TITLE 41
#define myID_SORT_DATE 42
#define myID_SORT_REVERSE 43
#define myID_SORT_RATING 44
#define myID_SEARCH_BOX 45

#define myID_TIGGIT_PAGE 20010
#define myID_OPEN_LOCATION 20011
#define myID_REFRESH_ITEM 20012
#define myID_TAGS 20020

struct TitleCol : ColumnHandler
{
  wxString getText(GameInfo &e)
  {
    return e.name;
  }
};

struct AddDateCol : ColumnHandler
{
  wxString getText(GameInfo &e)
  {
    return e.timeString;
  }
};

struct TypeCol : ColumnHandler
{
  wxString getText(GameInfo &e)
  {
    if(e.entry.tigInfo.isDemo)
      return wxT("demo");
    return wxT("freeware");
  }
};

struct RatingCol : ColumnHandler
{
  wxString getText(GameInfo &e)
  {
    return e.rating;
  }
};

struct DownloadsCol : ColumnHandler
{
  wxString getText(GameInfo &e)
  {
    return e.dlCount;
  }
};

struct PriceCol : ColumnHandler
{
  wxString getText(GameInfo &e)
  {
    return e.price;
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

  wxString getText(GameInfo &g)
  {
    if(g.isNone())
      return textNotInst;

    if(g.isInstalled())
      return textReady;

    if(g.isWorking())
      return g.getStatus();

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
  virtual void onDataChanged() = 0;
  virtual void switchToInstalled() = 0;
};

struct ListTab : TabBase, ScreenshotCallback
{
  wxButton *b1, *b2, *supportButton;
  MyList *list;
  wxTextCtrl *textView;
  ImageViewer *screenshot, *ad_img;
  int select;
  time_t last_launch;
  ListKeeper lister;
  wxListBox *tags;
  wxString tabName;
  StatusNotify *stat;
  wxChoice *rateBox;
  wxStaticText *rateText;
  wxString rateString[7];

  std::vector<TagSorter::Entry> taglist;

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

    wxBoxSizer *centerPane = new wxBoxSizer(wxVERTICAL);
    centerPane->Add(list, 1, wxGROW | wxRIGHT | wxBOTTOM, 10);
    centerPane->Add(searchBox, 0, wxBOTTOM, 2);
    /*
    centerPane->Add(sortBox, 0);
    centerPane->Add(new wxCheckBox(this, myID_SORT_REVERSE, wxT("Reverse order")),
                  0);
    */
    centerPane->Add(new wxStaticText(this, wxID_ANY, wxString(wxT("Mouse: double-click to ")) +
                                   ((listType == ListKeeper::SL_INSTALL)?
                                    wxT("play"):wxT("install")) +
                                   wxT("\nKeyboard: arrow keys + enter, delete")),
                  0, wxLEFT | wxBOTTOM, 4);

    textView = new wxTextCtrl
      (this, myID_TEXTVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
       wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

    screenshot = new ImageViewer(this, myID_SCREENSHOT, wxDefaultPosition,
                                 wxSize(300,260));

    b1 = new wxButton(this, myID_BUTTON1, wxT("No action"));
    b2 = new wxButton(this, myID_BUTTON2, wxT("No action"));

    supportButton = new wxButton(this, myID_SUPPORT, wxT("Support Game (donate)"));

    wxBoxSizer *buttonBar = new wxBoxSizer(wxHORIZONTAL);
    buttonBar->Add(b1, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);
    buttonBar->Add(b2, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);
    buttonBar->Add(new wxButton(this, myID_GAMEPAGE, wxT("Game Website")),
                   0, wxTOP | wxBOTTOM, 3);

    wxBoxSizer *buttonHolder = new wxBoxSizer(wxVERTICAL);
    buttonHolder->Add(supportButton, 0, wxALIGN_RIGHT);
    buttonHolder->Add(buttonBar, 0);

    wxString choices[7];
    choices[0] = wxT("Rate this game");
    choices[1] = wxT("5: Awesome!");
    choices[2] = wxT("4: Very Good");
    choices[3] = wxT("3: It's OK");
    choices[4] = wxT("2: Meh");
    choices[5] = wxT("1: Very Bad");
    choices[6] = wxT("0: Unplayable");

    rateString[0] = wxT("Your rating: not rated");
    rateString[1] = wxT("Your rating: 0 (unplayable)");
    rateString[2] = wxT("Your rating: 1 (very bad)");
    rateString[3] = wxT("Your rating: 2 (meh)");
    rateString[4] = wxT("Your rating: 3 (ok)");
    rateString[5] = wxT("Your rating: 4 (very good)");
    rateString[6] = wxT("Your rating: 5 (awesome)");

    rateBox = new wxChoice(this, myID_RATE, wxDefaultPosition, wxDefaultSize,
                           7, choices);
    rateText = new wxStaticText(this, wxID_ANY, wxT(""));

    wxBoxSizer *rateBar = new wxBoxSizer(wxHORIZONTAL);
    rateBar->Add(rateBox, 0, wxRIGHT, 5);
    rateBar->Add(rateText, 0, wxTOP | wxLEFT, 6);

    wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);
    rightPane->Add(screenshot, 0, wxTOP, 5);
    rightPane->Add(rateBar);
    rightPane->Add(textView, 1, wxGROW | wxTOP | wxRIGHT | wxBOTTOM, 7);
    rightPane->Add(buttonHolder);
    /*
    rightPane->Add(supportButton, 0, wxALIGN_RIGHT);
    rightPane->Add(buttonBar, 0);
    */

    tags = new wxListBox(this, myID_TAGS);

    wxBoxSizer *leftPane = new wxBoxSizer(wxVERTICAL);
    leftPane->Add(tags, 1, wxGROW);

    /*
    ad_img = new ImageViewer(this, myID_SCREENSHOT, wxDefaultPosition,
                             wxSize(100,160));
    leftPane->Add(new wxStaticText(this, wxID_ANY, wxT("Sponsor:")), 0);
    leftPane->Add(ad_img, 0);
    */

    createTagList();

    wxBoxSizer *panes = new wxBoxSizer(wxHORIZONTAL);
    panes->Add(leftPane, 30, wxGROW);
    panes->Add(centerPane, 100, wxGROW);
    panes->Add(rightPane, 60, wxGROW);

    SetSizer(panes);

    Connect(myID_TEXTVIEW, wxEVT_COMMAND_TEXT_URL,
            wxTextUrlEventHandler(ListTab::onUrlEvent));

    Connect(myID_TAGS, wxEVT_COMMAND_LISTBOX_SELECTED,
            wxCommandEventHandler(ListTab::onTagSelect));

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

    Connect(myID_RATE, wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(ListTab::onRating));

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

  void createTagList()
  {
    // Set up the tag list
    const vector<int> &base = lister.getBaseList();
    tagSorter.makeTagList(base, taglist);

    // Create and set up the wxString version of the tag list
    std::vector<wxString> labels;
    labels.resize(taglist.size()+1);
    labels[0] = wxString::Format(wxT("All (%d)"), base.size());
    for(int i=0; i<taglist.size(); i++)
      {
        TagSorter::Entry &e = taglist[i];
        wxString &str = labels[i+1];

        str = wxString(e.tag.c_str(), wxConvUTF8);
        str += wxString::Format(wxT(" (%d)"), e.games.size());
      }

    // Clear tag control
    tags->Clear();

    // Add new strings to it
    tags->InsertItems(labels.size(), &labels[0], 0);
  }

  // Respond to clickable URLs in the game description
  void onUrlEvent(wxTextUrlEvent &event)
  {
    if(!event.GetMouseEvent().ButtonDown(wxMOUSE_BTN_LEFT))
      return;

    wxString url = textView->GetRange(event.GetURLStart(), event.GetURLEnd());
    wxLaunchDefaultBrowser(url);
  }

  void onRating(wxCommandEvent &event)
  {
    int rate = event.GetInt();

    // The first choice is just "Rate this game"
    if(rate == 0) return;
    rate = 6 - rate; // The rest are in reverse order
    assert(rate >= 0 && rate <= 5);

    if(select < 0 || select >= lister.size())
      return;

    GameInfo::conv(lister.get(select)).rateGame(rate);

    updateGameInfo();
  }

  void onTagSelect(wxCommandEvent &event)
  {
    int sel = event.GetSelection();

    // Deselections are equivalent to selecting "All"
    if(sel <= 0 || !event.IsSelection() || sel > taglist.size())
      lister.clearSubSelection();
    else
      {
        sel--;

        assert(sel >= 0 && sel < taglist.size());
        TagSorter::Entry &e = taglist[sel];
        lister.setSubSelection(e.games);
      }

    listHasChanged();
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
    screenshot->clear();
    if(select < 0 || select >= lister.size())
      {
        textView->Clear();
        rateText->SetLabel(rateString[0]);
        rateBox->Disable();
        return;
      }

    DataList::Entry &e = lister.get(select);

    // Update the text view
    textView->ChangeValue(wxString(e.tigInfo.desc.c_str(), wxConvUTF8));

    GameInfo &g = GameInfo::conv(e);

    // Request a screenshot update
    g.requestShot(this);

    // Revert rating box
    rateBox->SetSelection(0);

    // Have we already rated this game?
    int rating = g.myRating();

    if(rating == -1)
      {
        // No, enable rating dropdown
        rateBox->Enable();
        rateText->SetLabel(rateString[0]);
      }
    else
      {
        // Yup. Update textbox to reflect our previous rating.
        rateBox->Disable();
        assert(rating >= 0 && rating <= 5);
        rateText->SetLabel(rateString[rating+1]);
      }
  }

  // Called whenever a screenshot is ready
  void shotIsReady(const std::string &idname, const wxImage &shot)
  {
    // Check if the shot belongs to the selected game
    if(select < 0 || select >= lister.size())
      return;
    if(GameInfo::conv(lister.get(select)).entry.idname != idname)
      return;

    // We have a match. Set the screenshot.
    screenshot->loadImage(shot);
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

    DataList::Entry &e = lister.get(select);

    if(e.tigInfo.isDemo)
      {
        supportButton->Enable();
        supportButton->SetLabel(wxT("Buy Game (external link)"));
      }
    else if(e.tigInfo.hasPaypal)
      {
        supportButton->Enable();
        supportButton->SetLabel(wxT("Support Game (donate)"));
      }
    else
      supportButton->Disable();

    b1->Enable();
    b2->Enable();

    GameInfo &g = GameInfo::conv(e);
    if(g.isNone())
      {
        b1->SetLabel(wxT("Install"));
        b2->SetLabel(wxT("No action"));
        b2->Disable();
      }
    else if(g.isWorking())
      {
        b1->SetLabel(wxT("Pause"));
        b2->SetLabel(wxT("Abort"));

        // Pausing is not implemented yet
        b1->Disable();
      }
    else if(g.isInstalled())
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

    else if(id == myID_SORT_RATING)
      lister.sortRating();

    else assert(0);

    listHasChanged();
  }

  void updateStatus(int index)
  {
    GameInfo &e = GameInfo::conv(lister.get(index));

    // If this is the current item, make sure the buttons are set
    // correctly.
    if(index == select) fixButtons();

    try
      {
        int i = e.updateStatus();

        if(i>0)
          statusChanged(false, i == 2);
      }
    catch(std::exception &e)
      {
        errorBox(wxString(e.what(), wxConvUTF8));
        statusChanged();
      }

    // Update the display
    list->RefreshItem(index);
  }

  void startDownload(int index)
  {
    DataList::Entry &e = lister.get(index);
    GameInfo &gi = GameInfo::conv(e);

    {
      // Update the .tig info first
      DataList::TigInfo ti;

      // TODO: In future versions, this will be outsourced to a worker
      // thread to keep from blocking the app.
      if(tig_reader.decodeTigUrl
         (e.urlname,    // url-name
          e.tigurl,     // url to tigfile
          ti,           // where to store result
          false))       // ONLY use cache if fetch fails
        {
          // Copy new data if the fetch was successful
          if(ti.launch != "")
            e.tigInfo = ti;
        }
    }

    gi.startDownload();

    // Update this entry now.
    updateStatus(index);

    /* Update lists and moved to the Installed tab.

       NOTE: this will switch tabs even if download immediately
       fails. This isn't entirely optimal, but is rare enough that it
       doesn't matter.
     */
    statusChanged(true);
  }

  /* Called whenever an item switches lists. Set newInst to true if we
     should switch to the "Installed" tab. dataChange is true if the
     lists have changed (meaning we have to update all the display
     lists)
  */
  void statusChanged(bool newInst=false, bool dataChange=true)
  {
    // Notify our parent if lists have changed
    if(dataChange)
      stat->onDataChanged();

    // Write install status to config file
    jinst.write(data);

    // Notify parent if newInst was set
    if(newInst) stat->switchToInstalled();
  }

  void doAction(int index, int b)
  {
    if(index < 0 || index >= lister.size())
      return;

    GameInfo &e = GameInfo::conv(lister.get(index));

    // True called as an 'activate' command, meaning that the list
    // told us the item was activated (it was double clicked or we
    // pressed 'enter'.)
    bool activate = false;

    if(b == 3)
      {
        activate = true;
        b = 1;
      }

    if(e.isNone() && b == 1)
      {
        startDownload(index);
        return;
      }
    else if(e.isWorking())
      {
        if(b == 2) // Abort
          e.abortJob();
      }
    else if(e.isInstalled())
      {
        // TODO: All these path and file operations could be out-
        // sourced to an external module. One "repository" module that
        // doesn't know about the rest of the program is the best bet.

        // Construct the install path
        boost::filesystem::path dir = conf.gamedir;
        dir /= e.entry.idname;
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
            boost::filesystem::path program = dir / e.entry.tigInfo.launch;

            if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
              errorBox(wxT("WARNING: Launching will probably not work on your platform.\n"));

            // Change the working directory before running. Since none
            // of our own code uses relative paths, this should not
            // affect our own operation.
            boost::filesystem::path workDir;

            // Calculate full working directory path
            if(e.entry.tigInfo.subdir != "")
              // Use user-requested sub-directory
              workDir = dir / e.entry.tigInfo.subdir;
            else
              // Use base path of the executable
              workDir = program.parent_path();

            wxSetWorkingDirectory(wxString(workDir.c_str(), wxConvUTF8));

            wxString command = wxString(program.c_str(), wxConvUTF8);

            int res = wxExecute(command);
            if(res == -1)
              errorBox(wxT("Failed to launch ") + command);

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
                wxSetWorkingDirectory(wxString(get.base.c_str(), wxConvUTF8));

                // Kill all the files
                wxBusyCursor busy;
		boost::filesystem::remove_all(dir);

		// Revert status
                GameInfo::conv(lister.get(index)).setUninstalled();
		if(index == select) fixButtons();
		list->RefreshItem(index);

                // Notify the system that a game has changed status
                statusChanged();
	      }
	    catch(exception &ex)
	      {
		wxString msg = wxT("Could not uninstall ") + e.name +
		  wxT(": ") + wxString(string(ex.what()).c_str(), wxConvUTF8)
		  + wxT("\nPerhaps the game is still running?");
                errorBox(msg);
	      }
          }
      }
  }

  void onGamePage(wxCommandEvent &event)
  {
    if(select < 0 || select >= lister.size())
      return;

    const GameInfo &e = GameInfo::conv(lister.get(select));

    wxString url;

    // Default url is the homepage
    if(e.entry.tigInfo.homepage != "")
      // Game homepage, if it exists
      url = wxString(e.entry.tigInfo.homepage.c_str(), wxConvUTF8);
    else
      // If not, use the tiggit page
      url = wxT("http://tiggit.net/game/") + e.urlname;

    // Only used from context menus
    if(event.GetId() == myID_TIGGIT_PAGE)
      url = wxT("http://tiggit.net/game/") + e.urlname;

    else if(event.GetId() == myID_SUPPORT)
      {
        // Is it a demo?
        if(e.entry.tigInfo.isDemo)
          {
            // Redirect to the buy page, if any
            if(e.entry.tigInfo.buypage != "")
              url = wxString(e.entry.tigInfo.buypage.c_str(), wxConvUTF8);

            // Otherwise, the homepage already set up in 'url' is
            // fine.

            // Warn the user that you can't actually download finished
            // games here yet
            /*
            if(!conf.seen_demo_msg)
              wxMessageBox(wxT("NOTE: Purchasing of games happens entirely outside of the tiggit system. We have not yet integrated any shopping functions into the launcher itself.\n\nWe still encourage you to buy games, but unfortunately you will NOT currently be able to find or play these newly purchased games inside the tiggit launcher. Instead you must download, install and run these games manually.\n\nWe know this is inconvenient, so this is something we hope to improve in the near future, in cooperation with game developers."),
                           wxT("Warning"), wxOK);

            conf.shown_demo_msg();
            */
          }

        else
          // Not a demo. Redirect to the support page.
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
    if(select < 0 || select >= lister.size())
      return;

    const GameInfo &e = GameInfo::conv(lister.get(select));

    // Set up context menu
    wxMenu menu;

    // Set up custom event handler for the menu
    menu.Connect(wxEVT_COMMAND_MENU_SELECTED,
                 (wxObjectEventFunction)&ListTab::onContextClick, NULL, this);

    // State-dependent actions
    if(e.isNone())
      menu.Append(myID_BUTTON1, wxT("Install"));
    else if(e.isWorking())
      menu.Append(myID_BUTTON2, wxT("Abort"));
    else if(e.isInstalled())
      {
        menu.Append(myID_BUTTON1, wxT("Play"));
        menu.Append(myID_BUTTON2, wxT("Uninstall"));
      }

    // Common actions
    menu.AppendSeparator();
    menu.Append(myID_GAMEPAGE, wxT("Visit Website"));
    menu.Append(myID_TIGGIT_PAGE, wxT("Visit Tiggit.net Page"));
    if(e.entry.tigInfo.hasPaypal)
      menu.Append(myID_SUPPORT, wxT("Support Developer"));

    // Currently only supported in Windows
    if((wxGetOsVersion() & wxOS_WINDOWS) != 0)
      if(e.isInstalled())
        menu.Append(myID_OPEN_LOCATION, wxT("Open Location"));

    //menu.Append(myID_REFRESH_ITEM, wxT("Refresh Info"));

    PopupMenu(&menu);
  }

  // Handle events from the context menu
  void onContextClick(wxCommandEvent &evt)
  {
    if(select < 0 || select >= lister.size())
      return;

    const DataList::Entry &e = lister.get(select);

    int id = evt.GetId();
    if(id == myID_BUTTON1 || id == myID_BUTTON2)
      onButton(evt);

    else if(id == myID_GAMEPAGE || id == myID_SUPPORT || id == myID_TIGGIT_PAGE)
      onGamePage(evt);

    else if(id == myID_OPEN_LOCATION)
      {
        boost::filesystem::path dir = conf.gamedir;
        dir /= e.idname;
        dir = get.getPath(dir.string());

        if((wxGetOsVersion() & wxOS_WINDOWS) != 0)
          {
            string cmd = "explorer \"" + dir.string() + "\"";
            wxString command(cmd.c_str(), wxConvUTF8);
            int res = wxExecute(command);
            if(res == -1)
              errorBox(wxT("Failed to launch ") + command);
          }
      }

    else if(id == myID_REFRESH_ITEM)
      cout << "Individual tig refresh isn't implemented yet!\n";
  }

  void onListActivate(wxListEvent &event)
  {
    doAction(event.GetIndex(), 3);
  }

  void dataChanged()
  {
    lister.reset();
    createTagList();
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
      {
        Show();
        ListTab::insertMe();
      }
    else
      Hide();
  }
};

// Lists newly added games
struct NewGameListTab : ListTabNonEmpty
{
  NewGameListTab(wxNotebook *parent, StatusNotify *s)
    : ListTabNonEmpty(parent, wxT("New"), ListKeeper::SL_NEW, s, new GameSortOptions)
  {
    list->addColumn(wxT("Name"), 300, new TitleCol);
    list->addColumn(wxT("Type"), 140, new TypeCol);
  }
};

// Lists freeware games
struct FreewareListTab : ListTab
{
  FreewareListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Browse"), ListKeeper::SL_FREEWARE, s, new GameSortOptions)
  {
    list->addColumn(wxT("Name"), 300, new TitleCol);
    list->addColumn(wxT("Rating"), 70, new RatingCol);
    if(conf.debug)
      list->addColumn(wxT("Downloads"), 70, new DownloadsCol);

    lister.sortRating();
    listHasChanged();
  }

  // Temporary development hack
  void insertMe()
  {
    if(gHasDemos)
      tabName = wxT("Freeware");
    else
      tabName = wxT("Browse");

    ListTab::insertMe();
  }
};

struct DemoListTab : ListTabNonEmpty
{
  DemoListTab(wxNotebook *parent, StatusNotify *s)
    : ListTabNonEmpty(parent, wxT("Demos"), ListKeeper::SL_DEMOS, s, new GameSortOptions)
  {
    list->addColumn(wxT("Name"), 300, new TitleCol);
    list->addColumn(wxT("Rating"), 70, new RatingCol);
    if(conf.debug)
      list->addColumn(wxT("Downloads"), 70, new DownloadsCol);
    //list->addColumn(wxT("Price"), 70, new PriceCol);

    lister.sortRating();
    listHasChanged();
  }
};

struct InstalledListTab : ListTab
{
  InstalledListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Installed"), ListKeeper::SL_INSTALL, s)
  {
    list->addColumn(wxT("Name"), 300, new TitleCol);
    list->addColumn(wxT("Status"), 170, new StatusCol);

    // Add delete = 2nd button accelerator
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, myID_BUTTON2);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);
  }

  void tick()
  {
    for(int i=0; i<lister.size(); i++)
      updateStatus(i);
  }
};

#define myID_MENU_REFRESH 30
#define myID_MENU_REFRESH_TOTAL 20031
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
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 700)),
      version(ver)
  {
    Centre();

    wxMenu *menuFile = new wxMenu;

    /*
    menuFile->Append(wxID_ABOUT, _("&About..."));
    menuFile->AppendSeparator();
    */
    menuFile->Append(wxID_EXIT, _("E&xit"));

    wxMenu *menuList = new wxMenu;
    menuList->Append(myID_MENU_REFRESH, wxT("&Reload List"));
    //menuList->Append(myID_MENU_REFRESH_TOTAL, wxT("Reload E&verything"));

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

    /*
      DISABLED for now since they messes up the search text box.

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_NORMAL, WXK_LEFT, myID_GOLEFT);
    entries[1].Set(wxACCEL_NORMAL, WXK_RIGHT, myID_GORIGHT);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);
    */

    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onAbout));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onExit));
    Connect(myID_MENU_REFRESH, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onRefresh));
    Connect(myID_MENU_REFRESH_TOTAL, wxEVT_COMMAND_MENU_SELECTED,
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

    // Remember whether we have demos or not
    gHasDemos = demoTab->lister.baseSize() != 0;

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

        // Start by selecting the 'new' tab, if applicable
        if(newTab->lister.baseSize() != 0)
          newTab->selectMe();

        // If not, check if there are installed games, and use that as
        // the starting tab instead.
        else if(installedTab->lister.baseSize() != 0)
          installedTab->selectMe();

        // If neither have any games, just start by browsing free
        // games.
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
  void onDataChanged()
  {
    // Notify all tabs that data has changed
    newTab->dataChanged();
    freewareTab->dataChanged();
    demoTab->dataChanged();
    installedTab->dataChanged();

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
    // Did the user request a total refresh?
    if(event.GetId() == myID_MENU_REFRESH_TOTAL)
      {
        if(!ask(wxT("A full reload will takes some time to re-download all necessary data. Are you sure?")))
          return;

        // Configure the loading process to disregard all cached
        // files.
        conf.updateTigs = true;
      }

    updateData(true);
    onDataChanged();
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

    jobQueue.update();
  }
};

static const wxCmdLineEntryDesc cmdLineDesc[] =
  {
    { wxCMD_LINE_OPTION, wxT("u"), wxT("update"), wxT("Update the given tiggit.exe"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_NONE }
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

  wxString updatePath;
  bool doUpdate;

public:
  virtual bool OnInit()
  {
    if (!wxApp::OnInit())
      return false;

    SetAppName(wxT("tiggit"));

    try
      {
        string exe(wxStandardPaths::Get().GetExecutablePath().mb_str());
        string appData(wxStandardPaths::Get().GetUserLocalDataDir().mb_str());
        string upPath(updatePath.mb_str());

        // Update the application first
        string version;
        {
          // This requires us to immediately exit in some cases.
          Updater upd(this);
          if(upd.doAutoUpdate(exe, doUpdate, upPath))
            return false;

          version = upd.version;
        }

        // Then find and load the repository
        Repository::setupPaths(exe, appData);
        auth.load();
        ratings.read();

        // Download cached data if this is the first time we run. This
        // is much faster and more server-friendly than spawning a
        // gazillion connections to get all the tigfiles and images
        // individually.
        if(conf.updateCache)
          {
            CacheFetcher cf(this);
            cf.goDoItAlready();
          }

        updateData(conf.updateList);
        //tig_reader.addTests(data);
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
        errorBox(msg);
      }

    return false;
  }

  void OnInitCmdLine(wxCmdLineParser& parser)
  {
    parser.SetDesc (cmdLineDesc);
    parser.SetSwitchChars(wxT("-"));
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser)
  {
    doUpdate = parser.Found(wxT("u"), &updatePath);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
