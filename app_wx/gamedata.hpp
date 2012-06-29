#ifndef __WXAPP_GAMEDATA_HPP_
#define __WXAPP_GAMEDATA_HPP_

#include "gamelist.hpp"
#include "tiglib/repo.hpp"
#include "gameconf.hpp"
#include "tiglib/news.hpp"

namespace wxTigApp
{
  struct GameNews : wxGameNews
  {
    TigLib::NewsReader news;
    std::vector<wxGameNewsItem> items;

    GameNews(TigLib::Repo *repo) : news(repo) {}

    const wxGameNewsItem &get(int i) const { return items[i]; }
    int size() const { return items.size(); }
    void reload();
    void markAsRead(int);
    void markAllAsRead();
  };

  struct GameData : wxGameData
  {
    GameList *latest, *freeware, *demos, *installed;

    GameConf config;
    GameNews news;
    TigLib::Repo &repo;

    GameData(TigLib::Repo &rep);
    ~GameData();

    wxGameList &getLatest() { return *latest; }
    wxGameList &getFreeware() { return *freeware; }
    wxGameList &getDemos() { return *demos; }
    wxGameList &getInstalled() { return *installed; }

    // Called when a game has started or finished installing, or has
    // been uninstalled.
    void installStatusChanged()
    {
      // Notify main lists that their views should be updated
      latest->notifyInfoChange();
      freeware->notifyInfoChange();
      demos->notifyInfoChange();

      // Refresh the installed list. Notifications will happen
      // automatically.
      installed->lister.refresh();
    }

    // Called regularly when there are games being installed, to
    // update display status.
    void updateDisplayStatus()
    {
      latest->notifyStatusChange();
      freeware->notifyStatusChange();
      demos->notifyStatusChange();
      installed->notifyStatusChange();
    }

    wxGameConf &conf() { return config; }
    wxGameNews &getNews() { return news; }
  };
}
#endif