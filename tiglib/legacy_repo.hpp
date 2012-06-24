/* Check a directory if it has an old-style repository
 */
static bool hasRepo(boost::filesystem::path &dir)
{
  Misc::DirFinder dfinder("tiggit.net", "tiggit");

  using namespace boost::filesystem;

  // A repo is found if it has a config file.
  path file = dir / "config";
  if(exists(file)) return true;

  /* On windows, legacy installations may have depended on UAC
     virtualization to store data in places normally not allowed.

     Once we have disabled virtualization for this app by including
     a manifest, those paths will stop working. We will therefore
     have to go hunting for that data on our own.
  */
#ifdef _WIN32
  // Get the relative version of the path, eg.:
  // C:\Program Files\Tiggit\   =>   Program Files\Tiggit       \
  dir = dir.relative_path();

  // Add the VirtualStore directory. New result would now be eg.:
  // C:\Users\myname\AppData\Local\VirtualStore\Program Files\Tiggit    \
  dir = path(dfinder.getAppData()) / "VirtualStore" / dir;

  // Return true if we found a repository
  file = dir / "config";
  if(exists(file)) return true;
#endif

  // Nothing was found
  return false;
}

  // Find and return the repository path.
  static std::string findRepoDir(const std::string &exePath,
                                 const std::string &appData)
  {
    using namespace boost::filesystem;
    using namespace Json;
    path p;

    // Check the standard path config location first
    {
      std::string res;
      if(dfinder.getStoredPath(res))
        {
          p = res;
          if(hasRepo(p)) return p.string();
        }
    }

    /* No stored path found. Try legacy locations. Everything below
       this line in this function will eventually be phased out and
       removed. We are keeping it now to give everyone on old
       installations time to convert. Ideally this entire function,
       and hasRepo(), will be removed.
    */

    path exeDir = exePath;
    exeDir = exeDir.parent_path();

    // Check for a paths.json co-located with the exe.
    {
      path pathfile = exeDir / "paths.json";
      if(exists(pathfile))
        {
          try
            {
              Value root = readJson(pathfile.string());
              p = root["repo_dir"].asString();

              if(p != "")
                {
                  // The paths file may have contained relative paths,
                  // convert them.
                  if(!p.has_root_path())
                    p = absolute_path(p, exeDir);

                  if(hasRepo(p)) return p.string();
                }
            }
          catch(...) {}
        }
    }

    // Check to see if we are upgrading from an old appdata location.
    p = appData;
    if(hasRepo(p)) return p.string();

    // Final solution: Check for a repository in the data/ folder
    // relative to the exe file.
    p = exeDir / "data";
    if(hasRepo(p)) return p.string();

    // Nothing found
    return "";
  }

  // Quick library version compatibility fix
  static Path absolute_path(const Path& p, const Path& base)
  {
    using namespace boost::filesystem;
#if BOOST_VERSION >= 104600 && BOOST_FILESYSTEM_VERSION >= 3
    return absolute(p, base);
#else
    return complete(p, base);
#endif
  }
};

// Should be used to upgrade an old repository to a new one.
bool Repo::upgradeRepo()
{
  assert(repo->isLocked());

  using namespace boost::filesystem;
  using namespace Misc;

  // Open config files
  conf.load(getPath("tiglib.conf"));
  inst.load(getPath("tiglib_installed.conf"));
  news.load(getPath("tiglib_news.conf"));
  // Opened further down:
  std::string rateConf = getPath("tiglib_rates.conf");

  // Is there an old config file?
  std::string oldcfg = getPath("config");
  if(exists(oldcfg))
    try
      {
        // Open the old config file to convert the values
        JConfig in(oldcfg);

        // Convert wxTiggit-specific options
        if(in.has("vote_count"))
          {
            std::string tmp = getPath("wxtiggit.conf");
            if(!exists(tmp))
              {
                JConfig out(tmp);
                out.setBool("show_votes", in.getBool("vote_count"));
              }
          }

        // Move last_time over to the new config.
        if(in.has("last_time") && !conf.has("last_time"))
          {
            int64_t oldTime = in.getInt("last_time");
            setLastTime(oldTime);
          }

        // Kill the old file
        remove(oldcfg);

        /* If the old 'config' file existed, chances are that this is
           an old repository. We should convert any other old data we
           can find as well.
         */

        // Rename the ratings file
        oldcfg = getPath("ratings.json");
        if(exists(oldcfg) && !exists(rateConf))
          rename(oldcfg, rateConf);

        // Convert list of read news
        oldcfg = getPath("readnews.json");
        if(exists(oldcfg))
          {
            // Convert JSON array to config value
            Json::Value root = ReadJson::readJson(oldcfg);
            for(int i=0; i<root.size(); i++)
              {
                // The old values were ints, now we are using strings.
                int val = root[i].asInt();
                char buf[10];
                snprintf(buf, 10, "%d", val);
                std::string key(buf);
                news.setBool(key, true);
              }

            remove(oldcfg);
          }

        // Convert the list of installed games
        oldcfg = getPath("installed.json");
        if(exists(oldcfg))
          {
            JConfig old(oldcfg);
            std::vector<std::string> list = old.getNames();

            for(int i=0; i<list.size(); i++)
              if(!inst.has(list[i]))
                setInstallStatus(list[i], 2);

            remove(oldcfg);
          }

        // Find and rename screenshot images (add .png to filenames)
        directory_iterator iter(getPath("cache/shot300x260/tiggit.net/")), end;
        for(; iter != end; ++iter)
          {
            path p = iter->path();
            if(!is_regular_file(p)) continue;

            // Check file ending
            std::string name = p.string();
            if(name.size() > 4 && name.substr(name.size()-4, 1) == ".")
              continue;

            // Rename to .png
            rename(name, name+".png");
          }

        // In some really old repos, the games may be installed into
        // "data/" instead of "games/". If so, rename it.
        if(exists(getPath("data/")) && !exists(getPath("games/")))
          rename(getPath("data"), getPath("games"));
      }
    catch(...) {}
}
