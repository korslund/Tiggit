  void reset()
  {
    base.resize(0);
    base.reserve(data.arr.size());

    if(selection == SL_ALL)
      {
        // Insert the entire thing
        base.resize(data.arr.size());

        for(int i=0; i<base.size(); i++)
          base[i] = i;
      }
    else
      {
        // Loop through the database and insert matching items
        for(int i=0; i<data.arr.size(); i++)
          {
            DataList::Entry &e = data.arr[i];
            GameInfo &g = GameInfo::conv(e);

            bool keep = false;

            // Installed tab
            if(selection == SL_INSTALL)
              {
                // We list everything that's got a non-zero install
                // status
                if(g.isInstalled() || g.isWorking())
                  keep = true;
              }

            /* Installed games may not be listed anywhere else, so
               skip this game.

               UPDATE: Disabled, we now gray these out instead of
               removing them. We could make this configurable.
            */
            //else if(g.isInstalled() || g.isWorking()) continue;

            // Insert into each tab according to their own criteria
            if(selection == SL_NEW && e.isNew ||
               selection == SL_FREEWARE && !e.tigInfo.isDemo ||
               selection == SL_DEMOS && e.tigInfo.isDemo)
              keep = true;

            if(keep)
              base.push_back(i);
          }
      }
  }
