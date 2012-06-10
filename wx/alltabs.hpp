/* This is a direct include file from frame.cpp. It is not meant to be
   included anywhere else.
 */
struct TitleCol : ColumnHandler
{
  bool addStatus;

  TitleCol(bool as=false) : addStatus(as) {}

  bool doSort(wxGameList &lst) { return lst.sortTitle(); }
  wxString getText(const wxGameInfo &e)
  {
    return e.getTitle(addStatus);
  }
};

struct AddDateCol : ColumnHandler
{
  bool doSort(wxGameList &lst) { return lst.sortDate(); }
  wxString getText(const wxGameInfo &e)
  {
    return e.timeString();
  }
};

struct TypeCol : ColumnHandler
{
  wxString demoText, freeText;

  TypeCol()
  {
    demoText = wxT("demo");
    freeText = wxT("free");
  }

  bool doSort(wxGameList &lst) { return false; }
  wxString getText(const wxGameInfo &e)
  {
    if(e.isDemo())
      return demoText;
    return freeText;
  }
};

struct RatingCol : ColumnHandler
{
  bool doSort(wxGameList &lst) { return lst.sortRating(); }
  wxString getText(const wxGameInfo &e)
  {
    return e.rateString();
  }
};

struct DownloadsCol : ColumnHandler
{
  bool doSort(wxGameList &lst) { return lst.sortDownloads(); }
  wxString getText(const wxGameInfo &e)
  {
    return e.dlString();
  }
};

struct StatusCol : ColumnHandler
{
  wxString textNotInst, textReady;

  bool doSort(wxGameList &lst) { return false; }
  StatusCol()
  {
    textNotInst = wxT("Not installed");
    textReady = wxT("Ready to play");
  }

  wxString getText(const wxGameInfo &e)
  {
    if(e.isUninstalled())
      return textNotInst;

    if(e.isInstalled())
      return textReady;

    return e.statusString();
  }
};

struct AllGamesTab : GameTab
{
  AllGamesTab(wxNotebook *parent, wxGameData &data)
    : GameTab(parent, wxT("All Games"), data.getAllList())
  {
    list->addColumn("Name", 310, new TitleCol);
    list->addColumn("Status", 170, new StatusCol);
  }
};

// Lists newly added games
struct NewGamesTab : GameTab
{
  int newGames;

  NewGamesTab(wxNotebook *parent, wxGameData &data)
    : GameTab(parent, wxT("Latest"), data.getAllList()),
      newGames(0)
  {
    list->addColumn("Name", 370, new TitleCol(true));
    list->addColumn("Added", 120, new AddDateCol);
    list->markNew = true;

    lister.sortDate();
  }

  int getTitleNumber()
  {
    newGames = 0;
    // Count number of new games
    /* TODO
    const std::vector<int> &base = lister.getBaseList();
    for(int i=0; i<base.size(); i++)
      if(data.arr[base[i]].isNew)
        newGames++;
    */
    return newGames;
  }
};

/*
// Lists freeware games
struct FreewareTab : GameTab
{
  FreewareTab(wxNotebook *parent, StatusNotify *s)
    : GameTab(parent, wxT("Browse"), ListKeeper::SL_FREEWARE, s)
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
    GameTab::insertMe();
  }
};

struct DemoTab : GameTab
{
  DemoTab(wxNotebook *parent, StatusNotify *s)
    : GameTab(parent, wxT("Demos"), ListKeeper::SL_DEMOS, s)
  {
    list->addColumn(wxT("Name"), 310, new TitleCol(true));
    list->addColumn(wxT("Rating"), 95, new RatingCol);
    list->addColumn(wxT("Downloads"), 75, new DownloadsCol);

    list->markInstalled = true;

    lister.sortRating();
    listHasChanged();
  }
};

struct InstalledTab : GameTab
{
  InstalledTab(wxNotebook *parent, StatusNotify *s)
    : GameTab(parent, wxT("Installed"), ListKeeper::SL_INSTALL, s)
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
*/
