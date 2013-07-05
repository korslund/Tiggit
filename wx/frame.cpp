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

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT, _("E&xit"));

  wxMenu *menuOpts = new wxMenu;
  menuOpts->AppendCheckItem(myID_MENU_SHOW_VOTES, wxT("Show &Vote Count"),
                            wxT("If checked will display the number of votes next to the rating in the game lists"));
  menuOpts->Append(myID_MENU_SETDIR, _("Set &Data Location..."));

  // Set current options
  menuOpts->Check(myID_MENU_SHOW_VOTES, data.conf().getShowVotes());

  /*
  wxMenu *menuData = new wxMenu;
  menuData->Append(myID_MENU_SETDIR, _("Set &Data Location..."));
  menuData->Append(myID_MENU_EXPORT, _("&Export Data"));
  menuData->Append(myID_MENU_IMPORT, _("Add/&Import Data"));
  menuData->Append(myID_MENU_EXTERNAL, _("External &Games"));
  */

  wxMenu *menuCommunity = new wxMenu;
  menuCommunity->Append(myID_MENU_SUGGEST, _("S&uggest Game..."));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, _("&App"));
  menuBar->Append(menuOpts, _("&Options"));
  //menuBar->Append(menuData, _("&Data"));
  menuBar->Append(menuCommunity, _("&Community"));

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

struct SuggestDialog : wxDialog
{
  bool ok;

  std::string title, homepage, shot, download, version, devname, tags, type, desc;

  wxPanel *panel;
  wxBoxSizer *sizer;

  wxBoxSizer *getSpaced(int spacing=6)
  {
    wxBoxSizer *holder = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(holder, 0, wxTOP, spacing);
    return holder;
  }

  wxTextCtrl *addText(const std::string &name, int spacing=6, bool isMultiLine=false)
  {
    wxBoxSizer *holder = getSpaced(spacing);

    holder->Add(new wxStaticText(panel, wxID_ANY, strToWx(name), wxDefaultPosition,
                                 wxSize(100,isMultiLine?50:22)));
    wxTextCtrl *res;
    if(isMultiLine)
      res = new wxTextCtrl(panel, -1, wxT(""), wxDefaultPosition, wxSize(320,160),
                           wxTE_MULTILINE);
    else
      res = new wxTextCtrl(panel, -1, wxT(""), wxDefaultPosition, wxSize(320,22));

    holder->Add(res);

    return res;
  }

  wxTextCtrl *addMultiText(const std::string &name, int spacing=6)
  {
    return addText(name, spacing, true);
  }

  wxRadioButton *addRadio(const std::string &name, bool isFirst=false, int spacing=0)
  {
    wxBoxSizer *holder = getSpaced(spacing);

    wxRadioButton *res;
    if(isFirst) res = new wxRadioButton(panel, -1, strToWx(name), wxDefaultPosition,
                                        wxDefaultSize, wxRB_GROUP);
    else res = new wxRadioButton(panel, -1, strToWx(name));
    holder->Add(res, 0, wxLEFT, 100);
    return res;
  }

  SuggestDialog(wxWindow *parent)
    : wxDialog(parent, -1, wxT("Suggest A Game"), wxDefaultPosition,
               wxSize(300,500))
  {
    panel = new wxPanel(this);

    // Set up the sizers
    wxBoxSizer *outer = new wxBoxSizer(wxVERTICAL);
    sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(outer);
    outer->Add(sizer, 1, wxGROW | wxALL, 20);

    wxTextCtrl *t_title, *t_homepage, *t_shot, *t_download, *t_version,
      *t_devname, *t_tags, *t_desc;

    wxRadioButton *r_free, *r_open, *r_demo;

    sizer->Add(new wxStaticText(panel, -1, wxT("Suggest a game to add to Tiggit:")));
    t_title = addText("Title:", 20);
    sizer->Add(new wxStaticText(panel, -1, wxT("OPTIONAL: provide more info to help add the game faster:")), 0, wxTOP, 17);
    t_homepage = addText("Homepage URL:", 18);
    t_shot = addText("Screenshot URL:");
    t_download = addText("Download URL:");
    t_desc = addMultiText("Description: (shown in Tiggit)", 8);

    r_free = addRadio("Freeware", true, 6);
    r_open = addRadio("Open source");
    r_demo = addRadio("Demo / Commercial");

    t_version = addText("Latest version:");
    t_devname = addText("Developer:");
    t_tags = addText("Tags:");
    sizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Suggested tags: arcade action cards casual fps fighter puzzle platform roguelike strategy music simulation racing rpg shooter single-player multi-player"), wxDefaultPosition, wxSize(400,55)), 0, wxTOP, 10);

    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttons, 0, wxTOP, 12);
    buttons->Add(new wxButton(panel, wxID_OK, wxT("Save")));
    buttons->Add(new wxButton(panel, wxID_CANCEL, wxT("Cancel")));

    panel->Fit();
    Fit();
    Center();

    ok = ShowModal() == wxID_OK;

    if(ok)
      {
        title = wxToStr(t_title->GetValue());
        homepage = wxToStr(t_homepage->GetValue());
        shot = wxToStr(t_shot->GetValue());
        download = wxToStr(t_download->GetValue());
        version = wxToStr(t_version->GetValue());
        devname = wxToStr(t_devname->GetValue());
        tags = wxToStr(t_tags->GetValue());
        desc = wxToStr(t_desc->GetValue());

        if(r_free->GetValue()) type="free";
        else if(r_open->GetValue()) type="open";
        else if(r_demo->GetValue()) type="demo";
        else type="unset";

        fixURL(homepage);
        fixURL(shot);
        fixURL(download);

        fixTags(tags);
      }

    Destroy();
  }

  void fixTags(std::string &tags)
  {
    for(int i=0; i<tags.size(); i++)
      {
        char &c = tags[i];
        if(c >= 'A' && c <= 'Z')
          c += 'a'-'A';
        else if(c==',' || c==':' || c==';') c=' ';
      }
  }

  void fixURL(std::string &url)
  {
    if(url != "" && url.find("://") == std::string::npos)
      url = "http://" + url;
  }
};

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
