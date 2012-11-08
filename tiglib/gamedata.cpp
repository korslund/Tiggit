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
  std::vector<std::string> games = repo->getGameList();
  for(int i=0; i<games.size(); i++)
    {
      const std::string &id = games[i];
      if(repo->getGameDir(id) != "")
        {
          LiveInfo *l = get(id);
          if(l) l->markAsInstalled();
        }
    }

  // Signal child lists that data has changed
  //allList.done();

  /* Update: this was a bad idea. Because systems further down stream
     may rely on the LiveInfo::extra member to be set, which of course
     isn't the case at this point.

     So do NOT signal child lists that we have changed. Leave that to
     the caller.
   */
}
