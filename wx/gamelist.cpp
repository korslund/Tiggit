#include "gamelist.hpp"

using namespace wxTiggit;

void ColumnHandler::sort(wxGameList &lst)
{
  if(doSort(lst))
    lst.flipReverse();
  else
    lst.setReverse(false);
}

GameListView::GameListView(wxWindow *parent, int id, wxGameList &lst)
  : ListBase(parent, id), lister(lst), markNew(false), markInstalled(false)
{
  orange.SetBackgroundColour(wxColour(255,240,180));
  orange.SetTextColour(wxColour(0,0,0));

  wxFont fnt = bold.GetFont();
  fnt.SetWeight(wxFONTWEIGHT_BOLD);
  bold.SetFont(fnt);

  fnt = gray.GetFont();
  fnt.SetStyle(wxFONTSTYLE_ITALIC);
  gray.SetFont(fnt);
  gray.SetTextColour(wxColour(128,128,128));

  Connect(wxEVT_COMMAND_LIST_COL_CLICK,
          wxListEventHandler(GameListView::onHeaderClick));

  // Get updates when the list changes
  lister.addListener(this);

  // Do a complete data refresh
  gameListChanged();
}

// Handle column header clicks
void GameListView::onHeaderClick(wxListEvent& event)
{
  int col = event.GetColumn();
  if(col < 0 || col >= colHands.size())
    return;

  // Let the column handler sort our list for us
  colHands[col]->sort(lister);
}

void GameListView::addColumn(const std::string &name, int width, ColumnHandler *ch)
{
  int colNum = colHands.size();
  colHands.push_back(ch);

  wxListItem col;
  col.SetId(colNum);
  col.SetText(strToWx(name));
  col.SetWidth(width);
  InsertColumn(colNum, col);
}

void GameListView::gameInfoChanged() { Refresh(); }
void GameListView::gameStatusChanged() { Refresh(); }
void GameListView::gameListChanged() { updateSize(); }
void GameListView::gameSelectionChanged() { updateSize(); }

// Refresh list size.
void GameListView::updateSize()
{
  SetItemCount(lister.size());
  SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  Refresh();
}

wxListItemAttr *GameListView::OnGetItemAttr(long item) const
{
  if(item < 0 || item >= lister.size())
    return NULL;

  const wxGameInfo &g = lister.get(item);

  if(markNew)
    {
      if(!g.isUninstalled())
        return (wxListItemAttr*)&gray;
      if(g.isNew())
        return (wxListItemAttr*)&bold;
      return NULL;
    }

  if(g.isUninstalled()) return NULL;

  // Status here is either installed or working
  if(g.isInstalled())
    {
      if(markInstalled)
        // Gray out installed games in this list
        return (wxListItemAttr*)&gray;
      else
        return NULL;
    }

  // Installing entries are marked in orange
  return (wxListItemAttr*)&orange;
}

wxString GameListView::OnGetItemText(long item, long column) const
{
  if(column < 0 || column >= colHands.size() ||
     item < 0 || item >= lister.size())
    return wxT("No info");

  assert(column >= 0 && column < colHands.size());
  ColumnHandler *h = colHands[column];
  assert(h);
  return h->getText(lister.get(item));
}
