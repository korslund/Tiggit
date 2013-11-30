#ifndef __WXAPP_GAMELIST_HPP_
#define __WXAPP_GAMELIST_HPP_

#include "gameinf.hpp"
#include "tiglib/gamelister.hpp"
#include "list/haschanged.hpp"
#include <set>

namespace wxTigApp
{
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
    GameList(List::ListBase &list, TigLib::GamePicker *pick)
      : lister(list, pick), notif(lister.topList(), this),
        sortStatus(SS_NONE) {}

    // Invoke gameListChanged() on our listeners
    void notifyListChange();

    // Invoke gameSelectionChanged() on listeners
    void notifySelectChange();

    // Invoke gameInfoChanged() on our listeners
    void notifyInfoChange();

    // Invoke gameStatusChanged() on our listeners
    void notifyStatusChange();

    TigLib::GameLister lister;

  private:
    Notifier notif;
    std::set<wxGameListener*> listeners;
    int sortStatus;

    enum SortStatus
      {
        SS_NONE, SS_TITLE, SS_DATE, SS_RATING, SS_DOWNLOADS
      };

    bool setStat(int i)
    {
      int old = sortStatus;
      sortStatus = i;
      return i == old;
    }

    void addListener(wxGameListener *p);
    void removeListener(wxGameListener *p);

    void flipReverse();
    void setReverse(bool);

    void clearTags();
    void setTags(const std::string &);
    void setSearch(const std::string &);
    int countTags(const std::string &);

    bool sortTitle();
    bool sortDate();
    bool sortRating();
    bool sortDownloads();

    int size() const;
    int totalSize() const;
    const wxGameInfo& get(int i) { return edit(i); }
    wxGameInfo& edit(int);
  };
}
#endif
