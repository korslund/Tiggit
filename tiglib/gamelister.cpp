#include "gamelister.hpp"
#include "sorters.hpp"
#include <boost/algorithm/string.hpp>

using namespace TigLib;

static DLSort dlSort;
static TitleSort titleSort;
static RateSort rateSort;
static DateSort dateSort;

void GameLister::sortTitle() { setSort(&titleSort); }
void GameLister::sortDate() { setSort(&dateSort); }
void GameLister::sortRating() { setSort(&rateSort); }
void GameLister::sortDownloads() { setSort(&dlSort); }

struct SearchPicker : GamePicker
{
  std::string str;

  bool include(const LiveInfo *info)
  {
    return boost::algorithm::icontains(info->ent->tigInfo.title, str);
  }
};

void GameLister::setSearch(const std::string &str)
{
  if(!searchPick)
    searchPick = new SearchPicker;

  ((SearchPicker*)searchPick)->str = str;
  search.setPick(searchPick);
}
