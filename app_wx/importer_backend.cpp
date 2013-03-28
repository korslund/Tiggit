#include "importer_backend.hpp"

#include "../misc/freespace.hpp"
#include <spread/misc/readjson.hpp>
#include <spread/misc/jconfig.hpp>
#include <spread/job/thread.hpp>
#include <stdexcept>
#include <sstream>
#include <assert.h>
#include <boost/filesystem.hpp>

using namespace Spread;
using namespace Misc;
using namespace std;

namespace bf = boost::filesystem;

template <typename X>
static string toStr(X i)
{
  stringstream str;
  str << i;
  return str.str();
}

struct Copy
{
  JobInfoPtr info;
  SpreadLib *spread;
  Logger *logger;

  Copy() : spread(NULL), logger(NULL) {}

  void log(const string &msg)
  {
    if(logger) (*logger)(msg);
  }

  void fail(const string &msg)
  {
    log("ERROR: " + msg);
    throw runtime_error(msg);
  }

  void prog(int64_t cur, int64_t tot)
  {
    if(info) info->setProgress(cur, tot);
  }

  void doCopyFiles(const string &from, const string &to, bool addPng=false)
  {
    log("doCopyFiles FROM=" + from + " TO=" + to);

    assert(spread);
    assert(!info || info->isBusy());

    using namespace boost::filesystem;
    if(!exists(from) || !is_directory(from))
      fail("COPY: Source directory " + from + " not found!\n");

    // Absolute base paths
    path srcDir = absolute(from);
    path dstDir = absolute(to);

    // Base path without slash
    string base = (srcDir/"tmp").parent_path().string();
    int curlen = base.size() + 1;

    vector<string> fromList, toList;
    int64_t totalSize = 0;

    // Recurse the directory and list files to copy
    recursive_directory_iterator iter(srcDir), end;
    log("  Indexing " + srcDir.string());
    for(; iter != end; ++iter)
      {
        path p = iter->path();
        if(!is_regular_file(p)) continue;

        /* Remove base path from the name, retaining only the local
           path. Example:

           p = c:\path\to\dir\some-file\in-here\somewhere.txt
           base = c:\path\to\dir
           =>
           local = some-file\in-here\somewhere.txt
        */
        string infile = p.string();
        string local = infile.substr(curlen);
        string outfile = (dstDir/local).string();

        // Add PNG extension to screenshots
        if(addPng)
          if(outfile.size() <= 4 || outfile[outfile.size()-4] != '.')
            outfile += ".png";

        // Don't overwrite files
        if(exists(outfile))
          continue;

        // List file
        fromList.push_back(infile);
        toList.push_back(outfile);

        totalSize += file_size(p);
      }

    assert(fromList.size() == toList.size());

    prog(0, totalSize);
    log("  Found " + toStr(fromList.size()) + " files, total " + toStr(totalSize) + " bytes");
    // Check free disk space
    int64_t diskFree, diskTotal;
    // Make sure the destination dir exists first
    bf::create_directories(dstDir);
    Misc::getDiskSpace(dstDir.string(), diskFree, diskTotal);
    log("  Free disk space: " + toStr(diskFree) + " bytes");

    // This isn't exact, since required allocated size will often be
    // larger than file data size. But it will catch the most cases.
    if(diskFree < totalSize)
      fail("Not enough free disk space on destination drive " + dstDir.string());

    log("  Copying files...");
    spread->cacheCopy(fromList, toList, info);
    log("  Done");
  }
};

static void file2GameList(vector<string> &games, const string &dir,
                          const std::string &name, Misc::Logger &log)
{
  std::string infile = (bf::path(dir)/name).string();

  log("Reading game list from " + infile);
  if(!bf::exists(infile)) return;

  JConfig conf(infile);
  vector<string> names = conf.getNames();
  games.reserve(names.size());
  log(toStr(names.size()) + " games listed as installed:");
  for(int i=0; i<names.size(); i++)
    {
      // Game id, eg. "tiggit.net/dwarf-fortress"
      const string &id = names[i];
      log("  Found: " + id);

      // May need to convert slashes, since they were mangled in
      // previous versions for some reason.
      string idname;
      idname.reserve(id.size());
      for(int i=0; i<id.size(); i++)
        {
          char c = id[i];
          if(c == '\\') c = '/';
          idname += c;
        }

      if(idname != id)
        log("    Converted to: " + idname);

      // Store it
      games.push_back(idname);
    }
}

