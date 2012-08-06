#include "gamedata.hpp"

using namespace TigData;
using namespace wxTigApp;
using namespace TigLib;

void GameNews::reload()
{
  news.reload();
  items.resize(news.size());

  for(int i=0; i<news.size(); i++)
    {
      wxGameNewsItem &out = items[i];
      const TigLib::NewsItem &in = news.get(i);

      out.read = in.isRead;
      out.dateNum = in.date;
      out.subject = strToWx(in.subject);
      out.body = strToWx(in.body);

      char buf[50];
      strftime(buf,50, "%Y-%m-%d", gmtime(&in.date));
      out.date = wxString(buf, wxConvUTF8);
    }
}

void GameNews::markAsRead(int i)
{
  news.markAsRead(i);
  items[i].read = true;
}

void GameNews::markAllAsRead()
{
  news.markAllAsRead();
  for(int i=0; i<items.size(); i++)
    items[i].read = true;
}

struct FreeDemoPick : GamePicker
{
  bool free;
  FreeDemoPick(bool _free) : free(_free) {}

  bool include(const LiveInfo *inf)
  { return inf->ent->isDemo() != free; }
};

struct InstalledPick : GamePicker
{
  bool include(const LiveInfo *inf) { return !inf->isUninstalled(); }
};

static FreeDemoPick freePick(true), demoPick(false);
static InstalledPick instPick;

wxTigApp::GameData::GameData(Repo &rep)
  : config(rep.getPath("wxtiggit.conf")), news(&rep),
    repo(rep)
{
  // Create GameInf structs attached to all the LiveInfo structs
  const InfoLookup &lst = rep.getList();
  InfoLookup::const_iterator it;
  for(it=lst.begin(); it!=lst.end(); it++)
    {
      LiveInfo *li = it->second;
      assert(li->extra == NULL);
      li->extra = new GameInf(li, &config);
    }

  // TODO: When updating the list, make sure to clean up and kill all
  // the GameInfs as well.
  latest = new GameList(rep.baseList(), NULL);
  freeware = new GameList(rep.baseList(), &freePick);
  demos = new GameList(rep.baseList(), &demoPick);
  installed = new GameList(rep.baseList(), &instPick);

  //freeware->lister.dumpTags();
}

wxTigApp::GameData::~GameData()
{
  delete latest;
  delete freeware;
  delete demos;
  delete installed;
}
