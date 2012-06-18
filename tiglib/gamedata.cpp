#include "gamedata.hpp"
#include "liveinfo.hpp"
#include "repo.hpp"

using namespace TigLib;

void GameData::createLiveData(Repo *repo)
{
  // Output list
  List::PtrList &out = allList.fillList();

  // Delete old objects
  for(int i=0; i<out.size(); i++)
    delete (LiveInfo*)out[i];

  // Source list
  using namespace GameInfo;
  const TigLoader::Lookup &list = data.getList();
  TigLoader::Lookup::const_iterator it;

  // Set up new objects
  out.resize(list.size());
  int index = 0;
  for(it = list.begin(); it != list.end(); it++)
    out[index++] = new LiveInfo(it->second, repo);

  // Signal child lists that data has changed
  allList.done();
}
