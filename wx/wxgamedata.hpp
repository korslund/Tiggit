#ifndef __WX_WXGAMEDATA_HPP_
#define __WX_WXGAMEDATA_HPP_

#include "wxcommon.hpp"
#include <stdint.h>
#include <vector>

/*
  This abstract set of interfaces represents all our interaction with
  the outside world.
 */
namespace wxTiggit
{
  struct wxGameInfo
  {
    // Get screenshot for this game. May return an empty image, but it
    // should still be usable.
    virtual const wxImage &getShot() = 0;

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
    virtual std::string getIdName() const = 0;
    virtual wxString getVersion() const = 0;
    virtual wxString getSize() const = 0;
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

    // Count the number of elements that would result in the list if
    // you had called setTags() with this string. The result is not
    // cached, so use sparingly.
    virtual int countTags(const std::string &) = 0;

    virtual bool sortTitle() = 0;
    virtual bool sortDate() = 0;
    virtual bool sortRating() = 0;
    virtual bool sortDownloads() = 0;

    virtual int size() const = 0;
    virtual int totalSize() const = 0;
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

  struct wxAppListener;
  struct wxGameData
  {
    virtual wxGameList &getLatest() = 0;
    virtual wxGameList &getFreeware() = 0;
    virtual wxGameList &getDemos() = 0;
    virtual wxGameList &getInstalled() = 0;

    virtual wxGameConf &conf() = 0;
    virtual wxGameNews &getNews() = 0;

    /* Move repository directory. Returns true on success, false if
       the directory was not writable.
     */
    virtual bool moveRepo(const std::string &newDir) = 0;
    virtual std::string getRepoDir() = 0;

    virtual const std::vector<std::string> &getLibraryMenu() = 0;
    virtual void installLibrary(int num) = 0;
    virtual bool getLeftImage(std::string &file, std::string &url) = 0;

    // Game bug reports
    virtual void submitBroken(const std::string &idname, const std::string &comment) = 0;

    // Game suggestions
    virtual void submitGame(const std::string &title, const std::string &homepage,
                            const std::string &shot, const std::string &download,
                            const std::string &version, const std::string &devname,
                            const std::string &tags, const std::string &type,
                            const std::string &desc) = 0;

    // True if there are any active jobs in progress
    virtual bool isActive() = 0;

    virtual void notifyButton(int id) = 0;

    wxAppListener *listener;
    wxGameData() : listener(NULL) {}
  };

  // Callbacks
  struct wxAppListener
  {
    wxAppListener() : _data(NULL) {}
    virtual ~wxAppListener()
    {
      if(_data) _data->listener = NULL;
    }
    wxGameData *_data;

    // Called when the app needs to reload and redisplay the news
    virtual void refreshNews() = 0;

    /* Called when the app should present an update message and
       progress.
     */
    virtual void displayProgress(const std::string &message, uint64_t cur,
                                 uint64_t total) = 0;

    /* Called when the app should present the user with a
       notification, and an associated button (if set). When the
       button is clicked, wxGameData::notifyButton(id) should be
       called.
    */
    virtual void displayNotification(const std::string &message, const std::string &button,
                                     int id) = 0;
  };

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
    // itself has not. Often called on the non-install lists when a
    // game has finished installing.
    virtual void gameInfoChanged() = 0;

    // Called when the current list has changed selection in response
    // to internal functions (such as sorting or tag selection), but
    // the base list has not changed.
    virtual void gameSelectionChanged() = 0;

    // Called when the list itself has changed in size, content or
    // ordering.
    virtual void gameListChanged() = 0;

    // Called on 'soft' status changes, for example on the 'ticks'
    // that happen while something is installing. Normally only the
    // list view needs to be refreshed.
    virtual void gameStatusChanged() = 0;
  };
}

#endif
