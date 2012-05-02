#ifndef __REPO_HPP_
#define __REPO_HPP_

// TODO: Will incorporate filegetter into this file soon, and move the
// shared Config instance into the Repository object.
#include "filegetter.hpp"
#include "config.hpp"
#include "readjson.hpp"
#include <boost/filesystem.hpp>
#include <wx/stdpaths.h>
#include <string>
#include <stdexcept>

struct Repository
{
  // TODO: Make non-static, kill all global variables, and pass along
  // paths.
  static void setupPaths(const std::string &exePath,
                         const std::string &appData)
  {
    using namespace boost::filesystem;
    using namespace Json;
    using namespace std;

    /* For browsing dirs: ::wxDirSelector();

       General wx note: we should remove ALL wxWidgets influence on
       this file, and on anything outside the interface layer. Do
       dialogs through callbacks.
     */

    path exe = exePath;
    path exeDir = exe.parent_path();
    path pathfile = exeDir / "paths.json";

    path repoDir;
    bool write = false; // If true, write paths.json
    bool hasAsked = false; // Whether user has chosen directories
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
            write = true;
            upgrading = true;

            // Use portable paths if possible
            if(exeDir == repoDir / "bin")
              repoDir = "../";
          }
      }

    // Our repoDir may still be empty at this point
    if(repoDir.empty())
      {
        /* We we get here, it means nothing was found. We assume this
           means a new installation.

           If so, ask the user where they want to install stuff. We
           should also give the option of where to install the launcher
           itself, and whether to set up icons etc.

           We do not accept empty dirs as a response, that will reopen
           the window. The user can also abort, causing the program to
           exit.
        */

        /* If a portable_zip marker file is present, then ask the user
           if they want to use the current install location (which
           presumably is wherever they unpacked the zip file.)
         */
        bool isZipInstall = exists(exeDir/"portable_zip.txt");

        throw std::runtime_error("Cannot find repository. Installation not implemented yet.");
        // Always write result after asking the user
        write = true;
        hasAsked = true;
      }

    // We must have set a repository directory at this point, or failed.
    assert(!repoDir.empty());

    if(write)
      {
        Value root;
        root["repo_dir"] = repoDir.string();
        writeJson(pathfile.string(), root);
      }

    // Expand relative paths for internal use, to make sure everything
    // still works if we change our working directory.
    if(!repoDir.has_root_path())
      repoDir = absolute(repoDir, exeDir);

    get.setBase(repoDir);
    conf.load(repoDir.string());

    // Notify the config that the user has set directories
    if(hasAsked)
      conf.setAskedDirs();

    // Be backwards compatible with old repositories
    if(upgrading && is_directory(repoDir / "data"))
      conf.setGameDir("data");

    // If an existing user has not yet been asked about directories,
    // give them a chance to move.
    /*
    if(!hasAsked && !conf.has_asked_dirs)
      userMoveDirs();
    */
  }
};

#endif
