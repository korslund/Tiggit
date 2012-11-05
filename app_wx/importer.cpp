#include "importer.hpp"

#include "../misc/logger.hpp"
#include "../misc/freespace.hpp"
#include <spread/spread.hpp>
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

void wxTigApp::copyTest(const std::string &from, const std::string &to, bool addPng,
                        SpreadLib *spread, JobInfoPtr info,
                        Misc::Logger *logger)
{
  Copy cpy;
  cpy.spread = spread;
  cpy.info = info;
  cpy.logger = logger;
  cpy.copyFiles(from, to, addPng);
}

// Old code:
/*
void readOldConf(JConfig &conf, JConfig &wxconf)
{
  try
    {
      bf::path file = fromDir/"config";
      if(!bf::exists(file)) return;

      JConfig in(file.string());

      // Convert wxTiggit-specific options
      if(in.has("vote_count") && !wxconf.has("show_votes"))
        wxconf.setBool("show_votes", in.getBool("vote_count"));

      // Move last_time over to the new config.
      if(in.has("last_time") && !conf.has("last_time"))
        {
          uint32_t oldTime = in.getInt("last_time");
          conf.setInt64("last_time", oldTime);
        }
    }
  catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
  catch(...) { PRINT("ERROR: Unknown"); }
}

void readNewConf(JConfig &conf)
{
  try
    {
      bf::path file = fromDir/"tiglib.conf";
      if(!bf::exists(file)) return;

      JConfig in(file.string());
      if(in.has("last_time") && !conf.has("last_time"))
        conf.setInt64("last_time", in.getInt64("last_time"));
    }
  catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
  catch(...) { PRINT("ERROR: Unknown"); }
}

void readNewWx(JConfig &wxconf)
{
  try
    {
      bf::path file = fromDir/"wxtiggit.conf";
      if(!bf::exists(file)) return;

      JConfig in(file.string());
      if(in.has("show_votes") && !wxconf.has("show_votes"))
        wxconf.setBool("show_votes", in.getBool("show_votes"));
    }
  catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
  catch(...) { PRINT("ERROR: Unknown"); }
}

void readOldNews(JConfig &news)
{
  try
    {
      bf::path file = fromDir/"readnews.json";
      if(!bf::exists(file)) return;

      // Convert JSON array to config values
      Json::Value root = ReadJson::readJson(file.string());
      for(int i=0; i<root.size(); i++)
        {
          // The old values were ints, we are now using strings.
          int val = root[i].asInt();
          char buf[10];
          std::snprintf(buf, 10, "%d", val);
          std::string key(buf);
          news.setBool(key, true);
        }
    }
  catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
  catch(...) { PRINT("ERROR: Unknown"); }
}

void readNewNews(JConfig &news)
{
  try
    {
      bf::path file = fromDir/"tiglib_news.conf";
      if(!bf::exists(file)) return;

      JConfig in(file.string());

      std::vector<std::string> names = in.getNames();
      for(int i=0; i<names.size(); i++)
        news.setBool(names[i], true);
    }
  catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
  catch(...) { PRINT("ERROR: Unknown"); }
}

void readRates(const std::string &name, JConfig &rates)
{
  try
    {
      bf::path file = fromDir/name;
      if(!bf::exists(file)) return;

      JConfig in(file.string());

      std::vector<std::string> names = in.getNames();
      for(int i=0; i<names.size(); i++)
        {
          const std::string &key = names[i];

          // May need to convert slashes, since they were mangled in
          // previous versions for some reason.
          std::string outkey;
          outkey.reserve(key.size());
          for(int i=0; i<key.size(); i++)
            {
              char c = key[i];
              if(c == '\\') c = '/';
              outkey += c;
            }

          // Don't overwrite values
          if(rates.has(outkey)) continue;

          int rate = in.getInt(key);
          if(rate < 0 || rate > 5) continue;
          rates.setInt(outkey, rate);
        }
    }
  catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
  catch(...) { PRINT("ERROR: Unknown"); }
}

void readOldRates(JConfig &rates)
{
  readRates("ratings.json", rates);
}

void readNewRates(JConfig &rates)
{
  readRates("tiglib_rates.conf", rates);
}

void listOldGames()
{
  listGames("games", "installed.json");
  listGames("data", "installed.json");
}

void listNewGames()
{
  listGames("gamedata", "tiglib_installed.conf");
}

struct ImportJob : Job
{
  bf::path fromDir, toDir;

  void moveOldShots()
  {
    moveShots("cache/shot300x260/tiggit.net/");
  }

  void moveNewShots()
  {
    moveShots("shots_300x260/tiggit.net/");
  }

  void moveShots(const std::string &dir)
  {
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

  void listGames(const std::string &dir, const std::string &confFile)
  {
    using namespace boost::filesystem;
    JConfig conf((fromDir/confFile).string());

    path srcDir = fromDir/dir;
    path dstDir = toDir/"gamedata";

    assert(srcDir != dstDir);

    // Traverse the list of installed games in the source repo
    std::vector<std::string> names = conf.getNames();
    for(int i=0; i<names.size(); i++)
      {
        // Game id, eg. "tiggit.net/dwarf-fortress"
        const std::string &id = names[i];

        PRINT("\nGAME: id=" << id);

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

        PRINT("idname=" << idname);

        path
          src = srcDir / idname,
          dst = dstDir / idname;

        PRINT("src=" << src << " dst=" << dst);

        // Does the source dir exist?
        if(!(exists(src) && is_directory(src))) continue;

        PRINT("Source existed");

        // Does the destination exist? (We won't overwrite it)
        if(exists(dst)) continue;

        PRINT("Destination did not exist. ADDED!");

        // Add the source path to our copy list
        games.push_back(src.string());
      }
  }

  void moveGame(const std::string &src, JConfig &inst)
  {
    PRINT("\nMOVING game " << src);
    try
      {
        using namespace boost::filesystem;
        assert(exists(src) && is_directory(src));

        path dest = src;
        const std::string &idname = "tiggit.net/" + dest.filename().string();

        PRINT("idname=" << idname);

        // If the entry already exists in the destination config,
        // don't do anything.
        if(inst.has(idname) && inst.getInt(idname) != 0) return;

        dest = toDir / "gamedata" / idname;
        create_directories(dest.parent_path());

        PRINT("Moving to: " << dest);

        DirCopy::moveDir(src, dest.string());

        // Add the entry to the config.
        inst.setInt(idname, 2);
      }
    catch(std::exception &e) { PRINT("ERROR: " << e.what()); }
    catch(...) { PRINT("ERROR: Unknown"); }
  }

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

  void doJob()
  {
    setBusy("Importing " + fromDir.string() + " into " + toDir.string());
    PRINT("Importing " << fromDir << " into " << toDir);

    // Make sure destionation exists
    create_directories(toDir);

    // Open destination config files
    JConfig conf, inst, news, rates, wxconf;
    conf.load((toDir/"tiglib.conf").string());
    inst.load((toDir/"tiglib_installed.conf").string());
    news.load((toDir/"tiglib_news.conf").string());
    rates.load((toDir/"tiglib_rates.conf").string());
    wxconf.load((toDir/"wxtiggit.conf").string());

    // We are allowed to import into ourselves (when upgrading a
    // repository), but in that case we only allow upgrading from
    // legacy data.
    bool toSelf = (fromDir == toDir) || bf::equivalent(fromDir, toDir);

    PRINT("Converting config files");
    readOldConf(conf, wxconf);
    readOldNews(news);
    readOldRates(rates);
    listOldGames();
    if(!toSelf)
      {
        readNewConf(conf);
        readNewWx(wxconf);
        readNewNews(news);
        readNewRates(rates);
        listNewGames();
      }

    // list*Games() has now built lists of game directories we need to
    // move. The lists only contain confirmed moves, ie. moves where
    // we know the source exists and the destination does not. This
    // gives us a good basis for the progress reports as well.

    setProgress(0, games.size() + 2);

    // Move over all the screenshots
    moveOldShots();
    if(!toSelf)
      moveNewShots();

    setProgress(1);

    for(int i=0; i<games.size(); i++)
      {
        moveGame(games[i], inst);
        setProgress(i+1);
      }

    // Clean out the entire imported directory. We might create a
    // copy-not-move import later, but we don't need it yet.
    deleteEverything();
    setProgress(games.size() + 2);

    PRINT("Import done!");

    setDone();
  }
};

JobInfoPtr TigLibInt::importRepo(const std::string &input,
                                 const std::string &output,
                                 bool async)
{
  ImportJob *job = new ImportJob;
  job->fromDir = input;
  job->toDir = output;
  return Thread::run(job,async);
}
*/
