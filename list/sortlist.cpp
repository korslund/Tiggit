#include <assert.h>
#include <algorithm>
#include "sortlist.hpp"

using namespace List;

SortList::SortList(ListBase *par)
  : ListBase(par), srt(NULL), reverse(false)
{ assert(par); }

void SortList::setSort(Sorter *p)
{
  srt = p;
  updateChildren();
}

void SortList::setReverse(bool b)
{
  reverse = b;
  updateChildren();
}

// Intermediary needed to work around std::sort() suckiness
struct Tmp
{
  Sorter *srt;

  bool operator()(const void *a, const void *b)
  { return srt->isLess(a,b); }
};

void SortList::updateList()
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
