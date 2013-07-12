#include "frame.hpp"
#include "gametab.hpp"
#include "myids.hpp"
#include "dialogs.hpp"
#include "boxes.hpp"

using namespace wxTiggit;

#include "alltabs.hpp"
#include "newstab.hpp"

#define myID_BUTTON_NOTICE 20230

TigFrame::TigFrame(const wxString& title, const std::string &ver,
                   wxGameData &_data)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 700)),
    data(_data)
{
  data.listener = this;

  Centre();

  wxMenuBar *menuBar = new wxMenuBar;

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT, _("E&xit"));
  menuBar->Append(menuFile, _("&App"));

  wxMenu *menuOpts = new wxMenu;
  menuOpts->AppendCheckItem(myID_MENU_SHOW_VOTES, wxT("Show &Vote Count"),
                            wxT("If checked will display the number of votes next to the rating in the game lists"));
  menuOpts->Append(myID_MENU_SETDIR, _("Set &Data Location..."));
  menuBar->Append(menuOpts, _("&Options"));

  // Set current options
  menuOpts->Check(myID_MENU_SHOW_VOTES, data.conf().getShowVotes());

  /*
  wxMenu *menuData = new wxMenu;
  menuData->Append(myID_MENU_SETDIR, _("Set &Data Location..."));
  menuData->Append(myID_MENU_EXPORT, _("&Export Data"));
  menuData->Append(myID_MENU_IMPORT, _("Add/&Import Data"));
  menuData->Append(myID_MENU_EXTERNAL, _("External &Games"));
  menuBar->Append(menuData, _("&Data"));
  */

  wxMenu *menuActions = new wxMenu;
  menuActions->Append(myID_MENU_SUGGEST, _("S&uggest Game..."));
  menuBar->Append(menuActions, _("&Actions"));

  wxMenu *menuLibraries = new wxMenu;
  menuActions->AppendSubMenu(menuLibraries, _("&Install..."));

  const std::vector<std::string> &libs = data.getLibraryMenu();

  for(int i=0; i<libs.size(); i++)
    {
      menuLibraries->Append(myID_LIBRARY+i, strToWx(libs[i]));
      Connect(myID_LIBRARY+i,  wxEVT_COMMAND_MENU_SELECTED,
              wxCommandEventHandler(TigFrame::onLibraryMenu));
    }

  SetMenuBar(menuBar);

  CreateStatusBar(2);
  int widths[] = {200,-1};
  SetStatusWidths(2, widths);
  SetStatusText(strToWx("Tiggit version " + ver), 0);
  SetStatusBarPane(2);

  mainSizer = new wxBoxSizer(wxVERTICAL);
  wxPanel *panel = new wxPanel(this);

  noticeSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(noticeSizer, 0, wxGROW | wxLEFT, 10);

  noticeSizer->Add(noticeText = new wxStaticText(panel, -1, wxT("No text")), 0, wxTOP | wxRIGHT, 7);
  noticeSizer->Add(noticeButton = new wxButton(panel, myID_BUTTON_NOTICE, wxT("No action")), 0, wxLEFT, 10);
  noticeSizer->Add(noticeGauge = new wxGauge(panel, -1, 10), 1, wxLEFT | wxRIGHT, 10);

  mainSizer->Show(noticeSizer, false);

  book = new wxNotebook(panel, myID_BOOK);
  mainSizer->Add(book, 1, wxGROW | wxALL, 10);

  panel->SetSizer(mainSizer);

  newGamesTab = new NewGamesTab(book, data);
  freewareTab = new FreewareTab(book, data);
  demosTab = new DemoTab(book, data);
  installedTab = new InstalledTab(book, data);
  newsTab = new NewsTab(book, data);

  updateTabNames();

  // Select starting tab. Start off on the "Latest" tab if there are
  // any new games (this will probably be user-configured later.)
  if(!newGamesTab->isEmpty())
    newGamesTab->select();

  // If there aren't any new games, select the Installed tab instead,
  // if any games are installed.
  else if(!installedTab->isEmpty())
    installedTab->select();

  // Otherwise just select the freeware tab.
  else
    freewareTab->select();

  Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(TigFrame::onClose));

  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onExit));
  Connect(myID_SPECIAL_KEY, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(TigFrame::onSpecialKey));

  Connect(myID_MENU_SHOW_VOTES, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onOption));

  Connect(myID_BUTTON_NOTICE, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(TigFrame::onNoticeButton));

  Connect(myID_MENU_SETDIR, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onDataMenu));
  Connect(myID_MENU_IMPORT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onDataMenu));
  Connect(myID_MENU_EXPORT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onDataMenu));
  Connect(myID_MENU_EXTERNAL, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onDataMenu));

  Connect(myID_MENU_SUGGEST, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onSuggest));
  Connect(myID_MENU_SUGGEST, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(TigFrame::onSuggest));
}

