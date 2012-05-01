#ifndef __REPO_HPP_
#define __REPO_HPP_

// TODO: Will incorporate filegetter into this file soon
#include "filegetter.hpp"
#include "readjson.hpp"
#include <boost/filesystem.hpp>
#include <wx/stdpaths.h>
#include <string>
#include <stdexcept>

struct Repository
{
  // TODO: Make non-static, kill all global variables, and pass along
  // paths.
  static void setupPaths()
  {
    using namespace boost::filesystem;
    using namespace Json;
    using namespace std;

    path exe = string(wxStandardPaths::Get().GetExecutablePath().mb_str());
    path exeDir = exe.parent_path();
    path conf = exeDir / "paths.json";

    path repoDir;
    bool write = false;

    // If the paths file exists, use it
    if(exists(conf))
      {
        try
          {
            Value root = readJson(conf.string());
            repoDir = root["repo_dir"].asString();
          }
        catch(...) {}
      }
    else
      {
        // Assume missing dir means we are converting from the old system
        path appdata = string(wxStandardPaths::Get().GetUserLocalDataDir().mb_str());

        // If there is a 'config' file there, then we've found our
        // repository
        if(exists(appdata / "config"))
          {
            repoDir = appdata;

            write = true;

            /* At this point we know we are upgrading an old
               installation (since we found config, but didn't have
               paths.json).

               So while we're here, give the user a chance to move to a
               new location.
            */

            // TODO: Call the EXACT same function here as you would
            // call for the menu -> Change Directories options.

            /* Also, the exe location will be set as an instance
               variable here, and used to install to that
               location. Figure this out.
             */

            cout << "Using old repository at " << repoDir << endl;

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

           TODO: If the installer and games are installed at the same
           place, always use a portable setup. Or 'portable' could be a
           separate option (install games and launcher in the same
           place).

           Also: should also work if the user selects an existing
           repo. We don't care about that at all at this point. The
           rest of the system takes care of using what's actually in
           the dirs.
        */
        throw std::runtime_error("Cannot find repository. Installation not implemented yet.");
        // Always write result after asking the user
        write = true;
      }

    // We must have set a repository directory at this point, or failed.
    assert(!repoDir.empty());

    if(write)
      {
        Value root;
        root["repo_dir"] = repoDir.string();
        writeJson(conf.string(), root);
      }

    // Expand relative paths for internal use, to make sure everything
    // still works if we change our working directory.
    if(!repoDir.has_root_path())
      repoDir = absolute(repoDir, exeDir);

    get.setBase(repoDir);
  }
};

#endif
