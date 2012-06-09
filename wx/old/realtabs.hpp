// List tab that only displays itself when non-empty
/*
struct ListTabNonEmpty : ListTab
{
  ListTabNonEmpty(wxNotebook *parent, const wxString &name, int listType,
                  StatusNotify *s)
    : ListTab(parent, name, listType, s)
  {}

  void insertMe()
  {
    tabNum = -1;

    if(lister.baseSize() > 0)
      {
        Show();
        ListTab::insertMe();
      }
    else
      Hide();
  }
};
*/

// Lists newly added games
struct NewGameListTab : ListTab
{
  int newGames;

  NewGameListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Latest"), ListKeeper::SL_ALL, s),
      newGames(0)
  {
    list->addColumn(wxT("Name"), 370, new TitleCol(true));
    list->addColumn(wxT("Added"), 120, new AddDateCol);
    //list->addColumn(wxT("Type"), 67, new TypeCol);

    list->markNew = true;

    lister.sortDate();
    listHasChanged();
  }

  void insertMe()
  {
    TabBase::insertMe();

    // Count number of new games
    const std::vector<int> &base = lister.getBaseList();
    newGames = 0;
    for(int i=0; i<base.size(); i++)
      if(data.arr[base[i]].isNew)
        newGames++;

    wxString name = tabName;
    if(newGames)
      name += wxString::Format(wxT(" (%d)"), newGames);

    book->SetPageText(tabNum, name);
  }
};

// Lists freeware games
struct FreewareListTab : ListTab
{
  FreewareListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Browse"), ListKeeper::SL_FREEWARE, s)
  {
    list->addColumn(wxT("Name"), 310, new TitleCol(true));
    list->addColumn(wxT("Rating"), 95, new RatingCol);
    list->addColumn(wxT("Downloads"), 75, new DownloadsCol);

    list->markInstalled = true;

    lister.sortRating();
    listHasChanged();
  }

  // Temporary development hack
  void insertMe()
  {
    tabName = wxT("Freeware");
    ListTab::insertMe();
  }
};

struct DemoListTab : ListTab
{
  DemoListTab(wxNotebook *parent, StatusNotify *s)
    : ListTabNonEmpty(parent, wxT("Demos"), ListKeeper::SL_DEMOS, s)
  {
    list->addColumn(wxT("Name"), 310, new TitleCol(true));
    list->addColumn(wxT("Rating"), 95, new RatingCol);
    list->addColumn(wxT("Downloads"), 75, new DownloadsCol);

    list->markInstalled = true;

    lister.sortRating();
    listHasChanged();
  }
};

struct InstalledListTab : ListTab
{
  InstalledListTab(wxNotebook *parent, StatusNotify *s)
    : ListTab(parent, wxT("Installed"), ListKeeper::SL_INSTALL, s)
  {
    list->addColumn(wxT("Name"), 310, new TitleCol);
    list->addColumn(wxT("Status"), 170, new StatusCol);
  }

  // This needs to die
  void tick()
  {
    for(int i=0; i<lister.size(); i++)
      updateGameStatus(i);
  }
};
