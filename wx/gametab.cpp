#include "gametab.hpp"
#include "image_viewer.hpp"
#include "image_clickable.hpp"
#include "myids.hpp"
#include "boxes.hpp"

using namespace wxTiggit;

GameTab::GameTab(wxNotebook *parent, const wxString &name, wxGameList &lst,
                 wxGameData &_data)
  : TabBase(parent, name), lister(lst), data(_data), select(0), last_launch(0)
{
  list = new GameListView(this, myID_LIST, lister);

  wxBoxSizer *searchBox = new wxBoxSizer(wxHORIZONTAL);
  searchBox->Add(new wxStaticText(this, wxID_ANY, wxT("Search:")), 0);
  searchCtrl = new wxTextCtrl(this, myID_SEARCH_BOX, wxT(""), wxDefaultPosition,
                              wxSize(260,22));
  searchBox->Add(searchCtrl,1, wxGROW);

  wxBoxSizer *bcLeft = new wxBoxSizer(wxVERTICAL);
  bcLeft->Add(searchBox, 0, wxBOTTOM | wxLEFT, 2);
  bcLeft->Add(new wxStaticText(this, wxID_ANY,
                               wxT("Mouse: double-click to play / install\nKeyboard: arrow keys + enter, delete")),
              0, wxLEFT | wxBOTTOM, 4);

  wxBoxSizer *bottomCenter = new wxBoxSizer(wxHORIZONTAL);
  bottomCenter->Add(bcLeft, 1, wxGROW);
  bottomCenter->Add(new wxButton(this, myID_MENU_SUGGEST, wxT("+ Add Game")), 0, wxRIGHT, 10);

  textView = new wxTextCtrl
    (this, myID_TEXTVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
     wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

  screenshot = new ImageViewer(this, myID_SCREENSHOT, wxDefaultPosition,
                               wxSize(300,260));

  b1 = new wxButton(this, myID_BUTTON1, wxT("No action"));
  b2 = new wxButton(this, myID_BUTTON2, wxT("No action"));

  wxBoxSizer *buttonBar = new wxBoxSizer(wxHORIZONTAL);
  buttonBar->Add(b1, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);
  buttonBar->Add(b2, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);

  wxBoxSizer *infoSizer = new wxBoxSizer(wxVERTICAL);
  infoSizer->Add(versionText = new wxStaticText(this, wxID_ANY, wxT("")));
  infoSizer->Add(sizeText = new wxStaticText(this, wxID_ANY, wxT("")));
  buttonBar->Add(infoSizer, 0, wxLEFT, 10);

  wxBoxSizer *buttonBar2 = new wxBoxSizer(wxHORIZONTAL);
  buttonBar2->Add(b3 = new wxButton(this, myID_GAMEPAGE, wxT("Game Website")));
  buttonBar2->Add(b4 = new wxButton(this, myID_BROKEN, wxT("Report Game Issue")),
                  0, wxLEFT, 3);

  wxBoxSizer *buttonHolder = new wxBoxSizer(wxVERTICAL);
  buttonHolder->Add(buttonBar, 0);
  buttonHolder->Add(buttonBar2, 0, wxTOP | wxBOTTOM, 3);

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

  tags = new wxListBox(this, myID_TAGS);

  wxBoxSizer *bottomLeft = new wxBoxSizer(wxVERTICAL);

  leftImage = new ClickableImage(this, myID_LEFTIMAGE, wxDefaultPosition,
                                       wxSize(150,76));
  bottomLeft->Add(leftImage, 0, wxALL, 3);

  setLeftImage();

  wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);
  rightPane->Add(screenshot, 0, wxTOP, 5);
  rightPane->Add(rateBar);
  rightPane->Add(textView, 1, wxGROW | wxTOP | wxRIGHT, 7);

  wxBoxSizer *topPart = new wxBoxSizer(wxHORIZONTAL);
  topPart->Add(tags, 30, wxGROW);
  topPart->Add(list, 100, wxGROW | wxRIGHT, 10);
  topPart->Add(rightPane, 60, wxGROW | wxBOTTOM, 2);

  wxBoxSizer *bottomPart = new wxBoxSizer(wxHORIZONTAL);
  bottomPart->Add(bottomLeft, 30, wxGROW);
  bottomPart->Add(bottomCenter, 100, wxGROW | wxTOP, 5);
  bottomPart->Add(buttonHolder, 60, wxGROW | wxTOP, 5);

  wxBoxSizer *parts = new wxBoxSizer(wxVERTICAL);
  parts->Add(topPart, 1, wxGROW);
  parts->Add(bottomPart, 0, wxGROW | wxTOP, 1);

  SetSizer(parts);

  Connect(myID_TEXTVIEW, wxEVT_COMMAND_TEXT_URL,
          wxTextUrlEventHandler(GameTab::onUrlEvent));

  Connect(myID_TAGS, wxEVT_COMMAND_LISTBOX_SELECTED,
          wxCommandEventHandler(GameTab::onTagSelect));
  Connect(myID_GAMEPAGE, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(GameTab::onGamePage));
  Connect(myID_BROKEN, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(GameTab::onBroken));

  Connect(myID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(GameTab::onButton));
  Connect(myID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(GameTab::onButton));

  Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_SELECTED,
          wxListEventHandler(GameTab::onListSelect));
  Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_DESELECTED,
          wxListEventHandler(GameTab::onListDeselect));

  Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
          wxListEventHandler(GameTab::onListRightClick));
  Connect(myID_LIST, wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
          wxListEventHandler(GameTab::onListActivate));

  Connect(myID_RATE, wxEVT_COMMAND_CHOICE_SELECTED,
          wxCommandEventHandler(GameTab::onRating));
  Connect(myID_SEARCH_BOX, wxEVT_COMMAND_TEXT_UPDATED,
          wxCommandEventHandler(GameTab::onSearch));

  Connect(myID_SPECIAL_KEY, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(GameTab::onSpecialKey));

  lister.addListener(this);
  gameListChanged();
}

