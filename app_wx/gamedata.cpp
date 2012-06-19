#include "gamedata.hpp"

using namespace TigData;
using namespace wxTigApp;
using namespace TigLib;

wxGameNewsItem it;

const wxGameNewsItem &GameNews::get(int i) const
{ return it; }
int GameNews::size() const
{ return 0; }
void GameNews::reload()
{}
void GameNews::markAsRead(int)
{}
void GameNews::markAllAsRead()
{}

struct FreeDemoPick : GamePicker
{
  bool free;
  FreeDemoPick(bool _free) : free(_free) {}

  bool include(const LiveInfo *inf)
  { return inf->ent->tigInfo.isDemo != free; }
};

struct InstalledPick : GamePicker
{
  bool include(const LiveInfo *inf)
  { return !(((GameInf*)inf->extra)->isUninstalled()); }
};

static FreeDemoPick freePick(true), demoPick(false);
static InstalledPick instPick;

wxTigApp::GameData::GameData(Repo &rep)
  : config(rep.getPath("wxtiggit.conf")), repo(rep)
{
  // Create GameInf structs attached to all the LiveInfo structs
  const InfoLookup &lst = rep.getList();
  InfoLookup::const_iterator it;
  for(it=lst.begin(); it!=lst.end(); it++)
    {
      LiveInfo *li = it->second;
      assert(li->extra == NULL);
      li->extra = new GameInf(li);
    }

  // TODO: When updating the list, make sure to clean up and kill all
  // the GameInfs as well.
  latest = new GameList(rep.baseList(), NULL);
  freeware = new GameList(rep.baseList(), &freePick);
  demos = new GameList(rep.baseList(), &demoPick);
  installed = new GameList(rep.baseList(), &instPick);
}

wxTigApp::GameData::~GameData()
{
  delete latest;
  delete freeware;
  delete demos;
  delete installed;
}
