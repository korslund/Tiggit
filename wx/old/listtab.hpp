/*
  CHANGES:

  - kill the statusnotify class. Replace it, if necessary, with custom
    wx events instead. Looks like this is pretty simple (bookmarked a
    guide.) Instead of creating new event classes and whatnot, we just
    spoof button click events, and use different IDs for the
    messages. It's possible we can kill the KeyAccel class with better
    understanding of wx event handling as well.

  - again, the biggest change here is that the data now updates the
    view, not the other way around. Any command to update this window
    will come from the list view. We already capture some native list
    events here, and that's fine. It's possible that those are enough
    to signal all the changes we need from the list view to this.
 */

struct ListTab : TabBase, ScreenshotCallback, KeyAccel
{
  GameList *list;
  int select;
  time_t last_launch;
  ListKeeper lister;
  StatusNotify *stat;

  std::vector<TagSorter::Entry> taglist;

  ListTab(wxNotebook *parent, const wxString &name, int listType,
          StatusNotify *s)
    : TabBase(parent), select(-1), last_launch(0),
      lister(data, listType), tabName(name), stat(s)
  {
    list = new MyList(this, myID_LIST, lister, this);

    createTagList();


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
  }

  // Filter key clicks from the list view
  void onKeyDown(wxKeyEvent &evt)
  {
    // Convert 'delete' key into button2 (abort or uninstall)
    if(evt.GetKeyCode() == WXK_DELETE)
      doAction(select, 2);

    // Anything else is passed on
    else
      stat->onKeyDown(evt);
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

  void updateCount()
  {
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

  // Called whenever there is a chance that the underlying data for
  // the selected item has changed.
  void updateCurrentItem()
  {
    updateSelection();
    list->RefreshItem(select);
  }

  // Fix buttons for the current selected item (if any)
  void fixButtons()
  {
    if(select < 0 || select >= lister.size())
      {
        b1->Disable();
        b2->Disable();
        //supportButton->Disable();
        b1->SetLabel(wxT("No action"));
        b2->SetLabel(wxT("No action"));
        return;
      }

    DataList::Entry &e = lister.get(select);

    /*
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
    */

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

  void updateGameStatus(int index)
  {
    GameInfo &e = GameInfo::conv(lister.get(index));

    // If this is the current item, make sure the buttons are set
    // correctly.
    if(index == select) fixButtons();

    try
      {
        int i = e.updateStatus();

        if(i>0)
          statusChanged(false);
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

    // Update the .tig info first, but skip in offline mode.
    if(!conf.offline)
    {
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
    updateGameStatus(index);

    /* Update lists and moved to the Installed tab.

       NOTE: this will switch tabs even if download immediately
       fails. This isn't entirely optimal, but is rare enough that it
       doesn't matter.
     */
    statusChanged(true);
  }

  void doAction(int index, int b)
  {
    if(index < 0 || index >= lister.size())
      return;

    GameInfo &e = GameInfo::conv(lister.get(index));

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

            wxSetWorkingDirectory(wxString(workDir.string().c_str(), wxConvUTF8));

            wxString command = wxString(program.string().c_str(), wxConvUTF8);

            if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
              wxShell(wxT("wine \"") + command + wxT("\""));
            else
              {
                int res = wxExecute(command);
                if(res == -1)
                  errorBox(wxT("Failed to launch ") + command);
              }

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
    doAction(event.GetIndex(), 1);
  }

  void dataChanged()
  {
    lister.reset();
    createTagList();
    listHasChanged(true);
  }
};
