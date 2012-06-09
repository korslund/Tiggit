#include "frame.hpp"

#include "gametab.hpp"

using namespace wxTiggit;

struct AllGamesTab : GameTab
{
  AllGamesTab(wxNotebook *parent)
    : GameTab(parent, wxT("All Games"), 0)
  {
  }
};

TigFrame::TigFrame(const wxString& title, const std::string &ver)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 700))
{
  Centre();

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT, _("E&xit"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, _("&App"));

  SetMenuBar( menuBar );

  CreateStatusBar();
  SetStatusText(strToWx("Welcome to Tiggit - version " + ver));

  wxPanel *panel = new wxPanel(this);
  book = new wxNotebook(panel, myID_BOOK);

  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(book, 1, wxGROW | wxALL, 10);

  panel->SetSizer(mainSizer);

  allTab = new AllGamesTab(book);

  updateTabNames();
  focusTab();

  Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(TigFrame::onExit));

  // Without this the menubar doesn't appear (in GTK on Linux) until
  // you move the mouse.
  SendSizeEvent();
}

void TigFrame::updateTabNames()
{
  allTab->updateTitle();
}

void TigFrame::focusTab()
{
  int cur = book->GetSelection();
  TabBase *tab = dynamic_cast<TabBase*>(book->GetPage(cur));
  assert(tab);
  tab->gotFocus();
}