// Hard-coded list of tags to display
static const char* itags[] =
  {
    "Action",
    "Arcade",
    "Cards",
    "Casual",
    "Fighter",
    "FPS",
    "Music",
    "Platform",
    "Point-n-click",
    "Puzzle",
    "Rogue-like",
    "RPG",
    "Racing",
    "Shooter",
    "Simulation",
    "Strategy",
    "single-player",
    "multi-player",
    "open-source",
    "\0"
  };

void GameTab::reloadData()
{
  setLeftImage();
}

void GameTab::setLeftImage()
{
  std::string ifile, iurl;
  if(data.getLeftImage(ifile, iurl))
    leftImage->setData(ifile, iurl);
  else
    leftImage->setData("", "");
}

void GameTab::updateTags()
{
  using namespace std;

  tagList.clear();
  lister.clearTags();

  vector<wxString> labels;
  labels.push_back(wxString::Format(wxT("All (%d)"), lister.size()));

  for(const char **ip = itags; **ip != 0; ip++)
    {
      string tag(*ip);

      int count = lister.countTags(tag);

      if(count)
        {
          // Add the tag
          tagList.push_back(tag);
          labels.push_back(strToWx(tag) + wxString::Format(wxT(" (%d)"), count));
        }
    }

  // Set up the tag control
  tags->Clear();
  tags->InsertItems(labels.size(), &labels[0], 0);
}

void GameTab::onSpecialKey(wxCommandEvent &event)
{
  if(event.GetInt() == WXK_DELETE)
    doAction2(select);
  else
    event.Skip();
}

void GameTab::gotFocus()
{
  list->SetFocus();
  updateSelection();
}

int GameTab::getTitleNumber()
{
  return lister.totalSize();
}

// Respond to clickable URLs in the game description
void GameTab::onUrlEvent(wxTextUrlEvent &event)
{
  if(!event.GetMouseEvent().ButtonDown(wxMOUSE_BTN_LEFT))
    return;

  wxString url = textView->GetRange(event.GetURLStart(), event.GetURLEnd());
  wxLaunchDefaultBrowser(url);
}

