class MyFrame : public wxFrame, public StatusNotify
{
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
    wxMenu *menuOpts = new wxMenu;
    menuOpts->AppendCheckItem(myID_MENU_SHOW_VOTES, wxT("Show &Vote Count"),
                              wxT("If checked will display the number of votes next to the rating in the game lists"));

    // Set current options
    menuOpts->Check(myID_MENU_SHOW_VOTES, conf.voteCount);

    /*
    wxMenu *menuData = new wxMenu;
    menuData->Append(myID_MENU_SETDIR, _("Select &Output Directory..."));
    menuData->Append(myID_MENU_EXPORT, _("&Export Data"));
    menuData->Append(myID_MENU_IMPORT, _("Add/&Import Data"));
    menuData->Append(myID_MENU_EXTERNAL, _("External &Games"));
    */

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, _("&App"));
    //menuBar->Append(menuList, _("&List"));
    //menuBar->Append(menuData, _("&Data"));
    menuBar->Append(menuOpts, _("&Options"));

    SetMenuBar( menuBar );

    Connect(myID_MENU_REFRESH, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onRefresh));
    Connect(myID_DEBUG_FUNCTION, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onDebug));
    Connect(myID_MENU_REFRESH_TOTAL, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onRefresh));

    Connect(myID_MENU_SHOW_VOTES, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onOption));

    Connect(myID_MENU_SETDIR, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onDataMenu));
    Connect(myID_MENU_IMPORT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onDataMenu));
    Connect(myID_MENU_EXPORT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onDataMenu));
    Connect(myID_MENU_EXTERNAL, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MyFrame::onDataMenu));

    setupTabs();
    setTabFocus();
  }

  void onDataMenu(wxCommandEvent &event)
  {
    if(event.GetId() == myID_MENU_SETDIR)
      {
        OutputDirDialog dlg(this, get.base.string());
        cout << "SetDir: " << dlg.ok << " " << dlg.move << " "
             << dlg.changed << " " << dlg.path << endl;
      }
    else if(event.GetId() == myID_MENU_IMPORT)
      {
        /*
        ImportDialog dlg(this, get.base.string());
        cout << "Import: " << dlg.ok << " " << dlg.copy << " "
             << dlg.source << endl;
        */
      }
    else if(event.GetId() == myID_MENU_EXPORT)
      {
        /*
        vector<string> games;
        games.push_back("Test1");
        games.push_back("Test2");
        ExportDialog dlg(this, games);
        cout << "Export: " << dlg.ok << " " << dlg.launcher << " "
             << dlg.selected.size() << " " << dlg.output << endl;
        */
      }
    else if(event.GetId() == myID_MENU_EXTERNAL)
      {
        /*
        AddExternalDialog dlg(this);
        cout << "External: " << dlg.ok << " " << dlg.name << " "
             << dlg.exe << endl;
        */
      }
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

  // This is passed on from the list controls
  void onKeyDown(wxKeyEvent &evt)
  {
    // Capture left and right arrow keys
    if(evt.GetKeyCode() == WXK_LEFT)
      {
        book->AdvanceSelection(false);
        setTabFocus();
      }
    else if(evt.GetKeyCode() == WXK_RIGHT)
      {
        book->AdvanceSelection(true);
        setTabFocus();
      }

    // Otherwise, let the list handle the key itself.
    else
      evt.Skip();
  }

  void setupTabs()
  {
    // KILLED this, only initial selection left

    // Re-select the previously selected tab, if it exists
    if((sel == NULL) || !sel->selectMe())
      {
        // No previously selected tab available.

        // Start by selecting the 'new' tab, if applicable
        if(newTab->newGames != 0 && false) // DISABLED
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

  // Don't keep this, but replace it. Pure display refreshes may still
  // happen.
  void softRefresh()
  {
    newTab->listHasChanged(true);
    freewareTab->listHasChanged(true);
    demoTab->listHasChanged(true);
    installedTab->listHasChanged(true);
  }

  void onOption(wxCommandEvent &event)
  {
    if(event.GetId() == myID_MENU_SHOW_VOTES)
      {
        conf.setVoteCount(event.IsChecked());
        softRefresh();
      }
  }
};
