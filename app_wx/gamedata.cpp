#include "gamedata.hpp"

#include <time.h>

using namespace TigData;
using namespace wxTigApp;
using namespace TigLib;

bool GameInf::isInstalled() const { return false; }
bool GameInf::isUninstalled() const { return !info.ent->tigInfo.isDemo; }
bool GameInf::isWorking() const { return info.ent->tigInfo.isDemo; }
bool GameInf::isDemo() const { return false; }
bool GameInf::isNew() const { return false; }

// Create a nice size string
static wxString sizify(uint64_t size)
{
  wxString tmp;
  if(size > 1024*1024*1024)
    tmp << wxString::Format(wxT("%.1f"), size/(1024.0*1024*1024)) << wxT("Gb");
  else if(size > 1024*1024)
    tmp << wxString::Format(wxT("%.1f"), size/(1024.0*1024)) << wxT("Mb");
  else if(size > 1024)
    tmp << wxString::Format(wxT("%.1f"), size/1024.0) << wxT("Kb");
  else
    tmp << size;
  return tmp;
}

// Friendly time formatting, spits out strings like "3 days ago"
static wxString ago(time_t when, time_t now=0)
{
  if(now == 0)
    now = time(NULL);

  time_t diff = now-when;

  wxString res;

  int times[6] = {60,60,24,7,4,12};
  wxString names[6] = {wxT("second"),wxT("minute"),wxT("hour"),wxT("day"),
                       wxT("week"),wxT("month")};
  for(int i=0;i<6;i++)
    {
      if(diff < times[i])
        {
          res = wxString::Format(wxT("%d ") + names[i], diff);
          if(diff > 1) res += wxT("s");
          res += wxT(" ago");
          break;
        }
      diff /= times[i];
    }

  if(!res.IsEmpty())
    return res;

  // If it's more than a year ago, use dates instead
  char buf[50];
  strftime(buf,50, "%Y-%m-%d", gmtime(&when));
  return wxString(buf, wxConvUTF8);
}

void GameInf::updateStatus()
{
  titleStatus = title;

  if(isWorking())
    {
      int current = 3343684;
      int total  = 19343865;

      int percent = 0;
      if(total > 0)
        percent = (int)((100.0*current)/total);

      // Update visible progress strings
      wxString status;
      status << percent << wxT("%");
      titleStatus += wxT(" [") + status + wxT("]");

      status << wxT(" (") << sizify(current) << wxT(" / ")
             << sizify(total) << wxT(")");
      statusStr = status;
    }
}

void GameInf::updateAll()
{
  const TigEntry *ent = info.ent;

  title = strToWx(ent->tigInfo.title);
  desc = strToWx(ent->tigInfo.desc);

  dlStr = wxString::Format(wxT("%d"), ent->dlCount);
  if(ent->rateCount > 0 && ent->rating > 0)
    {
      rateStr = wxString::Format(wxT("%3.2f"), ent->rating);
      rateStr2 = rateStr + wxString::Format(wxT(" (%d)"), ent->rateCount);
    }

  timeStr = ago(ent->addTime);

  updateStatus();
}

std::string GameInf::getHomepage() const
{ return info.ent->tigInfo.homepage; }
std::string GameInf::getTiggitPage() const
{ return "http://tiggit.net/game/" + info.ent->urlname; }
std::string GameInf::getIdName() const
{ return info.ent->idname; }
std::string GameInf::getDir() const
{ return ""; }
int GameInf::myRating() const
{ return -1; }

void GameInf::rateGame(int i)
{}

// Called when a screenshot file is ready, possibly called by TigLib
// from a worker thread, but may also be called directly from
// requestShot().
void GameInf::shotIsReady(const std::string &idname,
                          const std::string &file)
{
  assert(idname == getIdName());

  // Are we in unloaded-mode?
  if(loaded == 1)
    {
      // Load the image file
      wxLogNull dontLog;
      if(!screenshot.LoadFile(strToWx(file)))
        return;

      loaded = 2;
    }

  // We should now be loaded
  assert(loaded == 2);

  // Pass the event to the wxEvtHandler. It can handle threaded
  // queuing.
  ScreenshotEvent evt;
  evt.id = idname;
  evt.shot = &screenshot;
  shotHandler->AddPendingEvent(evt);
}