void GameTab::onRating(wxCommandEvent &event)
{
  int rate = event.GetInt();

  // The first choice is just "Rate this game"
  if(rate == 0) return;
  rate = 6 - rate; // The rest are in reverse order
  assert(rate >= 0 && rate <= 5);

  if(select < 0 || select >= lister.size())
    return;

  lister.edit(select).rateGame(rate);

  gotFocus();
}

void GameTab::onTagSelect(wxCommandEvent &event)
{
  int sel = event.GetSelection();

  /* Clear search when setting tags, as a lingering search string
     might be easily missed by the user, and they will usually expect
     that clicking on a tag will give them the full list of games for
     that tag (especially since the tag list displays tags along with
     the full game number), regardless of whether they searched for a
     game in the past.

     We might change this behavior later if we change how searching
     and tags interact.
  */
  searchCtrl->Clear();

  // Deselections are equivalent to selecting "All" (sel==0).
  if(sel <= 0 || !event.IsSelection() || sel > tagList.size())
    {
      lister.clearTags();
    }
  else
    {
      sel--; // Count from zero, not counting "All"
      assert(sel >= 0 && sel < tagList.size());
      lister.setTags(tagList[sel]);
    }
}

void GameTab::onSearch(wxCommandEvent &event)
{
  lister.setSearch(wxToStr(event.GetString()));
}

void GameTab::onGamePage(wxCommandEvent &event)
{
  if(select < 0 || select >= lister.size())
    return;

  const wxGameInfo &e = lister.get(select);

  if(e.getHomepage() != "")
    wxLaunchDefaultBrowser(strToWx(e.getHomepage()));
}

void GameTab::onButton(wxCommandEvent &event)
{
  if(event.GetId() == myID_BUTTON1)
    doAction1(select);
  else if(event.GetId() == myID_BUTTON2)
    doAction2(select);
  gotFocus();
}

void GameTab::onListActivate(wxListEvent &event)
{
  doAction1(event.GetIndex());
}

void GameTab::onListDeselect(wxListEvent &event)
{
  select = -1;
  updateSelection();
}

void GameTab::onListSelect(wxListEvent &event)
{
  select = event.GetIndex();
  updateSelection();
}

void GameTab::gameInfoChanged() { updateSelection(); }
void GameTab::gameSelectionChanged()
{
  updateSelection();
}
void GameTab::gameListChanged()
{
  updateTitle();

  // Updating tags will also automatically clear tag selection
  updateTags();

  // This generates the approprate event and clears the search
  // selection
  searchCtrl->Clear();

  // Just to make sure we update the current selection
  gameSelectionChanged();
}

void GameTab::onListRightClick(wxListEvent &event)
{
  if(select < 0 || select >= lister.size())
    return;

  const wxGameInfo &e = lister.get(select);

  // Set up context menu
  wxMenu menu;

  // Set up custom event handler for the menu
  menu.Connect(wxEVT_COMMAND_MENU_SELECTED,
               (wxObjectEventFunction)&GameTab::onContextClick, NULL, this);

  // State-dependent actions
  if(e.isUninstalled())
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
  menu.Append(myID_BROKEN, wxT("Report broken game"));;

  // Currently only supported in Windows
  if((wxGetOsVersion() & wxOS_WINDOWS) != 0)
    if(e.isInstalled())
      menu.Append(myID_OPEN_LOCATION, wxT("Open Location"));

  PopupMenu(&menu);
}

#include "broken.hpp"

void GameTab::onBroken(wxCommandEvent &evt)
{
  if(select < 0 || select >= lister.size())
    return;

  const wxGameInfo &e = lister.get(select);

  ProblemDialog diag(this, e.getTitle());

  if(diag.ok)
    {
      data.submitBroken(e.getIdName(), diag.comment);
    }
}

