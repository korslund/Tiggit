#ifndef __WX_WXGAMEDATA_HPP_
#define __WX_WXGAMEDATA_HPP_

#include "wxcommon.hpp"

/*
  This abstract set of interfaces represents all our interaction with
  the outside world.

 Abstract interface for any class that presents game info in a
   wx-usable format.
 */

namespace wxTiggit
{
  struct wxScreenshotCallback
  {
    virtual void shotIsReady(const std::string &id, const wxImage &shot) = 0;
  };

  struct wxGameInfo
  {
    virtual bool isNew() const = 0;
    virtual bool isInstalled() const = 0;
    virtual bool isUninstalled() const = 0;
    virtual bool isWorking() const = 0;
    virtual bool isDemo() const = 0;

    virtual wxString getTitle(bool includeStatus=false) const = 0;
    virtual wxString timeString() const = 0;
    virtual wxString rateString() const = 0;
    virtual wxString dlString() const = 0;
    virtual wxString statusString() const = 0;

    virtual std::string getHomepage() const = 0;
    virtual std::string getTiggitPage() const = 0;
    virtual std::string getIdName() const = 0;
    virtual std::string getDesc() const = 0;
    virtual int myRating() const = 0;
    virtual std::string getDir() const = 0;

    virtual void rateGame(int i) = 0;
    virtual void requestShot(wxScreenshotCallback*) = 0;

    virtual void installGame() = 0;
    virtual void uninstallGame() = 0;
    virtual void launchGame() = 0;
    virtual void abortJob() = 0;
  };

  struct wxGameListener;
  struct wxGameList
  {
    virtual void addListener(wxGameListener*) = 0;
    virtual void removeListener(wxGameListener*) = 0;

    virtual void flipReverse() = 0;
    virtual void setReverse(bool) = 0;

    virtual void clearTags() = 0;
    virtual void setTags(const std::string &) = 0;
    virtual void setSearch(const std::string &) = 0;

    virtual bool sortTitle() = 0;
    virtual bool sortDate() = 0;
    virtual bool sortRating() = 0;
    virtual bool sortDownloads() = 0;

    virtual int size() const = 0;
    virtual const wxGameInfo& get(int) = 0;
    virtual wxGameInfo& edit(int) = 0;
  };

  struct wxGameConf
  {
    virtual bool getShowVotes() = 0;
    virtual void setShowVotes(bool) = 0;
  };

  struct wxGameNewsItem
  {
    int id;
    time_t dateNum;
    wxString date, subject, body;
    bool read;
  };

  struct wxGameNews
  {
    virtual const wxGameNewsItem &get(int i) const = 0;
    virtual int size() const = 0;
    virtual void reload() = 0;
    virtual void markAsRead(int) = 0;
    virtual void markAllAsRead() = 0;
  };

  struct wxGameData
  {
    virtual wxGameList &getAllList() = 0;

    void addListener(wxGameListener *p) { getAllList().addListener(p); }
    void removeListener(wxGameListener *p) { getAllList().removeListener(p); }

    virtual wxGameConf &conf() = 0;
    virtual wxGameNews &getNews() = 0;
  };

  // Callback
  struct wxGameListener
  {
    wxGameListener() : list(NULL) {}
    virtual ~wxGameListener()
    {
      if(list)
        list->removeListener(this);
    }
    wxGameList *list;

    virtual void gameStatusChanged() = 0;
    virtual void gameInfoChanged() = 0;
    virtual void gameListChanged() = 0;
    virtual void gameListReloaded() = 0;
  };
}

#endif
