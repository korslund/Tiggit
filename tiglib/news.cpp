#include "news.hpp"
#include <spread/misc/readjson.hpp>
#include "repo.hpp"

#include <assert.h>
#include <set>
#include <ctime>

using namespace TigLib;

static void add(std::vector<NewsItem> &list,
                const std::string &id,
                time_t date,
                const std::string &subject,
                const std::string &body,
                bool isRead=false)
{
  NewsItem i;
  i.id = id;
  i.date = date;
  i.subject = subject;
  i.body = body;
  i.isRead = isRead;
  list.push_back(i);
}

void error(std::vector<NewsItem> &list,
           const std::string &body)
{
  add(list, "", std::time(NULL), "Error", body, true);
}

struct DateSorter
{
  bool operator()(const NewsItem &a, const NewsItem &b)
  { return a.date > b.date; }
};

void NewsReader::reload()
{
  using namespace Json;

  try
    {
      std::string file = repo->getNewsFile();

      Value root = ReadJson::readJson(file);
      if(!root.isObject())
        {
          error(items, "No news items were found");
          return;
        }

      Value::Members keys = root.getMemberNames();
      Value::Members::iterator it;
      items.clear();
      for(it = keys.begin(); it != keys.end(); it++)
        {
          const std::string &id = *it;
          Value ent = root[id];

          add(items, id, ent["date"].asUInt(),
              ent["subject"].asString(), ent["body"].asString());
        }

      // Create a lookup of read items
      std::set<std::string> haveRead;
      std::vector<std::string> names =
        repo->news.getNames();
      for(int i=0; i<names.size(); i++)
        haveRead.insert(names[i]);

      // Apply status
      for(int i=0; i<items.size(); i++)
        if(haveRead.count(items[i].id))
          items[i].isRead = true;

      // Finally, sort items by date
      sort(items.begin(), items.end(), DateSorter());
    }
  catch(std::exception &e)
    { error(items, e.what()); }
  catch(...)
    { error(items, "Unknown error"); }
}

void NewsReader::markAsRead(int i)
{
  assert(i >= 0 && i < size());
  NewsItem &it = items[i];
  it.isRead = true;
  repo->news.setBool(it.id, true);
}

void NewsReader::markAllAsRead()
{
  for(int i=0; i<items.size(); i++)
    markAsRead(i);
}
