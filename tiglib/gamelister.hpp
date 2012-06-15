#ifndef __TIGLIB_GAMELISTER_HPP_
#define __TIGLIB_GAMELISTER_HPP_

#include "list/picklist.hpp"
#include "list/sortlist.hpp"
#include "gameinfo/tigentry.hpp"

namespace TigLib
{
  /* These interfaces provide you with the base classes you need to
     design your own custom picker and sorter classes. They can be
     used with GameLister below.
   */
  struct GamePicker : List::Picker
  {
    virtual bool include(const TigData::TigEntry*) = 0;

  private:
    bool include(const void *p)
    { return include((const TigData::TigEntry*)p); }
  };

  struct GameSorter : List::Sorter
  {
    virtual bool isLess(const TigData::TigEntry*, const TigData::TigEntry*) = 0;

  private:
    bool isLess(const void* a, const void* b)
    { return isLess((const TigData::TigEntry*)a,
                    (const TigData::TigEntry*)b);
    }
  };

  /* Create a sub-list from a given base list, and apply sorting,
     searching and other filters to that.
   */
  struct GameLister
  {
    List::PickList base, tags, search;
    List::SortList sort;
    GamePicker *searchPick;

    GameLister(List::ListBase &all, GamePicker *mainPick = NULL)
      : base(&all), tags(&base), search(&tags), sort(&search),
        searchPick(NULL)
    { base.setPick(mainPick); }
    ~GameLister() { if(searchPick) delete searchPick; }

    const List::PtrList &getList() const { return sort.getList(); }
    int size() const { return getList().size(); }

    // Use this as the parent if you need to build more lists on top
    // of this list.
    List::ListBase &topList() { return sort; }

    void setPick(GamePicker *pck) { base.setPick(pck); }
    void setSort(GameSorter *srt) { sort.setSort(srt); }
    void setReverse(bool b) { sort.setReverse(b); }
    bool getReverse() { return sort.getReverse(); }
    void flipReverse() { setReverse(!getReverse()); }

    // Refresh lists
    void refresh() { base.refresh(); }

    // Use pre-defined sort classes
    void sortTitle();
    void sortDate();
    void sortRating();
    void sortDownloads();

    // Specify search string
    void setSearch(const std::string &str);
  };
}

#endif
