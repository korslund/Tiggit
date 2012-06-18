#ifndef __WX_WXGAMEDATA_HPP_
#define __WX_WXGAMEDATA_HPP_

#include "wxcommon.hpp"

/*
  This abstract set of interfaces represents all our interaction with
  the outside world.
 */

namespace wxTiggit
{
#define myEVT_SCREENSHOT_READY 20900

  struct ScreenshotEvent : wxEvent
  {
    std::string id;
    const wxImage *shot;

    ScreenshotEvent() : wxEvent(0, myEVT_SCREENSHOT_READY) {}
    wxEvent *Clone(void) const { return new ScreenshotEvent(*this); }
  };

  struct wxGameInfo
  {
    /* Request a screenshot for this game. The given event handler is
       sent a ScreenshotEvent when the screenshot is ready. This may
       happen synchronously (before requestShot() returns) or
       asynchronously (throgh AddPendingEvent from a worker thread.)
     */
    virtual void requestShot(wxEvtHandler*) = 0;

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
    virtual wxString getDesc() const = 0;
    virtual int myRating() const = 0;
    virtual std::string getDir() const = 0;

    virtual void rateGame(int i) = 0;

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
    virtual wxGameList &getLatest() = 0;
    virtual wxGameList &getFreeware() = 0;
    virtual wxGameList &getDemos() = 0;
    virtual wxGameList &getInstalled() = 0;

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

    // Called when game info or status has changed, but the list
    // itself has not.
    virtual void gameInfoChanged() = 0;

    // Called when the list itself has changed in size, content or
    // ordering.
    virtual void gameListChanged() = 0;
  };
}

#endif
