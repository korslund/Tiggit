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
namespace bf = boost::filesystem;

template <typename X>
static std::string toStr(X i)
{
  std::stringstream str;
  str << i;
  return str.str();
}

struct Copy
{
  JobInfoPtr info;
  SpreadLib *spread;
  Logger *logger;

  Copy() : spread(NULL), logger(NULL) {}

  void log(const std::string &msg)
  {
    if(logger) (*logger)(msg);
  }

  void fail(const std::string &msg)
  {
    log("ERROR: " + msg);
    throw std::runtime_error(msg);
  }

  void prog(int64_t cur, int64_t tot)
  {
    if(info) info->setProgress(cur, tot);
  }

  void copyFiles(const std::string &from, const std::string &to, bool addPng=false)
  {
    log("copyFiles FROM=" + from + " TO=" + to);

    assert(spread);
    assert(!info || info->isBusy());

    using namespace boost::filesystem;
    if(!exists(from) || !is_directory(from))
      fail("COPY: Source directory " + from + " not found!\n");

    // Absolute base paths
    path srcDir = absolute(from);
    path dstDir = absolute(to);

    // Base path without slash
    std::string base = (srcDir/"tmp").parent_path().string();
    int curlen = base.size() + 1;

    std::vector<std::string> fromList, toList;
    int64_t totalSize = 0;

    // Recurse the directory
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
        std::string infile = p.string();
        std::string local = infile.substr(curlen);
        std::string outfile = (dstDir/local).string();

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

    int64_t cur = 0;
    for(int i=0; i<fromList.size(); i++)
      {
        // If this is run in a Job thread, we should check regularly
        // for user aborts.
        if(info && info->checkStatus())
          return;

        const std::string &src = fromList[i];
        const std::string &dst = toList[i];

        log("  Copying " + src + " => " + dst);

        // Let Spread perform the copying, as it will hash and index
        // files in the process.
        spread->cacheCopy(src, dst);

        int64_t size = file_size(src);
        cur += size;
        prog(cur, totalSize);
      }
  }
};

void Import::copyFiles(const std::string &from, const std::string &to, bool addPng,
                       SpreadLib *spread, JobInfoPtr info,
                       Misc::Logger &logger)
{
  Copy cpy;
  cpy.spread = spread;
  cpy.info = info;
  cpy.logger = &logger;
  cpy.copyFiles(from, to, addPng);
}

