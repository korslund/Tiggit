#ifndef __WXAPP_WXGAMEDATA_HPP_
#define __WXAPP_WXGAMEDATA_HPP_

#include "wx/wxgamedata.hpp"
#include "gameinfo/tigentry.hpp"
#include "tiglib/gamelister.hpp"
#include "list/haschanged.hpp"
#include <set>

/*
  This is our backend implementation of the wx/wxgamedata.hpp abstract
  interface. The implementation uses pretty much all the other modules
  in the tiggit source set.
 */

// This header is not included in a lot of places, so this is OK.
using namespace wxTiggit;

namespace wxTigApp
{
  struct GameInf : wxGameInfo
  {
    GameInf(const TigData::TigEntry *e)
      : ent(e) { updateAll(); }

    bool isInstalled() const;
    bool isUninstalled() const;
    bool isWorking() const;
    bool isDemo() const;
    bool isNew() const;

  private:
    const TigData::TigEntry *ent;

    wxString title, titleStatus, timeStr, rateStr, rateStr2, dlStr, statusStr, desc;

    // Used to update cached wxStrings from source data
    void updateStatus();
    void updateAll();

    /* TODO: The backend should have a single-game status update
       callback.
     */

    wxString getTitle(bool includeStatus=false) const
    { return includeStatus?titleStatus:title; }
    wxString timeString() const { return timeStr; }
    wxString rateString() const { return rateStr; }
    wxString dlString() const { return dlStr; }
    wxString statusString() const { return statusStr; }
    wxString getDesc() const { return desc; }

    std::string getHomepage() const;
    std::string getTiggitPage() const;
    std::string getIdName() const;
    std::string getDir() const;
    int myRating() const;

    void rateGame(int i);
    void requestShot(wxScreenshotCallback*);

    void installGame();
    void uninstallGame();
    void launchGame();
    void abortJob();
  };

  struct GameList;
  struct Notifier : List::HasChanged
  {
    GameList *lst;
    Notifier(List::ListBase &list, GameList *l)
      : List::HasChanged(&list), lst(l) {}

    void notify();
  };

  struct GameList : wxGameList
  {
    TigLib::GameLister lister;
    Notifier notif;

    enum SortStatus
      {
        SS_NONE, SS_TITLE, SS_DATE, SS_RATING, SS_DOWNLOADS
      };
    int sortStatus;
    bool setStat(int i)
    {
      int old = sortStatus;
      sortStatus = i;
      return i == old;
    }

    std::set<wxGameListener*> listeners;

    GameList(List::ListBase &list, TigLib::GamePicker *pick)
      : lister(list, pick), notif(lister.topList(), this),
        sortStatus(SS_NONE) {}

    void addListener(wxGameListener *p);
    void removeListener(wxGameListener *p);

    // Invoke gameListChanged() on our listeners
    void notifyListChange();

    // Invoke gameInfoChanged() on our listeners
    void notifyInfoChange();

    void flipReverse();
    void setReverse(bool);

    void clearTags();
    void setTags(const std::string &);
    void setSearch(const std::string &);

    bool sortTitle();
    bool sortDate();
    bool sortRating();
    bool sortDownloads();

    int size() const;
    const wxGameInfo& get(int i) { return edit(i); }
    wxGameInfo& edit(int);
  };

  struct GameConf : wxGameConf
  {
    bool getShowVotes();
    void setShowVotes(bool);
  };

  struct GameNews : wxGameNews
  {
    const wxGameNewsItem &get(int i) const;
    int size() const;
    void reload();
    void markAsRead(int);
    void markAllAsRead();
  };

  struct GameData : wxGameData
  {
    GameList latest, freeware, demos, installed;

    GameConf config;
    GameNews news;

    GameData();

    wxGameList &getLatest() { return latest; }
    wxGameList &getFreeware() { return freeware; }
    wxGameList &getDemos() { return demos; }
    wxGameList &getInstalled() { return installed; }

    void installStatusChanged()
    {
      // Notify main lists that their views should be updated
      latest.notifyInfoChange();
      freeware.notifyInfoChange();
      demos.notifyInfoChange();

      // Refresh the installed list. Notifications will happen
      // automatically.
      installed.lister.refresh();
    }

    wxGameConf &conf() { return config; }
    wxGameNews &getNews() { return news; }
  };
}

#endif
