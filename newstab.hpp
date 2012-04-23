#ifndef __NEWSTAB_HPP_
#define __NEWSTAB_HPP_

#include "tabbase.hpp"
#include <wx/listctrl.h>
#include <vector>
#include <algorithm>
#include <time.h>

struct NewsItem
{
  time_t dateNum;
  wxString date, subject, body;
  bool read;

  bool operator<(const NewsItem &other) const
  {
    // Place newest items first
    return dateNum > other.dateNum;
  }
};

struct NewsHolder
{
  std::vector<NewsItem> items;

  void addItem(time_t time, const wxString &subject, const wxString &body, bool read)
  {
    NewsItem e;

    char buf[50];
    strftime(buf,50, "%Y-%m-%d", gmtime(&time));
    e.date = wxString(buf, wxConvUTF8);
    e.dateNum = time;
    e.subject = subject;
    e.body = body;
    e.read = read;

    items.push_back(e);
  }

  void sort()
  {
    std::sort(items.begin(), items.end());
  }
};

class NewsList : public wxListCtrl
{
  wxListItemAttr unreadStyle, readStyle;
  NewsHolder *news;

public:
  NewsList(wxWindow *parent, int ID)
    : wxListCtrl(parent, ID, wxDefaultPosition, wxDefaultSize,
                 wxBORDER_SUNKEN | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL),
      news(NULL)
  {
    /*
    wxFont fnt = unreadStyle.GetFont();
    fnt.SetWeight(wxFONTWEIGHT_BOLD);
    unreadStyle.SetFont(fnt);
    //*/

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

  void setData(NewsHolder &n)
  {
    news = &n;
    SetItemCount(news->items.size());
  }

  NewsItem *getItem(int item) const
  {
    if(!news || item < 0 || item >= news->items.size())
      return NULL;

    return &news->items[item];
  }

  wxListItemAttr *OnGetItemAttr(long item) const
  {
    const NewsItem *i = getItem(item);
    if(!i) return NULL;

    if(i->read) return (wxListItemAttr*)&readStyle;
    return (wxListItemAttr*)&unreadStyle;
  }

  wxString OnGetItemText(long item, long column) const
  {
    NewsItem *i = getItem(item);
    if(!i) return wxT("");

    if(column == 0)
      return i->date;
    return i->subject;
  }

  // Mark item as read. Return true if status was changed.
  bool markAsRead(int item)
  {
    NewsItem *i = getItem(item);
    if(!i) return false;

    if(!i->read)
      {
        i->read = true;
        Refresh();
        return true;
      }
    return false;
  }
};

#define myID_NEWSLIST 10043
#define myID_NEWSVIEW 10044

// The "Latest News" tab
struct NewsTab : TabBase
{
  NewsList *list;
  wxTextCtrl *textView;
  NewsHolder data;

  NewsTab(wxNotebook *parent)
    : TabBase(parent)
  {
    list = new NewsList(this, myID_NEWSLIST);

    textView = new wxTextCtrl
      (this, myID_NEWSVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
       /*wxBORDER_NONE | */wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    buttons->Add(new wxButton(this, 0, wxT("Mark all as read")),
                 0, wxLEFT | wxTOP | wxBOTTOM, 5);
    buttons->Add(new wxButton(this, 0, wxT("Refresh list")),
                 0, wxLEFT | wxTOP | wxBOTTOM, 5);

    wxBoxSizer *views = new wxBoxSizer(wxHORIZONTAL);
    views->Add(list, 1, wxGROW | wxALL, 5);
    views->Add(textView, 1, wxGROW | wxALL, 5);

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

    data.addItem(time(NULL), wxT("Test 1"), wxT("http://tiggit.net/"), false);
    data.addItem(time(NULL)-60*60*24*2, wxT("Test 2"), wxT("Test2"), true);
    data.addItem(time(NULL)+60*60*24*2, wxT("Test 3"), wxT("Test3"), false);
    data.sort();
    list->setData(data);
  }

  void onListSelect(wxListEvent &event)
  {
    int item = event.GetIndex();

    NewsItem *i = list->getItem(item);
    if(!i)
      onListDeselect(event);

    if(list->markAsRead(item))
      updateTabName();

    textView->ChangeValue(i->body);
  }

  void updateTabName()
  {
    // Find number of unread items
    int unread = 0;
    for(int i=0; i<data.items.size(); i++)
      if(!data.items[i].read)
        unread++;

    wxString name = wxT("Latest News");
    if(unread) name += wxString::Format(wxT(" (%d)"), unread);
    book->SetPageText(tabNum, name);
  }

  void onListDeselect(wxListEvent &event)
  {
    textView->Clear();
  }

  // Respond to clickable URLs in the game description
  void onUrlEvent(wxTextUrlEvent &event)
  {
    if(!event.GetMouseEvent().ButtonDown(wxMOUSE_BTN_LEFT))
      return;

    wxString url = textView->GetRange(event.GetURLStart(), event.GetURLEnd());
    wxLaunchDefaultBrowser(url);
  }

  void takeFocus()
  {}

  void dataChanged() {}

  void insertMe()
  {
    TabBase::insertMe();
    updateTabName();
  }
};

#endif
