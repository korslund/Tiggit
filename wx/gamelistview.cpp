#include "gamelistview.hpp"

using namespace wxTiggit;

void ColumnHandler::sort(wxGameList &lst)
{
  if(doSort(lst))
    lst.flipReverse();
  else
    lst.setReverse(false);
}

GameListView::GameListView(wxWindow *parent, int id, wxGameList &lst)
  : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
               wxBORDER_SUNKEN | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL),
    lister(lst), markNew(false), markInstalled(false)
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
  Connect(wxEVT_KEY_DOWN,
          wxKeyEventHandler(GameListView::onKeyDown));

  // Get updates when the list changes
  lister.addListener(this);

  // Do a complete data refresh
  gameListReloaded();
}

void GameListView::onKeyDown(wxKeyEvent &evt)
{
  // Capture special keys
  if(evt.GetKeyCode() == WXK_LEFT)
    {}
  else if(evt.GetKeyCode() == WXK_RIGHT)
    {}
  else if(evt.GetKeyCode() == WXK_DELETE)
    {}
  else
    evt.Skip();
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

void GameListView::gameStatusChanged() { Refresh(); }
void GameListView::gameInfoChanged() { Refresh(); }
void GameListView::gameListChanged() { updateSize(); }
void GameListView::gameListReloaded() { updateSize(); }

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

  if(markInstalled)
    // Gray out installed/installing games in this list
    return (wxListItemAttr*)&gray;

  if(g.isInstalled()) return NULL;
  return (wxListItemAttr*)&orange;
}

int GameListView::OnGetItemImage(long item) const
{
  return -1;
}

int GameListView::OnGetColumnImage(long item, long column) const
{
  return -1;
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
