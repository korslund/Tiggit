#include "gamedata.hpp"

#include <boost/unordered_map.hpp>
#include <time.h>

using namespace TigData;

/*
  You ALWAYS work on whatever you want. ALWAYS! You are not living or
  working for anyone else.

   Fix in tiglib:
   - special gameinfo-kind of structure there
     - then move that into the lists instead of using entries directly
     - can have extra-pointers or sharedptrs or whatever
   - hashmap solution
   - finally, fix up so we have everything behind a clean usable
     API. Then hide ALL the details from the user.

   Repo finder:
   - need tools to find and convert legacy repository locations and
     content. For example, we will need to convert our old config
     file.

   Backend:
   - fetching and loading news / read status
   - various fetches
     - includes cache fetching
     - fetching screenshots with callback
   - rating games / sending download ticks

   Tags:
   - tag list generator
   - working tag list selector

   Installing:
   - starting, finishing and aborting game installs
   - game install status
     - isInstalled() etc
     - job system
     - status notifications
   - launching games
 */

using namespace wxTigApp;

bool GameInf::isInstalled() const { return false; }
bool GameInf::isUninstalled() const { return !ent->tigInfo.isDemo; }
bool GameInf::isWorking() const { return ent->tigInfo.isDemo; }
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
{ return ent->tigInfo.homepage; }
std::string GameInf::getTiggitPage() const
{ return "http://tiggit.net/game/" + ent->urlname; }
std::string GameInf::getIdName() const
{ return ent->idname; }
std::string GameInf::getDir() const
{ return ""; }
int GameInf::myRating() const
{ return -1; }

void GameInf::rateGame(int i)
{}
void GameInf::requestShot(wxScreenshotCallback*)
{}

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

/* Lookup entry and find corresponding GameInfo.

   This is a quick hack with several problems. The biggest is what
   happens when reloading the data, we have to make sure to clear this
   out in that case.

   Another big problem is that this is a global variable, and we don't
   want that.

   This should be packed into some structure in tiglib, which again is
   passed to GameData.
 */

// This is used frequently, so let's use a hashmap instead of the
// slower std::map
static boost::unordered_map<const void*, GameInf*> ptrmap;

static GameInf &ptrToInfo(const void *p)
{
  GameInf *gi = ptrmap[p];
  if(!gi)
    {
      gi = new GameInf((const TigEntry*)p);
      ptrmap[p] = gi;
    }
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

struct FreeDemoPick : TigLib::GamePicker
{
  bool free;
  FreeDemoPick(bool _free) : free(_free) {}

  bool include(const TigEntry *ent)
  { return ent->tigInfo.isDemo != free; }
};

struct InstalledPick : TigLib::GamePicker
{
  bool include(const TigEntry *ent)
  { return !ptrToInfo(ent).isUninstalled(); }
};

static FreeDemoPick freePick(true), demoPick(false);
static InstalledPick instPick;

GameData::GameData(TigLib::Repo &rep)
  : latest(rep.mainList(), NULL)
  , freeware(rep.mainList(), &freePick)
  , demos(rep.mainList(), &demoPick)
  , installed(rep.mainList(), &instPick)
  , config(rep.getPath("wxtiggit.conf"))
{
  rep.fetchFiles();
  rep.loadData();
}
