#ifndef __WX_WXGAMELIST_HPP_
#define __WX_WXGAMELIST_HPP_

#include "wxcommon.hpp"

/* Abstract interface for any class that presents game info in a
   wx-usable format.
 */

namespace wxTiggit
{
  struct wxGameInfo
  {
    virtual bool isNew() const = 0;
    virtual bool isInstalled() const = 0;
    virtual bool isUninstalled() const = 0;
  };

  struct wxGameListener;
  struct wxGameList
  {
    virtual void addListener(wxGameListener*) = 0;
    virtual void removeListener(wxGameListener*) = 0;

    virtual void flipReverse() = 0;
    virtual void setReverse(bool) = 0;

    virtual int size() const = 0;
    virtual const wxGameInfo& get(int) = 0;
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
