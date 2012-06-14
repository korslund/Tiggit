#ifndef __LISTKEEPER_HPP_
#define __LISTKEEPER_HPP_

#include <vector>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <string>

class ListKeeper
{
  // The base selection for this list. Doesn't change after it has
  // been set up.
  std::vector<int> base;

  // Sub-selection, used for tag searches. Is applied before
  // searching.
  std::vector<int> subSelect;

  // Selection criterium used to set up the base. Set through the
  // constructor, and cannot be changed later.
  int selection;

  // Items from the base selected after searching. After search
  // selection, this list is also sorted in-place according to our
  // current sort configuration.
  std::vector<int> searched;

  bool isSearched;
  bool isSorted;

  /* 0 - title
     1 - date (newst to oldest)
     2 - rating, then title
     3 - downloads
   */
  int sortBy;
  bool reverse;

  // Current search string
  std::string search;

  DataList &data;

  struct SortBase
  {
    DataList &data;
    SortBase(DataList &d) : data(d) {}

    // Return true if 'a' should come before 'b' on the list
    virtual bool isLess(DataList::Entry &a, DataList::Entry &b) = 0;

    bool operator()(int a, int b)
    {
      return isLess(data.arr[a], data.arr[b]);
    }
  };

  struct TitleSort : SortBase
  {
    TitleSort(DataList &d) : SortBase(d) {}
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    { return isLessStatic(a, b); }

    // Kludge city, but it works for now
    static bool isLessStatic(DataList::Entry &a, DataList::Entry &b)
    {
      return boost::algorithm::ilexicographical_compare(a.tigInfo.title,
                                                        b.tigInfo.title);
    }
  };

  // Sort based on rating, fall back on title sort if rating doesn't
  // provide enough info to sort.
  struct RateSort : SortBase
  {
    RateSort(DataList &d) : SortBase(d) {}
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
      if(a.rateCount == 0) a.rating = -1;
      if(b.rateCount == 0) b.rating = -1;

      // Does 'a' have a rating? If so, sort by rating. This also
      // covers the case where 'b' has no rating (b.rating = -1).
      if(a.rating >= 0)
        {
          // Sort equal ratings by name
          if(a.rating == b.rating)
            return TitleSort::isLessStatic(a, b);
          else
            return a.rating > b.rating;
        }

      // 'a' has no rating. What about b?
      else if(b.rating >= 0)
        // If yes, then always put 'b' first.
        return false;

      // Neither has a rating. Sort by title instead.
      else return TitleSort::isLessStatic(a, b);
    }
  };

  struct DownloadSort : SortBase
  {
    DownloadSort(DataList &d) : SortBase(d) {}
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
      // If counts equal, sort by rating (which also automatically
      // sorts by name if neither are rated)
      if(a.dlCount == b.dlCount)
        {
          RateSort rs(data);
          return rs.isLess(a,b);
        }

      // Highest dl count first
      return a.dlCount > b.dlCount;
    }
  };

  // Not in use yet!
  struct PriceSort : SortBase
  {
    PriceSort(DataList &d) : SortBase(d) {}
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
      return a.tigInfo.price > b.tigInfo.price;
    }
  };

  struct DateSort : SortBase
  {
    DateSort(DataList &d) : SortBase(d) {}
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
      // Newest first
      return a.add_time > b.add_time;
    }
  };

  void makeReady()
  {
    if(!isSearched)
      {
        // Are we searching?
        if(search == "")
          {
            // Nope. Select all elements.
            searched.resize(subSelect.size());
            for(int i=0; i<searched.size(); i++)
              searched[i] = subSelect[i];
          }
        else
          {
            // Yes, we are searching. Start from scratch and add
            // elements.
            searched.resize(0);
            searched.reserve(subSelect.size());

            // Add all games that match the search. This is a dumb,
            // slow and useless algorithm, but at current list sizes
            // it's ok. Never optimize until you get hate mail.
            for(int i=0; i<subSelect.size(); i++)
              {
                int ind = subSelect[i];
                if(boost::algorithm::icontains(data.arr[ind].tigInfo.title, search))
                  searched.push_back(ind);
              }
          }

        isSearched = true;
        isSorted = false;
      }

    assert(isSearched);

    if(!isSorted)
      {
        // Apply current sorting
        if(sortBy == 0) // TITLE
          {
            if(reverse)
              sort(searched.rbegin(), searched.rend(), TitleSort(data));
            else
              sort(searched.begin(), searched.end(), TitleSort(data));
          }
        else if(sortBy == 1) // DATE
          {
            // Some repeated code, thanks to C++ actually being quite a
            // sucky language.
            if(reverse)
              sort(searched.rbegin(), searched.rend(), DateSort(data));
            else
              sort(searched.begin(), searched.end(), DateSort(data));
          }
        else if(sortBy == 2) // RATING
          {
            if(reverse)
              sort(searched.rbegin(), searched.rend(), RateSort(data));
            else
              sort(searched.begin(), searched.end(), RateSort(data));
          }
        else if(sortBy == 3) // DOWNLOADS
          {
            if(reverse)
              sort(searched.rbegin(), searched.rend(), DownloadSort(data));
            else
              sort(searched.begin(), searched.end(), DownloadSort(data));
          }

        isSorted = true;
      }

    assert(isSorted && isSearched);
  }

  bool setSort(int type)
  {
    if(type == sortBy)
      return true;

    sortBy = type;
    isSorted = false;

    return false;
  }