void GameInf::requestShot(wxEvtHandler *cb)
{
  // Don't do anything if we are already working or if there was a
  // failure.
  if(loaded == 1) return;

  // Store the handler so shotIsReady() finds it
  shotHandler = cb;

  // If the shot was already loaded, just return it directly
  if(loaded == 2)
    {
      shotIsReady(getIdName(), "");
      return;
    }

  assert(loaded == 0);

  // Set 'working' status
  loaded = 1;

  // Delegate file fetching to TigLib
  info.requestShot(this);
}

void GameInf::installGame()
{}
void GameInf::uninstallGame()
{}
void GameInf::abortJob()
{}
void GameInf::launchGame()
{}

void GameList::addListener(wxGameListener *p)
{
  listeners.insert(p);
  p->list = this;
}

void GameList::removeListener(wxGameListener *p)
{
  p->list = NULL;
  listeners.erase(p);
}

void GameList::notifyListChange()
{
  std::set<wxGameListener*>::iterator it;
  for(it = listeners.begin(); it != listeners.end(); it++)
    (*it)->gameListChanged();
}

void GameList::notifyInfoChange()
{
  std::set<wxGameListener*>::iterator it;
  for(it = listeners.begin(); it != listeners.end(); it++)
    (*it)->gameInfoChanged();
}

void Notifier::notify() { lst->notifyListChange(); }

void GameList::flipReverse() { lister.flipReverse(); }
void GameList::setReverse(bool b) { lister.setReverse(b); }
void GameList::clearTags() { setTags(""); }
void GameList::setTags(const std::string &) {}
void GameList::setSearch(const std::string &str) { lister.setSearch(str); }

bool GameList::sortTitle()
{
  lister.sortTitle();
  return setStat(SS_TITLE);
}
bool GameList::sortDate()
{
  lister.sortDate();
  return setStat(SS_DATE);
}
bool GameList::sortRating()
{
  lister.sortRating();
  return setStat(SS_RATING);
}
bool GameList::sortDownloads()
{
  lister.sortDownloads();
  return setStat(SS_DOWNLOADS);
}

int GameList::size() const { return lister.size(); }

static GameInf &ptrToInfo(const void *p)
{
  const LiveInfo *l = (const LiveInfo*)p;
  GameInf *gi = (GameInf*)l->extra;
  assert(gi);
  return *gi;
}

wxGameInfo& GameList::edit(int i)
{
  assert(i>=0 && i<size());
  return ptrToInfo(lister.getList()[i]);
}

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
  { return ptrToInfo(inf).isUninstalled(); }
};

static FreeDemoPick freePick(true), demoPick(false);
static InstalledPick instPick;

wxTigApp::GameData::GameData(Repo &rep)
  : config(rep.getPath("wxtiggit.conf")), repo(rep)
{
  rep.fetchFiles();
  rep.loadData();

  // Create GameInf structs attached to all the LiveInfo structs
  const List::PtrList &lst = rep.mainList().getList();
  for(int i=0; i<lst.size(); i++)
    {
      LiveInfo *li = (LiveInfo*)lst[i];
      assert(li->extra == NULL);
      li->extra = new GameInf(li, this);
    }

  // TODO: When updating the list, make sure to clean up and kill all
  // the GameInfs as well.

  latest = new GameList(rep.mainList(), NULL);
  freeware = new GameList(rep.mainList(), &freePick);
  demos = new GameList(rep.mainList(), &demoPick);
  installed = new GameList(rep.mainList(), &instPick);
}

wxTigApp::GameData::~GameData()
{
  delete latest;
  delete freeware;
  delete demos;
  delete installed;
}
