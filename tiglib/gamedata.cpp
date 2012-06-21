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
  lookup.clear();

  // Source list
  using namespace GameInfo;
  const TigLoader::Lookup &list = data.getList();
  TigLoader::Lookup::const_iterator it;

  // Set up new objects
  out.resize(list.size());
  int index = 0;
  int64_t maxTime = -1;
  for(it = list.begin(); it != list.end(); it++)
    {
      const TigData::TigEntry *ent = it->second;
      LiveInfo *inf = new LiveInfo(ent, repo);
      out[index++] = inf;
      lookup[it->first] = inf;

      if(ent->addTime > maxTime)
        maxTime = ent->addTime;
    }

  // Store new maxtime
  repo->setLastTime(maxTime);

  // Apply install status
  std::vector<std::string> games = repo->inst.getNames();
  for(int i=0; i<games.size(); i++)
    {
      const std::string &id = games[i];
      if(repo->inst.getInt(id) == 2)
        {
          LiveInfo *l = get(id);
          if(l) l->markAsInstalled();
        }
    }

  // TODO: Apply rating info

  // Signal child lists that data has changed
  allList.done();
}
