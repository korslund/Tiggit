#include "gamedata.hpp"

#include "wx/boxes.hpp"
#include "notifier.hpp"

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

void wxTigApp::GameData::updateReady()
{
  if(!listener) return;

  // Load the updated news file and refresh the display
  listener->refreshNews();

  // Check if there was actually any new data in the repo
  if(!repo.hasNewData())
    {
      // Just update the stats
      repo.loadStats();
      updateDisplayStatus();
      return;
    }

  /* If we got here, a full data update is necessary.

     However, we might have gotten a program update as well, which
     trumps data updates, and instead issues a request to restart the
     entire program. (Restarting reloads the data too, of course.)
   */

  if(repo.newProgramPath() != "")
    /* If a new version is available, notify the user so they can
       restart. No further action is needed, any restart at this point
       (even a crash) will work.

       When the user presses the displayed button, notifyButton() is
       invoked, and the program is restarted.

       If a new version is available, we do NOT reload the data. This
       is because the new data may be packaged in a new format that
       the current version doesn't know how to handle.

       This is by design, so that we can update the client and data
       simultaneously, without worrying about cross-version
       compatibility.
     */
    listener->displayNotification("A new version of Tiggit has been installed", "Restart now", 2);

  else
    {
      /* Data update but no program update. Don't ask the user, just
         do it immediately.
      */
      loadData();
    }
}

void wxTigApp::GameData::notifyButton(int id)
{
  assert(id == 2);
  // TODO: launch the newly installed executable
}

void wxTigApp::GameData::killData()
{
  const InfoLookup &lst = repo.getList();
  InfoLookup::const_iterator it;
  for(it=lst.begin(); it!=lst.end(); it++)
    {
      LiveInfo *li = it->second;
      GameInf *gi = (GameInf*)(li->extra);
      if(gi) delete gi;
      li->extra = NULL;
    }
}

void wxTigApp::GameData::loadData()
{
  using namespace std;

  // First, kill any existing data structures
  killData();

  // Then, load the data
  try { repo.loadData(); }
  catch(std::exception &e)
    {
      Boxes::error(e.what());
      return;
    }
  catch(...)
    {
      Boxes::error("Unknown error while loading data");
      return;
    }

  // Create GameInf structs attached to all the LiveInfo structs
  const InfoLookup &lst = repo.getList();
  InfoLookup::const_iterator it;
  for(it=lst.begin(); it!=lst.end(); it++)
    {
      LiveInfo *li = it->second;
      assert(li->extra == NULL);
      li->extra = new GameInf(li, &config);
    }

  /* Transfer existing install jobs, if any, over to the new LiveInfo
     structs. Installs-in-progress will thus "survive" a data reload
     uninterrupted.

     The StatusNotifier (notify) does all the grunt work of looking up
     the structures based on idname.
  */
  notify.reassignJobs();

  /* Propagate update notifications down the list hierarchy. This has
     to be called AFTER the GameInf structures are set up, otherwise
     the wx display classes will get update notifications before there
     is any data to update from.
   */
  repo.doneLoading();

  // Notify every list that the data has been reloaded
  notifyReloaded();
}

wxTigApp::GameData::GameData(Repo &rep)
  : config(rep.getPath("wxtiggit.conf")), news(&rep),
    repo(rep)
{
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

  killData();
}
