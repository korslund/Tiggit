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
#include "wx/dialogs.hpp"

// More ad-hoc crap, kill soon
DirFinder::Finder dfinder("tiggit.net", "tiggit");

struct Repository
{
  typedef boost::filesystem::path Path;

  /* Check a single path if it contains a repository. Returns true if
     found.

     NOTE that the dir parameter may change under some circumstances.
     In those cases the new value is the correct one. If the return
     value is false, the new value of 'dir' is undefined.
   */
  static bool hasRepo(Path &dir)
  {
    // A repo is found if it has a config file.
    Path file = dir / "config";
    if(exists(file)) return true;

    /* On windows, legacy installations may have depended on UAC
       virtualization to store data in places normally not allowed.

       Once we have disabled virtualization for this app by including
       a manifest, those paths will stop working. We will therefore
       have to go hunting for that data on our own.
    */
#ifdef _WIN32
    // Get the relative version of the path, eg.:
    // C:\Program Files\Tiggit\   =>   Program Files\Tiggit\
    dir = dir.relative_path();

    // Add the VirtualStore directory. New result would now be eg.:
    // C:\Users\myname\AppData\Local\VirtualStore\Program Files\Tiggit\
    dir = Path(dfinder.getAppData()) / "VirtualStore" / dir;

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

  // Set up all paths. Returns true if OK, false if the program should
  // exit. TODO: Kill the appData parameter here, preferably both. We
  // can't have any wx dependencies here.
  static bool setupPaths(const std::string &exePath,
                         const std::string &appData)
  {
    using namespace boost::filesystem;

    std::string repoDir;

    // See if we can't find a repository
    repoDir = findRepoDir(exePath, appData);

    if(repoDir == "")
      // Nothing found, use default.
      dfinder.getStandardPath(repoDir);

    path repoPath = repoDir;

    bool askUser = false;

    // Ask the user if there is no existing repository
    if(!exists(repoPath/"config"))
      askUser = true;
    else
      // Ask user if the repository is not writable
      askUser = !dfinder.setStoredPath(repoDir);

    if(askUser)
      {
        bool writeFail = false;
        while(true)
          {
            // This prompts the user to input a directory
            OutputDirDialog dlg(NULL, repoDir, writeFail, true);

            // Exit if the user clicked cancel.
            if(!dlg.ok)
              return false;

            repoDir = dlg.path;

            // Continue if the path was accepted
            if(dfinder.setStoredPath(repoDir))
              break;

            // Ask again if the path is not acceptable
            writeFail = true;
          }
        repoPath = repoDir;
      }

    // Set up and load config
    get.setBase(repoDir);
    conf.load(repoDir);

    // Be backwards compatible when reading old repositories. Old
    // paths may use data/ instead of games/ for game data.
    if(!is_directory(repoPath/"games") && is_directory(repoPath/"data"))
      conf.setGameDir("data");

    return true;
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

#endif
