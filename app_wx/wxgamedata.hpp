#ifndef __WXAPP_WXGAMEDATA_HPP_
#define __WXAPP_WXGAMEDATA_HPP_

#include "wx/wxgamedata.hpp"

/*
  This is our backend implementation of the wx/wxgamedata.hpp abstract
  interface. The implementation uses pretty much all the other modules
  in the tiggit source set to provide the necessary functionality.
 */

// This header is not included in a lot of places, so this is OK.
using namespace wxTiggit;

namespace wxTigApp
{
  struct GameInfo : wxGameInfo
  {
  private:
    wxString title, titleStatus, timeStr, rateStr, dlStr, statusStr, desc;

    bool isInstalled() const;
    bool isUninstalled() const;
    bool isWorking() const;
    bool isDemo() const;
    bool isNew() const;

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

  struct GameList : wxGameList
  {
    void addListener(wxGameListener*);
    void removeListener(wxGameListener*);

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
    const wxGameInfo& get(int);
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
    GameConf config;
    GameNews news;

    wxGameList &getAllList();
    wxGameConf &conf() { return config; }
    wxGameNews &getNews() { return news; }
  };
}

#endif
