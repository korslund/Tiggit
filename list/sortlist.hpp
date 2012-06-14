#ifndef __SORTLIST_HPP_
#define __SORTLIST_HPP_

#include "listbase.hpp"
#include <assert.h>
#include <algorithm>

namespace List
{
  struct Sorter
  {
    // Return true if a should come before b
    virtual bool isLess(const void* a, const void* b) = 0;
  };

  /* A sortlist sorts the parent list based on the given sorter.
   */
  class SortList : public ListBase
  {
  public:
    SortList(ListBase *par)
      : ListBase(par), srt(NULL), reverse(false)
    { assert(par); }

    // Set sorter. A NULL value will copy the parent list unsorted.
    void setSort(Sorter *p = NULL)
    {
      reverse = false;
      srt = p;
      updateChildren();
    }

    void setReverse(bool b)
    {
      reverse = b;
      updateChildren();
    }

    bool getReverse() { return reverse; }

  private:
    Sorter *srt;
    bool reverse;

    // Intermediary needed to work around std::sort() suckiness
    struct Tmp
    {
      Sorter *srt;

      bool operator()(const void *a, const void *b)
      { return srt->isLess(a,b); }
    };

    void updateList()
    {
      const PtrList &par = ((ListBase*)parent)->getList();

      // First, copy the list over
      list = par;

      // If there is no sorter, do simplified version
      if(!srt)
        {
          if(reverse)
            std::reverse(list.begin(), list.end());

          return;
        }

      Tmp tmp;
      tmp.srt = srt;

      if(reverse)
        std::sort(list.rbegin(), list.rend(), tmp);
      else
        std::sort(list.begin(), list.end(), tmp);
    }
  };
}
#endif