// Handle events from the context menu
void GameTab::onContextClick(wxCommandEvent &evt)
{
  if(select < 0 || select >= lister.size())
    return;

  const wxGameInfo &e = lister.get(select);

  int id = evt.GetId();
  if(id == myID_BUTTON1 || id == myID_BUTTON2)
    onButton(evt);

  else if(id == myID_GAMEPAGE)
    onGamePage(evt);

  else if(id == myID_BROKEN)
    onBroken(evt);

  else if(id == myID_OPEN_LOCATION)
    {
      if((wxGetOsVersion() & wxOS_WINDOWS) != 0)
        {
          std::string cmd = "explorer \"" + e.getDir() + "\"";

	  // Replace / with \ to make explorer happy
	  for(int i=0; i<cmd.size(); i++)
	    if(cmd[i] == '/') cmd[i] = '\\';

          int res = wxExecute(strToWx(cmd));
          if(res == -1)
            Boxes::error("Failed to launch " + cmd);
        }
    }
}

/*
  Called whenever there is a chance that a new game has been
  selected. This updates the available action buttons as well as the
  displayed game info to match the current selection.
*/
void GameTab::updateSelection()
{
  fixButtons();
  updateGameInfo();
}

// Fix buttons for the current selected item (if any)
void GameTab::fixButtons()
{
  if(select < 0 || select >= lister.size())
    {
      b1->Disable();
      b2->Disable();
      b3->Disable();
      b4->Disable();
      b1->SetLabel(wxT("No action"));
      b2->SetLabel(wxT("No action"));
      return;
    }

  const wxGameInfo &e = lister.get(select);

  b1->Enable();
  b2->Enable();
  b3->Enable();
  b4->Enable();

  if(e.isUninstalled())
    {
      b1->SetLabel(wxT("Install"));
      b2->SetLabel(wxT("No action"));
      b2->Disable();
    }
  else if(e.isWorking())
    {
      b1->SetLabel(wxT("Pause"));
      b2->SetLabel(wxT("Abort"));

      // Pausing is not implemented
      b1->Disable();
    }
  else if(e.isInstalled())
    {
      b1->SetLabel(wxT("Play Now"));
      b2->SetLabel(wxT("Uninstall"));
    }
}

void GameTab::updateGameInfo()
{
  screenshot->clear();
  if(select < 0 || select >= lister.size())
    {
      textView->Clear();
      rateText->SetLabel(rateString[0]);
      sizeText->SetLabel(wxT(""));
      versionText->SetLabel(wxT(""));
      rateBox->Disable();
      return;
    }

  wxGameInfo &e = lister.edit(select);

  // Update the text view
  textView->ChangeValue(e.getDesc());

  // Set the screenshot
  screenshot->loadImage(e.getShot());

  // Set size and version info
  sizeText->SetLabel(wxT("Size: ") + e.getSize());
  versionText->SetLabel(wxT("Version: " + e.getVersion()));

  // Revert rating box
  rateBox->SetSelection(0);

  // Have we already rated this game?
  int rating = e.myRating();

  if(rating == -1)
    {
      // Nope. Enable rating dropdown.
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

void GameTab::doAction1(int index)
{
  if(index < 0 || index >= lister.size())
    return;

  wxGameInfo &e = lister.edit(index);

  if(e.isUninstalled()) e.installGame();
  else if(e.isInstalled())
    {
      // If the user just launched a game, ask them if they meant to
      // do it twice.
      time_t now = time(0);
      if(difftime(now, last_launch) <= 10)
        if(!Boxes::ask("You just started a game. Are you sure you want to launch again?\n\nSome games may take a few seconds to start."))
          return;

      last_launch = now;

      try { e.launchGame(); }
      catch(std::exception &e)
        {
          Boxes::error("Failed to launch game: " + std::string(e.what()));
        }
      catch(...)
        {
          Boxes::error("Failed to launch game: Unknown error");
        }
    }
}

void GameTab::doAction2(int index)
{
  if(index < 0 || index >= lister.size())
    return;

  wxGameInfo &e = lister.edit(index);

  if(e.isWorking()) e.abortJob();
  else if(e.isInstalled())
    {
      if(Boxes::ask(wxT("Are you sure you want to uninstall ") + e.getTitle() +
                    wxT("? All savegames and configuration will be lost.")
                    )
         )
        e.uninstallGame();
    }
}
