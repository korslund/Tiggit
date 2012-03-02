#ifndef _LISTKEEPER_HPP_
#define _LISTKEEPER_HPP_

#include <vector>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include "datalist.hpp"
#include "gameinfo.hpp"

class ListKeeper
{
  // The base selection for this list. Doesn't change after it has
  // been set up.
  std::vector<int> base;

  // Selection criterium used to set up the base. Set through the
  // constructor, and cannot be changed later.
  int selection;

  // Items from the base selected after searching.
  std::vector<int> searched;

  bool isSearched;
  bool isSorted;

  /* 0 - title
     1 - date
     2 - rating, then title
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

  // Not in use yet!
  struct DownloadSort : SortBase
  {
    DownloadSort(DataList &d) : SortBase(d) {}
    bool isLess(DataList::Entry &a, DataList::Entry &b)
    {
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
            searched.resize(base.size());
            for(int i=0; i<searched.size(); i++)
              searched[i] = base[i];
          }
        else
          {
            // Yes. Start from scratch and add elements.
            searched.resize(0);
            searched.reserve(base.size());

            // Add all games that match the search. This is a dumb,
            // slow and useless algorithm, but at current list sizes
            // it's ok. Never optimize until you get hate mail.
            for(int i=0; i<base.size(); i++)
              if(boost::algorithm::icontains(data.arr[base[i]].tigInfo.title, search))
                searched.push_back(base[i]);
          }

        isSearched = true;
        isSorted = false;
      }

    assert(isSearched);

    if(!isSorted)
      {
        // Apply current sorting
        if(sortBy == 0)
          {
            if(reverse)
              sort(searched.rbegin(), searched.rend(), TitleSort(data));
            else
              sort(searched.begin(), searched.end(), TitleSort(data));
          }
        else if(sortBy == 1)
          {
            // Some repeated code, thanks to C++ actually being quite a
            // sucky language.
            if(reverse)
              sort(searched.rbegin(), searched.rend(), DateSort(data));
            else
              sort(searched.begin(), searched.end(), DateSort(data));
          }
        else if(sortBy == 2)
          {
            if(reverse)
              sort(searched.rbegin(), searched.rend(), RateSort(data));
            else
              sort(searched.begin(), searched.end(), RateSort(data));
          }

        isSorted = true;
      }

    assert(isSorted && isSearched);
  }

  void setSort(int type)
  {
    sortBy = type;
    isSorted = false;
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

  void sortTitle() { setSort(0); }
  void sortDate() { setSort(1); }
  void sortRating() { setSort(2); }

  void setReverse(bool rev)
  {
    reverse = rev;
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
    isSearched = false;
    isSorted = false;

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

            // Installed games may not be listed anywhere else, so
            // skip this game.
            else if(g.isInstalled() || g.isWorking()) continue;

            // Insert into each tab according to their own criteria
            if(selection == SL_NEW && e.isNew ||
               selection == SL_FREEWARE && !e.tigInfo.isDemo ||
               selection == SL_DEMOS && e.tigInfo.isDemo)
              keep = true;

            if(keep)
              base.push_back(i);
          }
      }
  }

  int baseSize() { return base.size(); }
  int size()
  {
    makeReady();
    return searched.size();
  }
};

#endif