void Import::getGameList(vector<string> &games, const string &from,
                         Misc::Logger &log)
{
  /* Note: This does NOT work with games installed in non-standard
     locations.
   */
  file2GameList(games, from, "installed.json", log);
  file2GameList(games, from, "tiglib_installed.conf", log);
}

struct CopyJob : Spread::Job
{
  string fromDir, toDir, idname, outConf;
  Logger *log;
  SpreadLib *spread;
  bool addPng;

  CopyJob() : log(NULL), addPng(false) {}

  void doJob()
  {
    setBusy("Copying files");
    assert(spread);

    Copy cpy;
    cpy.spread = spread;
    cpy.info = info;
    cpy.logger = log;
    cpy.doCopyFiles(fromDir, toDir, addPng);

    if(checkStatus()) return;

    // Success. Write the entry to the output config file, if any.
    if(outConf != "" && idname != "")
      {
        if(log)
          (*log)("Updating " + outConf + " with " + idname + "=" + toDir);
        JConfig conf(outConf);
        conf.set(idname, toDir);
      }

    setDone();
  }
};

JobInfoPtr Import::copyFiles(const string &from, const string &to,
                             SpreadLib *spread, Misc::Logger *logger)
{
  if(logger) (*logger)("Copying " + from + " to " + to);

  if(bf::equivalent(from, to))
    return JobInfoPtr();

  CopyJob *job = new CopyJob;
  job->fromDir = from;
  job->toDir = to;
  job->log = logger;
  job->spread = spread;
  return Thread::run(job);
}

JobInfoPtr Import::importGame(const string &game,
                              const string &from, const string &to,
                              SpreadLib *spread, Misc::Logger &log, bool async)
{
  log("Importing " + game + " from " + from + " to " + to);

  if(bf::equivalent(from, to))
    {
      log("ERROR: from and to paths are equivalent");
      return JobInfoPtr();
    }

  string fromDir1 = (bf::path(from)/"data"/game).string();
  string fromDir2 = (bf::path(from)/"games"/game).string();
  string fromDir3 = (bf::path(from)/"gamedata"/game).string();

  string toDir = (bf::path(to)/"gamedata"/game).string();
  JobInfoPtr info;

  log("  fromDir1=" + fromDir1);
  log("  fromDir2=" + fromDir2);
  log("  fromDir3=" + fromDir3);
  log("  toDir=" + toDir);

  bool isOk = true;

  // Don't do anything if the destination exists
  if(bf::exists(toDir))
    {
      log("Destination already exists. Abort.");
      return info;
    }

  // Check destination config
  string outConf = (bf::path(to)/"tiglib_installed.conf").string();
  {
    JConfig conf(outConf);
    if(conf.has(game))
      {
        log("Game already registered in " + outConf + ". Abort.");
        return info;
      }
  }

  if(bf::exists(fromDir1) && bf::is_directory(fromDir1))
    {}
  else if(bf::exists(fromDir2) && bf::is_directory(fromDir2))
    fromDir1 = fromDir2;
  else if(bf::exists(fromDir3) && bf::is_directory(fromDir3))
    fromDir1 = fromDir3;
  else
    {
      log("No source directory found. Abort.");
      return info;
    }

  log("Picked source dir " + fromDir1);

  // Make sure to absolute() paths. If we are running in a thread, we
  // may risk changing the programs working directory while we are
  // working.
  fromDir1 = bf::absolute(fromDir1).string();
  toDir = bf::absolute(toDir).string();
  log("FINAL IN: " + fromDir1);
  log("FINAL OUT: " + toDir);

  CopyJob *job = new CopyJob;
  job->fromDir = fromDir1;
  job->toDir = toDir;
  job->idname = game;
  job->outConf = outConf;
  job->log = &log;
  job->spread = spread;
  job->addPng = false;
  return Thread::run(job, async);
}

