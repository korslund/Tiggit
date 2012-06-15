#include "gamedata.hpp"

#include "misc/dirfinder.hpp"
#include "misc/lockfile.hpp"
#include "tasks/unpack.hpp"
#include "tasks/download.hpp"
#include "tiglib/gamedata.hpp"

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <time.h>

#include <iostream>
using namespace std;

using namespace TigData;

/* 
   Modularize:
   - see if you could move more stuff to tiglib
   - fix up the status stuff there, in a struct much like GameInf
     here. Drop the hashmap solution and use shared_ptr extra-infos
     instead. Or I guess you'll end up using hashmaps behind the
     scenes anyway, unless you replace the void* in the base
     lists. And that might be just as good a solution, actually.

   Backend:
   - config files
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

std::string getHomeDir()
{
  Misc::DirFinder find("tiggit.net", "tiggit");

  std::string res;
  if(find.getStoredPath(res))
    return res;

  if(!find.getStandardPath(res))
    {
      //cout << "Failed to find any usable repo path";
      return "";
    }

  find.setStoredPath(res);
  return res;
}

void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void fetchFile(const std::string &url, const std::string &outfile)
{
  using namespace Jobify;

  JobInfoPtr info(new JobInfo);
  Tasks::DownloadTask dl(url, outfile, info);
  dl.run();
  if(!info->isSuccess())
    fail("Failed to download " + url);
}

bool fetchIfOlder(const std::string &url,
                  const std::string &outfile,
                  int minutes)
{
  namespace bs = boost::filesystem;

  // Check age if file exists
  if(bs::exists(outfile))
    {
      time_t ft = bs::last_write_time(outfile);
      time_t now = time(0);

      // If we haven't expired, keep existing file
      if(difftime(now,ft) < 60*minutes)
        return false;
    }

  // Otherwise, go get it!
  fetchFile(url, outfile);
  return true;
}

struct MyFetch : GameInfo::URLManager
{
  void getUrl(const std::string &url, const std::string &outfile)
  { fetchFile(url, outfile); }
};

struct Repo
{
  Misc::LockFile lock;
  std::string dir;

  TigLib::GameData data;

  std::string listFile, tigDir;

  Repo(const std::string &where="")
    : dir(where)
  {
    if(dir == "")
      dir = getHomeDir();

    if(dir == "")
      fail("Could not find a standard repository path");

    if(!lock.lock(getPath("lock")))
      fail("Failed to lock " + dir);

    //cout << "Found repo at: " << dir << endl;

    listFile = getPath("all_games.json");
    tigDir = getPath("tigfiles/");
  }

  std::string getPath(const std::string &fname)
  {
    return (boost::filesystem::path(dir)/fname).string();
  }

  void fetchFiles()
  {
    fetchIfOlder("http://tiggit.net/api/all_games.json",
                 listFile, 60);
  }

  void loadData()
  {
    MyFetch fetch;
    data.data.addChannel(listFile, tigDir, &fetch);
    data.copyList();
  }

  static const TigEntry* tt(const void*p)
  { return (const TigEntry*)p; }

  /*
  void print()
  {
    //lister.setPick(&topPick);
    lister.sortRating();
    //lister.setSearch("ab");
    //lister.setReverse(true);

    cout << "\n  " << setw(40) << left << "Title"
         << "   " << setw(10) << "Rating"
         << "   " << setw(10) << "Downloads\n\n";

    const List::PtrList &games = lister.getList();
    for(int i=0; i<games.size(); i++)
      {
        const TigEntry *ent = tt(games[i]);
        std::string title = ent->tigInfo.title;
        if(title.size() > 35)
          title = title.substr(0,35) + "...";

        cout << "  " << setw(40) << left << title
             << "   " << setw(10) << ent->rating
             << "   " << setw(10) << ent->dlCount
             << endl;

        if(i > 30)
          {
            cout << "...\n";
            break;
          }
      }
  }
  */
};

Repo rep;

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

   Fix both issues later.
 */

// This is used frequently, so let's use a hashmap instead of the
// slower std::map
static boost::unordered_map<const void*, GameInf*> ptrmap;

GameInf &ptrToInfo(const void *p)
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

bool GameConf::getShowVotes() { return false; }
void GameConf::setShowVotes(bool) {}

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

GameData::GameData()
  : latest(rep.data.allList, NULL)
  , freeware(rep.data.allList, &freePick)
  , demos(rep.data.allList, &demoPick)
  , installed(rep.data.allList, &instPick)
{
  rep.fetchFiles();
  rep.loadData();
}