void Import::getGameList(std::vector<std::string> &games, const std::string &from,
                         Misc::Logger &log)
{
  using namespace std;

  string infile = (bf::path(from)/"installed.json").string();
  log("Reading game list from " + infile);

  JConfig conf(infile);
  vector<string> names = conf.getNames();
  games.reserve(names.size());
  log(toStr(names.size()) + " games listed as installed:");
  for(int i=0; i<names.size(); i++)
    {
      // Game id, eg. "tiggit.net/dwarf-fortress"
      const std::string &id = names[i];
      log("  Found: " + id);

      // May need to convert slashes, since they were mangled in
      // previous versions for some reason.
      std::string idname;
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

struct CopyJob : Spread::Job
{
  std::string fromDir, toDir, idname, outConf;
  Logger *log;
  SpreadLib *spread;
  bool addPng;

  void doJob()
  {
    setBusy("Copying files");
    Import::copyFiles(fromDir, toDir, addPng, spread, info, *log);
    if(checkStatus()) return;

    // Success. Write the entry to the output config file, if any.
    if(outConf != "" && idname != "")
      {
        assert(!addPng);
        (*log)("Updating " + outConf + " with " + idname + "=" + toDir);
        JConfig conf(outConf);
        conf.set(idname, toDir);
      }

    setDone();
  }
};

JobInfoPtr Import::importGame(const std::string &game,
                              const std::string &from, const std::string &to,
                              SpreadLib *spread, Misc::Logger &log, bool async)
{
  using namespace std;

  log("Importing " + game + " from " + from + " to " + to);

  string fromDir1 = (bf::path(from)/"data"/game).string();
  string fromDir2 = (bf::path(from)/"games"/game).string();
  string toDir = (bf::path(to)/"gamedata"/game).string();
  JobInfoPtr info;

  log("  fromDir1=" + fromDir1);
  log("  fromDir2=" + fromDir2);
  log("  toDir=" + toDir);

  bool isOk = true;

  // Don't do anything if the destination exists
  if(bf::exists(toDir))
    {
      log("Destination already exists. Abort.");
      return info;
    }

  // Check destination config
  std::string outConf = (bf::path(to)/"tiglib_installed.conf").string();
  {
    JConfig conf(outConf);
    if(conf.has(game))
      {
        log("Game already registered in " + outConf + ". Abort.");
        return info;
      }
  }

  if(!bf::exists(fromDir1) || !bf::is_directory(fromDir1))
    {
      if(!bf::exists(fromDir2) || !bf::is_directory(fromDir2))
        {
          log("No source directory found. Abort.");
          return info;
        }

      fromDir1 = fromDir2;
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

JobInfoPtr Import::importShots(const std::string &from, const std::string &to,
                               Spread::SpreadLib *spread, Misc::Logger &log,
                               bool async)
{
  /*
    List:
    - base this on the same job as importGame uses, just with slightly
      different parameters

    - integrate old code from below
   */
  assert(0);
}

void Import::cleanup(const std::string &from, std::vector<std::string> &games,
                     Misc::Logger &log)
{
  assert(0);
}

void Import::importConfig(const std::string &from, const std::string &to,
                          Misc::Logger &log)
{
  log("Converting config options from " + from + " to " + to);

  bf::path fromDir = from;
  bf::path toDir = to;

  using namespace Misc;
  using namespace std;

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
  catch(std::exception &e) { log("ERROR: " + string(e.what())); }
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
              std::string key = toStr(val);
              news.setBool(key, true);
            }
          log("  Done");
        }
      else log("  No input file found, skipping.");
    }
  catch(std::exception &e) { log("ERROR: " + string(e.what())); }
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
  catch(std::exception &e) { log("ERROR: " + string(e.what())); }
  catch(...) { log("ERROR: Unknown"); }
}

/*
  void moveShots(const std::string &dir)
  {
    moveShots("cache/shot300x260/tiggit.net/");
    try {
      using namespace boost::filesystem;

      path srcDir = fromDir/dir;

      if(!exists(srcDir) || !is_directory(srcDir))
        return;

      path destDir = toDir/"shots_300x260/tiggit.net/";

      // Make sure destination directory exists
      create_directories(destDir);

      // Move all the screenshot files
      directory_iterator iter(srcDir), end;
      for(; iter != end; ++iter)
        {
          path source = iter->path();
          PRINT("\nSHOT: input=" << source);
          if(!is_regular_file(source)) continue;

          // Get file name
          std::string name = source.filename().string();

          // Check if the file has an extension, or add one if necessary
          if(name.size() <= 4 || name[name.size()-4] != '.')
            name += ".png";

          PRINT("name=" << name);

          path dest = destDir/name;

          // Don't overwrite files
          if(exists(dest)) continue;

          PRINT("Moving to: " << dest);

          // Move the file
          DirCopy::moveFile(source.string(), dest.string());
        }
    }
    catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
    catch(...) { PRINT("ERROR: Unknown"); }
  }

  std::vector<std::string> games;

  void deleteEverything()
  {
    // Delete everything we know about from the imported directory
    const char* files[] =
      {
	// Legacy repo files
	"all_games.json", "auth.json", "config", "installed.json", "latest.tig",
	"news.json", "promo.json", "ratings.json", "readnews.json", "cache",
	"data", "games", "incoming", "tigfiles",

	// Current repo files
	"gamedata", "launch.log", "launch.log.old", "run", "shots_300x260",
	"spread", "stats.json", "tiglib.conf", "tiglib.conf.old",
	"tiglib_installed.conf", "tiglib_installed.conf.old", "tiglib_news.conf",
	"tiglib_news.conf.old", "tiglib_rates.conf", "tiglib_rates.conf.old",
	"update.log", "update.log.old", "wxtiggit.conf", "wxtiggit.conf.old",

	// Terminator
	""
      };

    int i = 0;
    while(true)
      {
	const std::string &file = files[i++];
	if(file == "") break;

	try
	  {
	    bf::path p = fromDir/file;
	    if(bf::exists(p))
	      {
		PRINT("Deleting " << p);
		bf::remove_all(p);
	      }
	  }
        catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
        catch(...) { PRINT("ERROR: Unknown"); }
      }
  }
*/
