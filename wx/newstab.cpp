#include "newstab.hpp"
#include "listbase.hpp"

using namespace wxTiggit;

namespace wxTiggit {
class NewsList : public ListBase
{
  wxListItemAttr unreadStyle, readStyle;
  wxGameNews &news;

public:

  NewsList(wxWindow *parent, int ID, wxGameNews &data)
    : ListBase(parent, ID), news(data)
  {
    wxFont fnt = unreadStyle.GetFont();
    fnt.SetWeight(wxFONTWEIGHT_BOLD);
    unreadStyle.SetFont(fnt);
    readStyle.SetTextColour(wxColour(128,128,128));

    wxListItem col;
    col.SetId(0);
    col.SetText(wxT("Date"));
    col.SetWidth(120);
    InsertColumn(0, col);

    col.SetId(1);
    col.SetText(wxT("Subject"));
    col.SetWidth(400);
    InsertColumn(1, col);
  }

  void updateNews()
  {
    news.reload();
    SetItemCount(news.size());
  }

  void takeFocus()
  {
    // This marks items as 'read', which is not what we want.
    //SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    SetFocus();
  }

  const wxGameNewsItem *getItem(int item) const
  {
    if(item < 0 || item >= news.size())
      return NULL;

    return &news.get(item);
  }

  wxListItemAttr *OnGetItemAttr(long item) const
  {
    const wxGameNewsItem *i = getItem(item);
    if(!i) return NULL;

    if(i->read) return (wxListItemAttr*)&readStyle;
    return (wxListItemAttr*)&unreadStyle;
  }

  wxString OnGetItemText(long item, long column) const
  {
    const wxGameNewsItem *i = getItem(item);
    if(!i) return wxT("");

    if(column == 0)
      return i->date;
    return i->subject;
  }

  // Mark item as read. Return true if status was changed.
  bool markAsRead(int item)
  {
    const wxGameNewsItem *i = getItem(item);
    if(!i) return false;

    if(!i->read)
      {
        news.markAsRead(item);
        Refresh();
        return true;
      }
    return false;
  }

  void markAllAsRead()
  {
    news.markAllAsRead();
    Refresh();
  }

  int unreadNum()
  {
    // Find number of unread items
    int unread = 0;
    for(int i=0; i<news.size(); i++)
      if(!news.get(i).read)
        unread++;
    return unread;
  }
};
}

#define myID_NEWSLIST 10043
#define myID_NEWSVIEW 10044
#define myID_BMARKALL 10046
#define myID_TIGHOME  10060
#define myID_TIGFORUM 10061
#define myID_TIGTWIT  10063

NewsTab::NewsTab(wxNotebook *parent, wxGameData &data)
  : TabBase(parent, wxT("News"))
{
  list = new NewsList(this, myID_NEWSLIST, data.getNews());

  textView = new wxTextCtrl
    (this, myID_NEWSVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
     wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

  wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
  buttons->Add(new wxButton(this, myID_BMARKALL, wxT("Mark all as read")),
               0, wxLEFT | wxTOP | wxBOTTOM, 5);
  buttons->Add(new wxStaticText(this, wxID_ANY, wxT("       Visit:")),
               0, wxLEFT | wxTOP, 11);
  buttons->Add(new wxButton(this, myID_TIGHOME, wxT("Tiggit.net")),
               0, wxLEFT | wxTOP | wxBOTTOM, 5);
  buttons->Add(new wxButton(this, myID_TIGFORUM, wxT("Tiggit Forum")),
               0, wxLEFT | wxTOP | wxBOTTOM, 5);
  buttons->Add(new wxButton(this, myID_TIGTWIT, wxT("Twitter")),
               0, wxLEFT | wxTOP | wxBOTTOM, 5);

  wxBoxSizer *views = new wxBoxSizer(wxHORIZONTAL);
  views->Add(list, 1, wxGROW | wxLEFT | wxBOTTOM, 5);
  views->Add(textView, 1, wxGROW | wxBOTTOM | wxRIGHT | wxLEFT, 5);

  wxBoxSizer *all = new wxBoxSizer(wxVERTICAL);
  all->Add(buttons);
  all->Add(views, 1, wxGROW);
  SetSizer(all);

  Connect(myID_NEWSVIEW, wxEVT_COMMAND_TEXT_URL,
          wxTextUrlEventHandler(NewsTab::onUrlEvent));

  Connect(myID_NEWSLIST, wxEVT_COMMAND_LIST_ITEM_SELECTED,
          wxListEventHandler(NewsTab::onListSelect));
  Connect(myID_NEWSLIST, wxEVT_COMMAND_LIST_ITEM_DESELECTED,
          wxListEventHandler(NewsTab::onListDeselect));

  Connect(myID_BMARKALL, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewsTab::onReadAll));

  Connect(myID_TIGHOME, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewsTab::onWebsite));
  Connect(myID_TIGFORUM, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewsTab::onWebsite));
  Connect(myID_TIGTWIT, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NewsTab::onWebsite));

  reloadData();
}

void NewsTab::reloadData()
{
  list->updateNews();
  updateTitle();
}

void NewsTab::onWebsite(wxCommandEvent &event)
{
  int id = event.GetId();

  wxString url;

  if(id == myID_TIGHOME) url = wxT("http://tiggit.net/");
  if(id == myID_TIGFORUM) url = wxT("http://tiggit.net/forum/");
  if(id == myID_TIGTWIT) url = wxT("https://twitter.com/tiggitdev");

  wxLaunchDefaultBrowser(url);
}

void NewsTab::onReadAll(wxCommandEvent &event)
{
  list->markAllAsRead();
  updateTitle();
}

void NewsTab::onListSelect(wxListEvent &event)
{
  int item = event.GetIndex();

  const wxGameNewsItem *i = list->getItem(item);
  if(!i)
    onListDeselect(event);

  if(list->markAsRead(item))
    updateTitle();

  textView->ChangeValue(i->body);
}

int NewsTab::getTitleNumber()
{
  return list->unreadNum();
}

void NewsTab::onListDeselect(wxListEvent &event)
{
  textView->Clear();
}

// Respond to clickable URLs in the game description
void NewsTab::onUrlEvent(wxTextUrlEvent &event)
{
  if(!event.GetMouseEvent().ButtonDown(wxMOUSE_BTN_LEFT))
    return;

  wxString url = textView->GetRange(event.GetURLStart(), event.GetURLEnd());
  wxLaunchDefaultBrowser(url);
}

void NewsTab::gotFocus()
{
  list->takeFocus();
}
