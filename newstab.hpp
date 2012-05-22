#ifndef __NEWSTAB_HPP_
#define __NEWSTAB_HPP_

#include "tabbase.hpp"
#include <wx/listctrl.h>
#include <vector>
#include <algorithm>
#include <set>
#include <time.h>
#include "filegetter.hpp"
#include "readjson.hpp"

struct NewsItem
{
  int id;
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

  void addItem(int id, time_t time, const std::string &sub, const std::string &body, bool isRead=false)
  {
    addItem(id, time,
            wxString(sub.c_str(), wxConvUTF8),
            wxString(body.c_str(), wxConvUTF8),
            isRead);
  }

  void addItem(int id, time_t time, const wxString &subject, const wxString &body, bool isRead=false)
  {
    NewsItem e;

    char buf[50];
    strftime(buf,50, "%Y-%m-%d", gmtime(&time));
    e.date = wxString(buf, wxConvUTF8);

    e.id = id;
    e.dateNum = time;
    e.subject = subject;
    e.body = body;
    e.read = isRead;

    items.push_back(e);
  }

  void writeStatus()
  {
    using namespace Json;

    Value v;

    for(int i=0; i<items.size(); i++)
      if(items[i].read)
        v.append(items[i].id);

    writeJson(get.getPath("readnews.json"), v);
  }

  void reload()
  {
    using namespace Json;
    using namespace std;

    items.resize(0);

    // TODO: This is just a temporary solution until we implement the
    // new data fetching architecture.
    try
      {
        if(conf.offline)
          {
            addItem(0, time(NULL), "Error loading news", "Could not load news in offline mode", true);
            return;
          }

        Value root = readJson(get.getTo("http://tiggit.net/api/news.json", "news.json"));

        if(!root.isObject())
          {
            addItem(0, time(NULL), "No news", "No news items were found", true);
            return;
          }

        Value::Members keys = root.getMemberNames();
        Value::Members::iterator it;
        for(it = keys.begin(); it != keys.end(); it++)
          {
            const string &id = *it;
            Value ent = root[id];

            addItem(atoi(id.c_str()),
                    ent["date"].asUInt(),
                    ent["subject"].asString(),
                    ent["body"].asString());
          }

        // Apply read-status
        try
          {
            root = readJson(get.getPath("readnews.json"));

            std::set<int> haveRead;

            for(int i=0; i<root.size(); i++)
              haveRead.insert(root[i].asUInt());

            // Apply status
            for(int i=0; i<items.size(); i++)
              if(haveRead.count(items[i].id))
                items[i].read = true;
          }
        // Ignore missing readnews.json file
        catch(...) {}

        sort(items.begin(), items.end());
      }
    catch(std::exception &e)
      {
        addItem(0, time(NULL), "Error loading news", e.what(), true);
      }
    catch(...)
      {
        addItem(0, time(NULL), "Error loading news", "Unknown error", true);
      }
  }
};

class NewsList : public wxListCtrl
{
  wxListItemAttr unreadStyle, readStyle;
  NewsHolder *news;
  KeyAccel *keys;

public:
  NewsList(wxWindow *parent, int ID, KeyAccel *k)
    : wxListCtrl(parent, ID, wxDefaultPosition, wxDefaultSize,
                 wxBORDER_SUNKEN | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL),
      news(NULL), keys(k)
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

    Connect(wxEVT_KEY_DOWN,
            wxKeyEventHandler(NewsList::onKeyDown));
  }

  void onKeyDown(wxKeyEvent &evt)
  {
    assert(keys);
    keys->onKeyDown(evt);
  }

  void setData(NewsHolder &n)
  {
    news = &n;
    n.reload();
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
        news->writeStatus();
        return true;
      }
    return false;
  }
};

#define myID_NEWSLIST 10043
#define myID_NEWSVIEW 10044
#define myID_BGETNEWS 10045
#define myID_BMARKALL 10046
#define myID_TIGHOME  10060
#define myID_TIGFORUM 10061
#define myID_TIGBLOG  10062

// The "Latest News" tab
struct NewsTab : TabBase
{
  NewsList *list;
  wxTextCtrl *textView;
  NewsHolder data;

  NewsTab(wxNotebook *parent, KeyAccel *keys)
    : TabBase(parent)
  {
    list = new NewsList(this, myID_NEWSLIST, keys);

    textView = new wxTextCtrl
      (this, myID_NEWSVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
       /*wxBORDER_NONE | */wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    buttons->Add(new wxButton(this, myID_BMARKALL, wxT("Mark all as read")),
                 0, wxLEFT | wxTOP | wxBOTTOM, 5);
    buttons->Add(new wxButton(this, myID_BGETNEWS, wxT("Refresh list")),
                 0, wxLEFT | wxTOP | wxBOTTOM, 5);
    buttons->Add(new wxStaticText(this, wxID_ANY, wxT("       Visit:")),
                 0, wxLEFT | wxTOP, 11);
    buttons->Add(new wxButton(this, myID_TIGHOME, wxT("Tiggit.net")),
                 0, wxLEFT | wxTOP | wxBOTTOM, 5);
    buttons->Add(new wxButton(this, myID_TIGFORUM, wxT("Tiggit Forum")),
                 0, wxLEFT | wxTOP | wxBOTTOM, 5);
    buttons->Add(new wxButton(this, myID_TIGBLOG, wxT("Tiggit Blog")),
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
    Connect(myID_BGETNEWS, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(NewsTab::onGetNews));

    Connect(myID_TIGHOME, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(NewsTab::onWebsite));
    Connect(myID_TIGFORUM, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(NewsTab::onWebsite));
    Connect(myID_TIGBLOG, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(NewsTab::onWebsite));

    reloadData();
  }

  void reloadData()
  {
    list->setData(data);
  }

  void onWebsite(wxCommandEvent &event)
  {
    int id = event.GetId();

    wxString url;

    if(id == myID_TIGHOME) url = wxT("http://tiggit.net/");
    if(id == myID_TIGFORUM) url = wxT("http://tiggit.net/forum/");
    if(id == myID_TIGBLOG) url = wxT("http://tiggit.net/blog");

    wxLaunchDefaultBrowser(url);
  }

  void onGetNews(wxCommandEvent &event)
  {
    reloadData();
  }

  void onReadAll(wxCommandEvent &event)
  {
    for(int i=0; i<data.items.size(); i++)
      data.items[i].read = true;
    list->Refresh();
    data.writeStatus();
    updateTabName();
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
  {
    list->SetFocus();
  }

  void dataChanged() {}

  void insertMe()
  {
    TabBase::insertMe();
    updateTabName();
  }
};

#endif
