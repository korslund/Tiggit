#include <vector>
#include <time.h>

struct NewsItem
{
  int id;
  time_t dateNum;
  wxString date, subject, body;
  bool read;
};

struct NewsHolder
{
  wxGameNews &orig;

  NewsHolder(wxGameNews &data) : orig(data) {}

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

  // Get fresh information from the source
  void reload()
  {
    orig.fetchData();

    items.resize(0);
    for(int i=0; i<orig.size(); i++)
      {
        const wxGameNewsItem &item = orig.get(i);
        addItem(item.id, item.time, item.subject, item.body, item.isRead);
      }
  }

  // Store current status
  void storeStatus()
  {
    for(int i=0; i<items.size(); i++)
      if(items[i].read)
        orig.markAsRead(items[i].id);
    orig.storeStatus();
  }
};

#include <algorithm>

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
