#ifndef __SORTLIST_HPP_
#define __SORTLIST_HPP_

#include "listbase.hpp"

namespace List
{
  struct Sorter
  {
    // Return true if a should come before b
    virtual bool isLess(const void* a, const void* b) = 0;
    virtual ~Sorter() {}
  };

  /* A sortlist sorts the parent list based on the given sorter.
   */
  class SortList : public ListBase
  {
  public:
    SortList(ListBase *par);

    // Set sorter. A NULL value will copy the parent list unsorted
    // (but reverse status still applies.)
    void setSort(Sorter *p = NULL);
    void setReverse(bool b);
    bool getReverse() { return reverse; }

  private:
    Sorter *srt;
    bool reverse;

    void updateList();
  };
}
#endif
