#ifndef __TIGLIB_NEWS_HPP_
#define __TIGLIB_NEWS_HPP_

#include <string>
#include <vector>

namespace TigLib
{
  struct NewsItem
  {
    time_t date;
    std::string id, subject, body;
    bool isRead;
  };

  class Repo;
  struct NewsReader
  {
    NewsReader(Repo *_repo) : repo(_repo) {}

    // Fetch from the net and load into memory
    void reload();

    // Mark one or all items as read
    void markAsRead(int);
    void markAllAsRead();

    // Get news items
    int size() const { return items.size(); }
    const NewsItem &get(int i) const { return items[i]; }

  private:
    std::vector<NewsItem> items;
    Repo *repo;
  };
}

#endif
