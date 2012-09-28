#include "repo_import.hpp"

#include <spread/misc/readjson.hpp>
#include <spread/misc/jconfig.hpp>
#include <spread/job/thread.hpp>
#include <boost/filesystem.hpp>
#include <cstdio>

namespace bf = boost::filesystem;
using namespace Spread;
using namespace Misc;

// Uncommenting this disables all file moves and deletes in the source
// repository. It might still create files and directories in the
// output repo.
#define DRY_RUN
#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

struct ImportJob : Job
{
  bf::path fromDir, toDir;

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
    catch(...) {}
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
    catch(...) {}
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
    catch(...) {}
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
    catch(...) {}
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
    catch(...) {}
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
    catch(...) {}
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

    path destDir = toDir/"shots_300x260/tiggit.net/";

    // Make sure destination directory exists
    create_directories(destDir);

    // Move all the screenshot files
    directory_iterator iter(fromDir/dir), end;
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
#ifndef DRY_RUN
        rename(source, dest);
#endif
      }
    } catch(...) {}
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

#ifndef DRY_RUN
        rename(src, dest);
#endif

        // Add the entry to the config.
        inst.setInt(idname, 2);
      }
    catch(...) {}
  }

  void deleteEverything()
  {
    /* Delete everything we know about from the imported directory
     */
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
#ifndef DRY_RUN
                bf::remove_all(p);
#endif
              }
          }
        catch(...) {}
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

    /* list*Games() has now built lists of game directories we need to
       move. The lists only contain confirmed moves, ie. moves where
       we know the source exists and the destination does not. This
       gives us a good basis for the progress reports as well.
     */

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