public:
  /*
    Second parameter to the constructor is the selection. Choose from
    the following enum values.

    TODO: We could make a flag system out of this or something, but
    it's not that important. We will probably script the entire thing
    later at some point anyway.
   */
  enum
    {
      SL_ALL      = 1,    // All games
      SL_FREEWARE = 2,    // Non-installed, non-new freeware games
      SL_DEMOS    = 3,    // Non-installed, non-new demos
      SL_NEW      = 4,    // New, non-installed games (of all types)
      SL_INSTALL  = 8     // Games installed or being installed (of all types)
    };

  ListKeeper(DataList &dt, int select = SL_ALL)
    : selection(select), isSearched(false), isSorted(false), sortBy(0),
      reverse(false), data(dt)
  { reset(); }

  // Set sort mode. All return true if this was already the selected
  // sort mode.
  bool sortTitle() { return setSort(0); }
  bool sortDate() { return setSort(1); }
  bool sortRating() { return setSort(2); }
  bool sortDownloads() { return setSort(3); }

  void setSubSelection(const std::vector<int> &sel)
  {
    isSorted = false;
    isSearched = false;

    subSelect.resize(sel.size());
    for(int i=0; i<sel.size(); i++)
      {
        int ind = sel[i];
        assert(ind >= 0 && ind < base.size());
        subSelect[i] = base[ind];
      }
  }

  void clearSubSelection()
  {
    isSorted = false;
    isSearched = false;

    subSelect.resize(base.size());
    for(int i=0; i<subSelect.size(); i++)
      subSelect[i] = base[i];
  }

  void setReverse(bool rev)
  {
    reverse = rev;
    isSorted = false;
  }

  void flipReverse()
  {
    reverse = !reverse;
    isSorted = false;
  }

  void setSearch(const std::string &str)
  {
    search = str;
    isSearched = false;
  }

  DataList::Entry &get(int index)
  {
    assert(index >= 0 && index < size());
    makeReady();

    return data.arr[searched[index]];
  }

  /* Called whenever the source data list changes. It basically means
     we have to throw everything out. It is also called from the
     constructor.

     Calling reset() will NOT clear/reset the current search string or
     sorting selection. It will only throw out our cached lists,
     forcing the search and sort to be repeated on the new dataset
     upon the next call to get().
  */
  void reset()
  {
    base.resize(0);
    base.reserve(data.arr.size());

    if(selection == SL_ALL)
      {
        // Insert the entire thing
        base.resize(data.arr.size());

        for(int i=0; i<base.size(); i++)
          base[i] = i;
      }
    else
      {
        // Loop through the database and insert matching items
        for(int i=0; i<data.arr.size(); i++)
          {
            DataList::Entry &e = data.arr[i];
            GameInfo &g = GameInfo::conv(e);

            bool keep = false;

            // Installed tab
            if(selection == SL_INSTALL)
              {
                // We list everything that's got a non-zero install
                // status
                if(g.isInstalled() || g.isWorking())
                  keep = true;
              }

            /* Installed games may not be listed anywhere else, so
               skip this game.

               UPDATE: Disabled, we now gray these out instead of
               removing them. We could make this configurable.
            */
            //else if(g.isInstalled() || g.isWorking()) continue;

            // Insert into each tab according to their own criteria
            if(selection == SL_NEW && e.isNew ||
               selection == SL_FREEWARE && !e.tigInfo.isDemo ||
               selection == SL_DEMOS && e.tigInfo.isDemo)
              keep = true;

            if(keep)
              base.push_back(i);
          }
      }

    // Set up the subselection to be the entire base list
    clearSubSelection();
  }

  int baseSize() { return base.size(); }
  int size()
  {
    makeReady();
    return searched.size();
  }

  const std::vector<int> &getBaseList() { return base; }
};

#endif
