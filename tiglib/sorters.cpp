#include "sorters.hpp"
#include <boost/algorithm/string.hpp>

using namespace TigLib;

bool TitleSort::isLess(const LiveInfo *a, const LiveInfo *b)
{
  return boost::algorithm::ilexicographical_compare
    (a->ent->title, b->ent->title);
}

bool RateSort::isLess(const LiveInfo *a, const LiveInfo *b)
{
  float rateA = a->ent->rating;
  float rateB = b->ent->rating;

  if(a->ent->rateCount == 0) rateA = -1;
  if(b->ent->rateCount == 0) rateB = -1;

  // Does 'a' have a rating? If so, sort by rating. This also
  // covers the case where 'b' has no rating (rateB = -1).
  if(rateA >= 0)
    {
      // Sort equal ratings by name
      if(rateA == rateB)
        return TitleSort::isLess(a, b);

      // Greatest first
      return rateA > rateB;
    }

  // 'a' has no rating. What about b?
  if(rateB >= 0)
    // If yes, then always put 'b' first.
    return false;

  // Neither has a rating. Sort by title instead.
  return TitleSort::isLess(a, b);
}

bool DLSort::isLess(const LiveInfo *a, const LiveInfo *b)
{
  if(a->ent->dlCount == b->ent->dlCount)
    return RateSort::isLess(a,b);

  // Highest dl count first
  return a->ent->dlCount > b->ent->dlCount;
}

bool DateSort::isLess(const LiveInfo *a, const LiveInfo *b)
{
  return a->ent->addTime > b->ent->addTime;
}
