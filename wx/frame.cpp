#include "frame.hpp"
#include "gametab.hpp"
#include "myids.hpp"

using namespace wxTiggit;

#include "alltabs.hpp"
#include "newstab.hpp"

TigFrame::TigFrame(const wxString& title, const std::string &ver,
                   wxGameData &_data)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 700)),
    data(_data)
{
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

  wxPanel *panel = new wxPanel(this);
  book = new wxNotebook(panel, myID_BOOK);

  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(book, 1, wxGROW | wxALL, 10);

  panel->SetSizer(mainSizer);

  // Doesn't seem to work anymore
  SendSizeEvent();

  allTab = new AllGamesTab(book, data);
  newGamesTab = new NewGamesTab(book, data);
  newsTab = new NewsTab(book, data);

  updateTabNames();
  /* TODO: Select startup tab.
     - 'latest' tab if there are any new games added
     - 'installed' tab if there are installed games
     - 'freeware' tab if there are neither
   */
  focusTab();

  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onExit));
  Connect(myID_SPECIAL_KEY, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(TigFrame::onSpecialKey));

  Connect(myID_MENU_SHOW_VOTES, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onOption));

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
  allTab->updateTitle();
  newGamesTab->updateTitle();
  newsTab->updateTitle();
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
