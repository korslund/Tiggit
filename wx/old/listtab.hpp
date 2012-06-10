// MOVE : Open Location
      /*
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
      */


/* MOVE: Launch game

      // TODO: All these path and file operations could be out-
      // sourced to an external module. One "repository" module that
      // doesn't know about the rest of the program is the best bet.

      // Construct the install path
      boost::filesystem::path dir = conf.gamedir;
      dir /= e.entry.idname;
      dir = get.getPath(dir.string());

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

/* MOVE: Uninstall
      // TODO: All these path and file operations could be out-
      // sourced to an external module. One "repository" module that
      // doesn't know about the rest of the program is the best bet.

      // Construct the install path
      boost::filesystem::path dir = conf.gamedir;
      dir /= e.entry.idname;
      dir = get.getPath(dir.string());

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
  */

{
  time_t last_launch;
  std::vector<TagSorter::Entry> taglist;
  last_launch(0)

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
