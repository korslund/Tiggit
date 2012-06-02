#ifndef __REPO_HPP_
#define __REPO_HPP_

// TODO: Will incorporate filegetter into this file soon, and move the
// shared Config instance into the Repository object.
#include "filegetter.hpp"
#include "config.hpp"
#include "readjson.hpp"
#include "dirfinder/dirfinder.hpp"
#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>

// More ad-hoc crap, kill soon
DirFinder::Finder dfinder("tiggit.net", "tiggit");

struct Repository
{
  typedef boost::filesystem::path Path;

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

  // Find and return the repository path.
  static std::string findRepoDir(const std::string &exePath,
                          const std::string &appData)
  {
    using namespace boost::filesystem;
    using namespace Json;

    // Check the standard path config location first
    {
      std::string res;
      if(dfinder.getStoredPath(res))
        return res;
    }

    // No stored path found. Try legacy locations.
    path exeDir = exePath;
    exeDir = exeDir.parent_path();
    path p;

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

                  if(exists(p / "config")) return p.string();
                }
            }
          catch(...) {}
        }
    }

    // Check to see if we are upgrading from an old appdata location.
    p = appData;
    if(exists(p / "config")) return p.string();

    // Final solution: Check for a repository in the data/ folder
    // relative to the exe file.
    p = exeDir / "data";
    if(exists(p / "config")) return p.string();

    // Nothing found
    return "";
  }

  // TODO: Make non-static, kill all global variables, and pass along
  // paths to important objects.
  static void setupPaths(const std::string &exePath,
                         const std::string &appData)
  {
    using namespace boost::filesystem;

    std::string repoDir;

    // See if we can't find a repository
    repoDir = findRepoDir(exePath, appData);

    if(repoDir == "")
      {
        // Nothing found. Use default. TODO: Ask user.
        dfinder.getStandardPath(repoDir);
      }

    // Store the result for future use
    dfinder.setStoredPath(repoDir);

    // Set up and load config
    get.setBase(repoDir);
    conf.load(repoDir);

    // Be backwards compatible with old repositories. Old paths may
    // use data/ instead of games/ for game data.
    path rp = repoDir;
    if(!is_directory(rp/"games") && is_directory(rp/"data"))
      conf.setGameDir("data");
  }
};

#endif