JobInfoPtr Import::importShots(const string &from, const string &to,
                               Spread::SpreadLib *spread, Misc::Logger &log,
                               bool async)
{
  log("Importing screenshots from " + from + " to " + to);

  if(bf::equivalent(from, to))
    {
      log("ERROR: from and to paths are equivalent");
      return JobInfoPtr();
    }

  string fromDir = (bf::path(from)/"cache/shot300x260/tiggit.net").string();
  string fromDir2 = (bf::path(from)/"shots_300x260/tiggit.net").string();
  string toDir = (bf::path(to)/"shots_300x260/tiggit.net").string();
  JobInfoPtr info;

  if(!bf::exists(fromDir) || !bf::is_directory(fromDir))
    {
      fromDir = fromDir2;

      if(!bf::exists(fromDir) || !bf::is_directory(fromDir))
        {
          log("No screenshots found. Nothing to do.");
          return info;
        }
    }

  // Make sure to absolute() paths. If we are running in a thread, we
  // may risk changing the programs working directory while we are
  // working.
  fromDir = bf::absolute(fromDir).string();
  toDir = bf::absolute(toDir).string();
  log("FINAL IN: " + fromDir);
  log("FINAL OUT: " + toDir);

  CopyJob *job = new CopyJob;
  job->fromDir = fromDir;
  job->toDir = toDir;
  job->log = &log;
  job->spread = spread;
  job->addPng = true;
  return Thread::run(job, async);
}

void Import::cleanup(const string &from, const vector<string> &games,
                     Misc::Logger &log)
{
  bf::path fromDir = bf::absolute(from);
  log("Cleaning up " + fromDir.string());

  // Fixed file names to delete from the old repo
  const char* files[] =
    {
      // Old repo names
      "all_games.json", "auth.json", "config", "installed.json", "latest.tig",
      "news.json", "promo.json", "ratings.json", "readnews.json", "cache",
      "incoming", "tigfiles",

      // New repo names
      "stats.json", "tiglib.conf", "tiglib_installed.conf", "tiglib_news.conf",
      "tiglib_rates.conf", "wxtiggit.conf",

      // Terminator
      ""
    };

  vector<bf::path> kill;
  kill.reserve(games.size() + 12);

  // Add the fixed list from above
  {
    int i = 0;
    while(true)
      {
	const string &file = files[i++];
	if(file == "") break;

        kill.push_back(fromDir/file);
      }
  }

  /*
    Add games from parameter list.

    We do NOT remove the "games" and "data" directories themselves,
    since these might contain games that failed to import, and we
    don't want to just kill the user's savegames and data without
    asking.
  */
  for(int i=0; i<games.size(); i++)
    {
      bf::path dir = fromDir / "games" / games[i];
      if(!bf::exists(dir))
        {
          dir = fromDir / "data" / games[i];
          if(!bf::exists(dir))
            {
              dir = fromDir / "gamedata" / games[i];
              if(!bf::exists(dir))
                {
                  log("WARNING: Cleanup: Could not find requested game: " + games[i]);
                  continue;
                }
            }
        }
      kill.push_back(dir);
    }

  // Kill all the files and directories found.
  for(int i=0; i<kill.size(); i++)
    {
      const bf::path &p = kill[i];
      log("Deleting " + p.string());
      try
        {
          if(bf::exists(p))
            bf::remove_all(p);
          else
            log("  Not found, ignoring.");
        }
      catch(exception &e) { log("  Failed: " + string(e.what())); }
      catch(...) { log("  Failed: Unknown error"); }
    }
}

static void copyFile(const std::string &fromDir,
                      const std::string &toDir,
                      const std::string &name)
{
  try
    {
      bf::path from = fromDir;
      bf::path to = toDir;

      from /= name;
      to /= name;

      if(exists(from) && !exists(to))
        copy_file(from, to);
    }
  catch(...) {}
}