#include "suggest.hpp"

void TigFrame::onSuggest(wxCommandEvent &event)
{
  SuggestDialog diag(this);

  if(diag.ok)
    {
      data.submitGame(diag.title, diag.homepage, diag.shot, diag.download, diag.version,
                      diag.devname, diag.tags, diag.type, diag.desc);
    }
}

void TigFrame::onClose(wxCloseEvent &event)
{
  bool exit = true;
  if(event.CanVeto())
    {
      if(data.isActive())
        exit = Boxes::ask("There are downloads in progress. Are you sure you want to exit? All downloads will be aborted.");
    }

  if(exit) Destroy();
  else event.Veto();
}

void TigFrame::onOption(wxCommandEvent &event)
{
  if(event.GetId() == myID_MENU_SHOW_VOTES)
    {
      data.conf().setShowVotes(event.IsChecked());
      Refresh();
    }
}

void TigFrame::updateTabNames()
{
  newGamesTab->updateTitle();
  freewareTab->updateTitle();
  demosTab->updateTitle();
  installedTab->updateTitle();
  newsTab->updateTitle();
}

void TigFrame::refreshNews()
{
  newsTab->reloadData();

  // Reload left image in gametabs as well
  newGamesTab->reloadData();
  freewareTab->reloadData();
  demosTab->reloadData();
  installedTab->reloadData();
}

void TigFrame::onNoticeButton(wxCommandEvent &event)
{
  mainSizer->Show(noticeSizer, false);
  mainSizer->Layout();
  data.notifyButton(noticeID);
  noticeID = 0;
}

void TigFrame::displayProgress(const std::string &message, uint64_t cur, uint64_t total)
{
  noticeGauge->SetRange(total);
  noticeGauge->SetValue(cur);
  if(cur == total && message == "")
    {
      mainSizer->Hide(noticeSizer);
    }
  else
    {
      noticeText->SetLabel(strToWx(message));
      mainSizer->Show(noticeSizer, true);
      noticeSizer->Show(noticeButton, false);
    }
  mainSizer->Layout();
}

void TigFrame::displayNotification(const std::string &message, const std::string &button,
                                   int id)
{
  noticeText->SetLabel(strToWx(message));
  noticeButton->SetLabel(strToWx(button));
  noticeSizer->Show(noticeButton);
  noticeID = id;

  mainSizer->Show(noticeSizer, true);
  noticeSizer->Show(noticeGauge, false);
  mainSizer->Layout();
}

void TigFrame::onLibraryMenu(wxCommandEvent &event)
{
  int num = event.GetId() - myID_LIBRARY;
  data.installLibrary(num);
}

void TigFrame::onDataMenu(wxCommandEvent &event)
{
  if(event.GetId() == myID_MENU_SETDIR)
    {
      const std::string &curDir = data.getRepoDir();
      bool error = false;

      while(true)
        {
          std::string path;

          {
            OutputDirDialog dlg(this, curDir, "", error);

            // Abort if the user pressed 'cancel', or if the new path is
            // the same as the old.
            if(!dlg.ok || !dlg.changed)
              break;

            path = dlg.path;

            // Make sure dlg goes out of scope before continuing
          }

          // Start import procedure. Will return false if the path was
          // not writable, otherwise true on success.
          if(data.moveRepo(path)) break;

          // Give the user feedback and let them try again
          error = true;
        }
    }
  /*
  else if(event.GetId() == myID_MENU_IMPORT)
    {
      ImportDialog dlg(this, get.base.string());
      cout << "Import: " << dlg.ok << " " << dlg.copy << " "
           << dlg.source << endl;
    }
  else if(event.GetId() == myID_MENU_EXPORT)
    {
      vector<string> games;
      games.push_back("Test1");
      games.push_back("Test2");
      ExportDialog dlg(this, games);
      cout << "Export: " << dlg.ok << " " << dlg.launcher << " "
           << dlg.selected.size() << " " << dlg.output << endl;
    }
  else if(event.GetId() == myID_MENU_EXTERNAL)
    {
      AddExternalDialog dlg(this);
      cout << "External: " << dlg.ok << " " << dlg.name << " "
           << dlg.exe << endl;
    }
  */
  else assert(0);
}

void TigFrame::onSpecialKey(wxCommandEvent &event)
{
  if(event.GetInt() == WXK_LEFT)
    {
      book->AdvanceSelection(false);
      focusTab();
    }
  else if(event.GetInt() == WXK_RIGHT)
    {
      book->AdvanceSelection(true);
      focusTab();
    }
  else
    event.Skip();
}

void TigFrame::focusTab()
{
  int cur = book->GetSelection();
  TabBase *tab = dynamic_cast<TabBase*>(book->GetPage(cur));
  assert(tab);
  tab->gotFocus();
}
