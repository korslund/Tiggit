#include "gamedata.hpp"

using namespace TigLib;

void GameData::copyList()
{
  // Setup output list
  List::PtrList &out = allList.fillList();
  out.resize(0);

  // Setup source list
  using namespace GameInfo;
  const TigLoader::Lookup &list = data.getList();
  TigLoader::Lookup::const_iterator it;

  // Iterate and copy all the pointers
  out.reserve(list.size());
  for(it = list.begin(); it != list.end(); it++)
    out.push_back(it->second);

  // Signal child lists that data has changed
  allList.done();
}
