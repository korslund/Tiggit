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
    string finalExeDir;

    path repoDir;
    bool write = false; // If true, write paths.json
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
              {
                repoDir = "../";
                finalExeDir = "bin";
              }
          }
      }

    // Our repoDir may still be empty at this point
    if(repoDir.empty())
      {
        /* If we get here, it means nothing was found. We assume this
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
        finalExeDir = "something";

        // Always write result after asking the user
        write = true;
      }

    // We must have set a repository directory at this point, or failed.
    assert(!repoDir.empty());

    // Write repo path
    if(write)
      writePaths(exeDir.string(), repoDir.string());

    // Expand relative path for internal use, to make sure everything
    // still works if we change our working directory.
    if(!repoDir.has_root_path())
      repoDir = absolute(repoDir, exeDir);

    get.setBase(repoDir);
    conf.load(repoDir.string());

    // If there is no existing exedir configured, and we have a
    // suggestion for a new one, then use that.
    if(conf.exedir == "")
      {
        if(finalExeDir != "")
          conf.setExeDir(finalExeDir);
        else
          // If there is no configured exe path, no natural
          // suggestions and no user-supplied path, use existing exe
          // path.
          conf.setExeDir(exeDir.string());
      }

    // Expand relative exe dirs
    exeDir = conf.exedir;
    if(!exeDir.has_root_path())
      exeDir = absolute(exeDir, repoDir);
    conf.fullexedir = exeDir.string();

    // Be backwards compatible with old repositories
    if(upgrading && is_directory(repoDir / "data"))
      conf.setGameDir("data");
  }

  // Writes dest_dir / "paths.json"
  static void writePaths(const std::string &dest_dir,
                         const std::string &value)
  {
    Json::Value root;
    boost::filesystem::path pathfile = dest_dir;
    pathfile /= "paths.json";
    root["repo_dir"] = value;
    writeJson(pathfile.string(), root);
  }
};

#endif
