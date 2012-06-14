#include "sorters.hpp"
#include <boost/algorithm/string.hpp>

using namespace TigLib;

bool TitleSort::isLess(const TigData::TigEntry *a, const TigData::TigEntry *b)
{
  return boost::algorithm::ilexicographical_compare
    (a->tigInfo.title, b->tigInfo.title);
}

bool RateSort::isLess(const TigData::TigEntry *a, const TigData::TigEntry *b)
{
  float rateA = a->rating;
  float rateB = b->rating;

  if(a->rateCount == 0) rateA = -1;
  if(b->rateCount == 0) rateB = -1;

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

bool DLSort::isLess(const TigData::TigEntry *a, const TigData::TigEntry *b)
{
  if(a->dlCount == b->dlCount)
    return RateSort::isLess(a,b);

  // Highest dl count first
  return a->dlCount > b->dlCount;
}

bool DateSort::isLess(const TigData::TigEntry *a, const TigData::TigEntry *b)
{
  return a->addTime > b->addTime;
}
