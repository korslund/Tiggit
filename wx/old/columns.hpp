struct TitleCol : ColumnHandler
{
  bool addStatus;

  TitleCol(bool as = false) : addStatus(as) {}

  bool doSort(ListKeeper &lst) { return lst.sortTitle(); }
  wxString getText(GameInfo &e)
  {
    if(addStatus)
      {
        if(e.isInstalled())
          return e.name + wxT(" [installed]");
        else if(e.isWorking())
          return e.name + wxT(" [installing]");
      }

    return e.name;
  }
};

struct AddDateCol : ColumnHandler
{
  bool doSort(ListKeeper &lst) { return lst.sortDate(); }
  wxString getText(GameInfo &e)
  {
    return e.timeString;
  }
};

struct TypeCol : ColumnHandler
{
  bool doSort(ListKeeper &lst) { return false; }
  wxString getText(GameInfo &e)
  {
    if(e.entry.tigInfo.isDemo)
      return wxT("demo");
    return wxT("freeware");
  }
};

struct RatingCol : ColumnHandler
{
  bool doSort(ListKeeper &lst) { return lst.sortRating(); }
  wxString getText(GameInfo &e)
  {
    if(conf.voteCount)
      return e.rating2;
    else
      return e.rating;
  }
};

struct DownloadsCol : ColumnHandler
{
  bool doSort(ListKeeper &lst) { return lst.sortDownloads(); }
  wxString getText(GameInfo &e)
  {
    return e.dlCount;
  }
};

struct PriceCol : ColumnHandler
{
  bool doSort(ListKeeper &lst) { return false; }
  wxString getText(GameInfo &e)
  {
    return e.price;
  }
};

struct StatusCol : ColumnHandler
{
  wxString textNotInst, textReady;

  bool doSort(ListKeeper &lst) { return false; }
  StatusCol()
  {
    textNotInst = wxT("Not installed");
    textReady = wxT("Ready to play");
  }

  wxString getText(GameInfo &g)
  {
    if(g.isNone())
      return textNotInst;

    if(g.isInstalled())
      return textReady;

    if(g.isWorking())
      return g.getStatus();

    assert(0);
  }
};
