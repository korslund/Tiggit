#ifndef __REPO_HPP_
#define __REPO_HPP_

// TODO: Will incorporate filegetter into this file soon, and move the
// shared Config instance into the Repository object.
#include "filegetter.hpp"
#include "config.hpp"
#include "readjson.hpp"
#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>

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

  // TODO: Make non-static, kill all global variables, and pass along
  // paths.
  static void setupPaths(const std::string &exePath,
                         const std::string &appData)
  {
    using namespace boost::filesystem;
    using namespace Json;
    using namespace std;

    path exe = exePath;
    path exeDir = exe.parent_path();
    path pathfile = exeDir / "paths.json";

    path repoDir;
    bool upgrading = false; // We are upgrading from an old directory

    // If the paths file exists, use it
    if(exists(pathfile))
      {
        try
          {
            Value root = readJson(pathfile.string());
            repoDir = root["repo_dir"].asString();
          }
        // Failure to set repoDir is handled below
        catch(...) {}
      }
    else
      {
        // Assume missing dir means we are converting from the old system
        path appdata = appData;

        // If there is a 'config' file there, then we've found our
        // repository
        if(exists(appdata / "config"))
          {
            // Use the old location.
            repoDir = appdata;
            upgrading = true;

            // Use portable paths if possible
            if(exeDir == repoDir / "bin")
              repoDir = "../";

            writePaths(exeDir.string(), repoDir.string());
          }
      }

    // Our repoDir may still be empty at this point
    if(repoDir.empty())
      {
        /* If we get here, it means nothing was found. We assume this
           means a new installation.

           Use data/ as the default directory for now.
        */
        repoDir = "data";
        writePaths(exeDir.string(), repoDir.string());
        setDir(repoDir, exeDir);

        // Ask the user where they want to store data
        //moveDir(repoDir, exeDir);
      }
    else
      {
        setDir(repoDir, exeDir);

        // Be backwards compatible with old repositories
        if(upgrading && is_directory(repoDir / "data"))
          conf.setGameDir("data");
      }
  }

  static void setDir(Path repoDir, const Path &exeDir)
  {
    // Expand relative path for internal use, to make sure everything
    // still works when we change our working directory.
    if(!repoDir.has_root_path())
      repoDir = absolute_path(repoDir, exeDir);

    get.setBase(repoDir);
    conf.load(repoDir.string());
  }

  // Writes dest_dir / "paths.json"
  static void writePaths(const std::string &dest_dir,
                         const std::string &value)
  {
    Json::Value root;
    Path pathfile = dest_dir;
    pathfile /= "paths.json";
    root["repo_dir"] = value;
    writeJson(pathfile.string(), root);
  }
};

#endif
