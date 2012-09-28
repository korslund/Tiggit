#include "frame.hpp"
#include "gametab.hpp"
#include "myids.hpp"

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

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT, _("E&xit"));

  wxMenu *menuOpts = new wxMenu;
  menuOpts->AppendCheckItem(myID_MENU_SHOW_VOTES, wxT("Show &Vote Count"),
                            wxT("If checked will display the number of votes next to the rating in the game lists"));

  // Set current options
  menuOpts->Check(myID_MENU_SHOW_VOTES, data.conf().getShowVotes());

  /*
    wxMenu *menuData = new wxMenu;
    menuData->Append(myID_MENU_SETDIR, _("Select &Output Directory..."));
    menuData->Append(myID_MENU_EXPORT, _("&Export Data"));
    menuData->Append(myID_MENU_IMPORT, _("Add/&Import Data"));
    menuData->Append(myID_MENU_EXTERNAL, _("External &Games"));
  */

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, _("&App"));
  //menuBar->Append(menuData, _("&Data"));
  menuBar->Append(menuOpts, _("&Options"));

  SetMenuBar(menuBar);

  CreateStatusBar();
  SetStatusText(strToWx("Welcome to Tiggit - version " + ver));

  mainSizer = new wxBoxSizer(wxVERTICAL);
  wxPanel *panel = new wxPanel(this);

  noticeSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(noticeSizer, 0, wxGROW | wxALL, 7);

  noticeSizer->Add(noticeText = new wxStaticText(panel, -1, wxT("No text")), 0, wxTOP, 5);
  noticeSizer->Add(noticeButton = new wxButton(panel, myID_BUTTON_NOTICE, wxT("No action")), 0, wxLEFT, 10);

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
}

void TigFrame::onNoticeButton(wxCommandEvent &event)
{
  mainSizer->Show(noticeSizer, false);
  mainSizer->Layout();
  data.notifyButton(noticeID);
  noticeID = 0;
}

void TigFrame::displayNotification(const std::string &message, const std::string &button,
                                   int id)
{
  noticeText->SetLabel(strToWx(message));
  noticeButton->SetLabel(strToWx(button));
  noticeID = id;

  mainSizer->Show(noticeSizer, true);
  mainSizer->Layout();
}


void TigFrame::onDataMenu(wxCommandEvent &event)
{
  /*
  if(event.GetId() == myID_MENU_SETDIR)
    {
      OutputDirDialog dlg(this, get.base.string());
      cout << "SetDir: " << dlg.ok << " " << dlg.move << " "
           << dlg.changed << " " << dlg.path << endl;
    }
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
