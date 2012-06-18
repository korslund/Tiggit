#ifndef __TIGLIB_SORTERS_HPP_
#define __TIGLIB_SORTERS_HPP_

#include "gamelister.hpp"

namespace TigLib
{
  // Sort based on title (ascending, case insensitive)
  struct TitleSort : GameSorter
  {
    bool isLess(const LiveInfo *a, const LiveInfo *b);
  };

  /* Sort based on rating (decending). Sort missing ratings (0 votes)
     after any non-missing rating. Fall back to title sort if ratings
     are the same or both missing.
  */
  struct RateSort : TitleSort
  {
    bool isLess(const LiveInfo *a, const LiveInfo *b);
  };

  /* Sort by download count (descending). If counts are equal, fall
     back to rate sort (which again will fall back to title sort if
     necessary.)
   */
  struct DLSort : RateSort
  {
    bool isLess(const LiveInfo *a, const LiveInfo *b);
  };

  /* Sort by date (newest first)
   */
  struct DateSort : GameSorter
  {
    bool isLess(const LiveInfo *a, const LiveInfo *b);
  };
}

#endif
