#ifndef _LISTKEEPER_HPP_
#define _LISTKEEPER_HPP_

#include <vector>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include "datalist.hpp"

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
    {
      return boost::algorithm::ilexicographical_compare(a.tigInfo.title,
                                                        b.tigInfo.title);
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
        else
          {
            // Some repeated code, thanks to C++ actually being quite a
            // sucky language.
            if(reverse)
              sort(searched.rbegin(), searched.rend(), DateSort(data));
            else
              sort(searched.begin(), searched.end(), DateSort(data));
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
      SL_ALL    = 1,    // All games
      SL_BROWSE = 2,    // Non-installed games
      SL_INSTALL= 3     // Games installed or being installed
    };

  ListKeeper(DataList &dt, int select = SL_ALL)
    : selection(select), isSearched(false), isSorted(false), sortBy(0),
      reverse(false), data(dt)
  { reset(); }

  void sortTitle() { setSort(0); }
  void sortDate() { setSort(1); }

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
            const DataList::Entry &e = data.arr[i];
            if((e.status == 0 && selection == SL_BROWSE) ||
               (e.status > 0 && selection == SL_INSTALL))
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
