#include "misc/dirfinder.hpp"
#include "misc/lockfile.hpp"
#include "tasks/unpack.hpp"
#include "tasks/download.hpp"
#include "tiglib/gamedata.hpp"
#include "tiglib/gamelister.hpp"

#include <boost/filesystem.hpp>
#include <iostream>
#include <stdexcept>
#include <time.h>
#include <iomanip>

using namespace std;
using namespace TigData;

/* 
   GOAL: Do ALL the things covered by wxGameInfo, but without any wx
   dependency.

   Full list (in any order):

   - game install status
     - isInstalled() etc
     - job system
     - status notifications
   - starting, finishing and aborting game installs
   - launching games
   - rating games / sending download ticks
   - config files
   - fetching and loading news / read status
   - various fetches
     - includes cache fetching
     - fetching screenshots with callback

    You don't have to stay here. It's just an option. Feel free to
    work on modules instead if you want. Or do something entirely
    different.
 */

std::string getHomeDir()
{
  Misc::DirFinder find("tiggit.net", "tiggit");

  std::string res;
  if(find.getStoredPath(res))
    return res;

  if(!find.getStandardPath(res))
    {
      cout << "Failed to find any usable repo path";
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

struct TopPick : TigLib::GamePicker
{
  bool include(const TigEntry *ent)
  { return ent->rating >= 4.0; }
} topPick;

class Repo
{
  Misc::LockFile lock;
  std::string dir;

  TigLib::GameData data;
  TigLib::GameLister lister;

  std::string listFile, tigDir;

public:
  Repo(const std::string &where="")
    : dir(where), lister(data.allList)
  {
    if(dir == "")
      dir = getHomeDir();

    if(dir == "")
      fail("Could not find a standard repository path");

    if(!lock.lock(getPath("lock")))
      fail("Failed to lock " + dir);

    cout << "Found repo at: " << dir << endl;

    listFile = getPath("all_games.json");
    tigDir = getPath("tigfiles/");
  }

  std::string getPath(const std::string &fname)
  {
    return (boost::filesystem::path(dir)/fname).string();
  }

  void fetchFiles()
  {
    if(fetchIfOlder("http://tiggit.net/api/all_games.json",
                    listFile, 60))
      cout << "Fetched " << listFile << endl;
    else
      cout << "Used existing " << listFile << endl;
  }

  void loadData()
  {
    MyFetch fetch;
    data.data.addChannel(listFile, tigDir, &fetch);
    data.copyList();
    cout << "Loaded " << lister.getList().size() << " games\n";
  }

  static const TigEntry* tt(const void*p)
  { return (const TigEntry*)p; }

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
};

int main()
{
  Repo rep;
  rep.fetchFiles();
  rep.loadData();
  rep.print();
  return 0;
}