void Import::importConfig(const string &from, const string &to,
                          Misc::Logger &log)
{
  log("Converting config options from " + from + " to " + to);

  if(bf::equivalent(from, to))
    {
      log("ERROR: from and to paths are equivalent");
      return;
    }

  bf::path fromDir = from;
  bf::path toDir = to;

  using namespace Misc;

  // Copy over new files first, if present
  copyFile(from, to, "tiglib.conf");
  copyFile(from, to, "tiglib_news.conf");
  copyFile(from, to, "tiglib_rates.conf");
  copyFile(from, to, "wxtiggit.conf");
  copyFile(from, to, "stats.json");
  copyFile(from, to, "news.json");

  // Convert main config file
  try
    {
      string infile = (fromDir/"config").string();
      string outlib = (toDir/"tiglib.conf").string();
      string outwx = (toDir/"wxtiggit.conf").string();

      log("Converting " + infile + " => " + outlib + ", " + outwx);

      if(bf::exists(infile))
        {
          JConfig in(infile);
          JConfig conf(outlib);
          JConfig wxconf(outwx);

          // Convert wxTiggit-specific options
          if(in.has("vote_count") && !wxconf.has("show_votes"))
            wxconf.setBool("show_votes", in.getBool("vote_count"));

          // Move over the timestamp of the last seen game to the new
          // config file, if necessary
          if(in.has("last_time"))
            {
              int oldTime = in.getInt("last_time");

              // Check the new time
              int64_t newTime = 0;
              if(conf.has("last_time"))
                newTime = conf.getInt64("last_time");

              log("  oldTime=" + toStr(oldTime) + " newTime=" + toStr(newTime));

              // Update the time to the highest of the two
              if(oldTime > newTime)
                conf.setInt64("last_time", oldTime);
            }
          log("  Done");
        }
      else log("  No input file found, skipping.");
    }
  catch(exception &e) { log("ERROR: " + string(e.what())); }
  catch(...) { log("ERROR: Unknown"); }

  // Merge in list of read news items, so old items don't show up as
  // unread
  try
    {
      string infile = (fromDir/"readnews.json").string();
      string outfile = (toDir/"tiglib_news.conf").string();

      log("Converting " + infile + " => " + outfile);

      if(bf::exists(infile))
        {
          JConfig news(outfile);

          // Convert JSON array to config values
          Json::Value root = ReadJson::readJson(infile);
          for(int i=0; i<root.size(); i++)
            {
              // The old values were arrays of ints. We are now using
              // string:bool.
              int val = root[i].asInt();
              string key = toStr(val);
              news.setBool(key, true);
            }
          log("  Done");
        }
      else log("  No input file found, skipping.");
    }
  catch(exception &e) { log("ERROR: " + string(e.what())); }
  catch(...) { log("ERROR: Unknown"); }

  // Finally convert the list of games the user has rated, and their
  // ratings, so the new UI reflects this
  try
    {
      string infile = (fromDir/"ratings.json").string();
      string outfile = (toDir/"tiglib_rates.conf").string();

      log("Converting " + infile + " => " + outfile);

      if(bf::exists(infile))
        {
          JConfig in(infile);
          JConfig rates(outfile);

          vector<string> names = in.getNames();
          for(int i=0; i<names.size(); i++)
            {
              const string &key = names[i];

              /* May need to convert slashes, since they were mangled
                 in previous versions for some reason. Game id names
                 are internal identifiers and have nothing to do with
                 file system names. They should therefore always use
                 forward (/) slashes.
              */
              string outkey;
              outkey.reserve(key.size());
              for(int i=0; i<key.size(); i++)
                {
                  char c = key[i];
                  if(c == '\\') c = '/';
                  outkey += c;
                }

              // Don't overwrite existing values
              if(rates.has(outkey)) continue;

              int rate = in.getInt(key);
              if(rate < 0 || rate > 5) continue;
              rates.setInt(outkey, rate);
            }
          log("  Done");
        }
      else log("  No input file found, skipping.");
    }
  catch(exception &e) { log("ERROR: " + string(e.what())); }
  catch(...) { log("ERROR: Unknown"); }
}
