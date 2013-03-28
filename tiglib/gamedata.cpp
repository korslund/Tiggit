#include "gamedata.hpp"
#include "liveinfo.hpp"
#include "repo.hpp"

using namespace TigLib;

void GameData::clear()
{
  List::PtrList &out = allList.fillList();

  // Delete old objects
  for(int i=0; i<out.size(); i++)
    delete (LiveInfo*)out[i];
  lookup.clear();

  out.resize(0);
}

void GameData::createLiveData(Repo *repo)
{
  clear();

  // Output list
  List::PtrList &out = allList.fillList();

  // Source list
  using namespace GameInfo;
  const TigLoader::Lookup &list = data.getList();
  TigLoader::Lookup::const_iterator it;

  // Set up new objects
  out.resize(list.size());
  int index = 0;
  int64_t maxTime = 0;
  for(it = list.begin(); it != list.end(); it++)
    {
      const std::string &idname = it->first;
      const TigData::TigEntry *ent = it->second;
      LiveInfo *inf = new LiveInfo(ent, repo);
      out[index++] = inf;
      lookup[idname] = inf;

      if(ent->addTime > maxTime)
        maxTime = ent->addTime;

      inf->instSize = repo->getGameSize(ent->urlname);
    }

  // Store new maxtime
  repo->setLastTime(maxTime);

  // Apply current status information
  Repo::StatusList games;
  repo->getStatusList(games);
  for(int i=0; i<games.size(); i++)
    {
      const Repo::GameStatus &e = games[i];
      LiveInfo *l = get(e.id);
      if(!l) continue;
      l->markAsInstalled(e.curVer, e.newVer, e.isUpdated);
    }
}
