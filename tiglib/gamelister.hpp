#ifndef __TIGLIB_GAMELISTER_HPP_
#define __TIGLIB_GAMELISTER_HPP_

#include "list/picklist.hpp"
#include "list/sortlist.hpp"
#include "gameinfo/tigentry.hpp"
#include "liveinfo.hpp"

namespace TigLib
{
  /* These interfaces provide you with the base classes you need to
     design your own custom picker and sorter classes. They can be
     used with GameLister below.
   */
  struct GamePicker : List::Picker
  {
    virtual bool include(const LiveInfo*) = 0;

  private:
    bool include(const void *p)
    { return include((const LiveInfo*)p); }
  };

  struct GameSorter : List::Sorter
  {
    virtual bool isLess(const LiveInfo*, const LiveInfo*) = 0;

  private:
    bool isLess(const void* a, const void* b)
    { return isLess((const LiveInfo*)a,(const LiveInfo*)b); }
  };

  /* Create a sub-list from a given base list, and apply sorting,
     searching and other filters to that.
   */
  class GameLister
  {
    List::PickList base, tags, search;
    List::SortList sort;
    GamePicker *searchPick, *tagPick;

  public:
    GameLister(List::ListBase &all, GamePicker *mainPick = NULL)
      : base(&all), tags(&base), search(&tags), sort(&search),
        searchPick(NULL), tagPick(NULL)
    { base.setPick(mainPick); }
    ~GameLister()
    {
      if(searchPick) delete searchPick;
      if(tagPick) delete tagPick;
    }

    LiveInfo &get(int i) { return *((LiveInfo*)sort.getList()[i]); }
    int size() const { return sort.getList().size(); }
    int totalSize() const { return base.getList().size(); }

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

    /* Set space and/or comma-delimited tag list. List will only pick
       out games that match ALL the given tags, or aliases of them.
     */
    void setTags(const std::string &str);

    /* Count the number of games matching a given tag string.
     */
    int countTags(const std::string &str);

    //void dumpTags();
  };
}

#endif
