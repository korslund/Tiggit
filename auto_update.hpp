#ifndef _AUTO_UPDATE_HPP
#define _AUTO_UPDATE_HPP

#include <boost/algorithm/string.hpp>
#include "data_reader.hpp"
#include "progress_holder.hpp"
#include <fstream>
#include <time.h>

struct UpdateLog
{
  std::ofstream logfile;

  UpdateLog()
  {
    logfile.open("update_log.txt", std::ios::app);
  }

  void log(const std::string &msg)
  {
    using namespace std;
    char buf[100];
    time_t now = time(NULL);
    strftime(buf, 100, "%Y-%m-%d %X", gmtime(&now));
    string log = buf;
    log += ":   " + msg;
    logfile << log << endl;
  }

  void copyLog(const boost::filesystem::path &from,
               const boost::filesystem::path &to)
  {
    log("Copy: " + from.string() + " => " + to.string());
    boost::filesystem::copy_file(from, to);
  }
};

struct Updater : ProgressHolder
{
  // Current program version.
  std::string version;

  FileGetter gett;

  Updater(wxApp *_app)
    : ProgressHolder(_app), version("unknown")
  {}

  // Check if a given version is current. Returns true if no update is
  // needed.
  bool checkVersion(const std::string &url,
                    DataList::TigInfo &ti,
                    const std::string &ver)
  {
    setMsg("Checking for updates");
    wxBusyCursor busy;

    try
      {
        // Fetch the latest client information
        std::string tig = gett.getFile(url);

        if(!TigListReader::decodeTigFile(tig, ti))
          return true;
      }
    // Ignore errors and just keep existing version
    catch(...) { return true; }

    // Do we have the latest version?
    if(ver == ti.version)
      return true;

    return false;
  }

  /*
    This funtion performs all our auto-update steps, transparently to
    the rest of the application.

    If it returns true, you should immediately exit the application.
  */
  bool doAutoUpdate(const boost::filesystem::path &this_exe, bool isUpdater,
                    boost::filesystem::path new_exe)
  {
    using namespace Json;
    using namespace boost::filesystem;
    using namespace std;

    // This part is only done on Windows
    if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
      return false;

    // Is there an override file in our current dir?
    if(exists(path(this_exe).parent_path() / "override"))
      // If so, skip auto update and just run the current version.
      return false;

    UpdateLog log;

    path this_path = this_exe.parent_path();
    gett.setBase(this_path);

    log.log("this_path=" + this_path.string());

    // Detect old updater style (to manage the transition
    // alpha_039->alpha_040)
    if(!isUpdater && this_exe.leaf() == "update.exe")
      {
        new_exe = this_path / "tiggit.exe";
        isUpdater = true;

        log.log("Detected old version. Setting isUpdater manually.");
      }

    if(isUpdater)
      {
        /*
          isUpdater is set when we are called with the -u command line
          parameter.

          This means the update has already been downloaded and
          unpacked. In fact WE, the currently running exe, is the
          latest version.

          Our only job is to copy ourself (with or without DLL files,
          as needed) into the location given in new_exe, run that
          version, then exit.
         */

        log.log("Updating to new_exe=" + new_exe.string());

        setMsg("Installing update...");

        // Wait a second to make sure the old program has had time to
        // exit, since we are going to delete the file.
        wxSleep(1);

        path new_path = new_exe.parent_path();

        // Is the directory the same?
        if(new_path == this_path)
          {
            // Yes. Just copy the exe.
            if(exists(new_exe))
              remove(new_exe);

            log.copyLog(this_exe, new_exe);
          }
        else
          {
            // Nope. That means we have an entire directory to update.

            // Copy files over
            directory_iterator iter(this_path), end;
            for(; iter != end; ++iter)
              {
                path p = iter->path();

                // Only process files
                if(!is_regular_file(p)) continue;

                // Destination
                path dest = new_path / p.leaf();

                // Remove destination, if it exists
                if(exists(dest))
                  remove(dest);

                // Copy the file
                log.copyLog(p, dest);
              }
          }

        // In any case, run the new exe and exit the current program.
        log.log("Running " + new_exe.string());
        wxExecute(wxString(new_exe.c_str(), wxConvUTF8));
        return true;
      }

    // Temporary exe used for updates
    string updater_exe = (this_path/"update.exe").string();

    // Update destination.
    string up_dest = (this_path/"update").string();

    // Kill update remains if there are any
    bool didClean = false;
    if(exists(up_dest) || exists(updater_exe))
      {
        setMsg("Cleaning up...");

        log.log("Cleaning up " + updater_exe + " and " + up_dest + "/");

        // Wait a sec to give the program a shot to exit
        wxSleep(1);

        // Kill update/
        if(exists(up_dest))
          remove_all(up_dest);

        // Ditto for updater_exe
        if(exists(updater_exe))
          remove(updater_exe);

        didClean = true;
      }

    // At this point, we know we are running from our correct install
    // path. Our job now is to check if we are running the latest
    // version, and to upgrade if we are not.

    // Get current version
    {
      ifstream inf((this_path / "version").string().c_str());
      if(inf)
        inf >> version;
    }

    // If we just did an upgrade round, no point in doing it
    // again. Just exit.
    if(didClean)
      return false;

    // Fetch the latest client information
    DataList::TigInfo ti;
    {
      string lurl = "http://tiggit.net/client/latest.tig";

      // Hack to allow testing the updater without making it
      // public. We will improve this later.
      if(exists(this_path/"use_test_url"))
        lurl = "http://tiggit.net/client/latest_test.tig";

      log.log("Fetching " + lurl);
      if(checkVersion(lurl, ti, version))
        {
          // Current version is current, nothing more to do.
          log.log("Version " + version + " up-to-date");
          return false;
        }
    }

    log.log("Upgrading version " + version + " => " + ti.version);

    string vermsg = "Downloading latest update, please wait...\n"
      + version + " -> " + ti.version;

    if(!doUpdate(ti.url, up_dest, vermsg))
      return false;

    // Check if there are any new dll files as well
    string dll_version;
    {
      // Current dll version
      ifstream inf((this_path / "dll_version").string().c_str());
      if(inf)
        inf >> dll_version;
    }

    bool newDlls = false;
    if(!checkVersion("http://tiggit.net/client/dlls.tig", ti, dll_version))
      {
        log.log("Upgrading dll-pack version " + dll_version + " => " + ti.version);

        // Get the DLL files as well
        if(!doUpdate(ti.url, up_dest, vermsg))
          return false;
        newDlls = true;
      }

    /* Figure out what to run at this point. If we got new dlls, we
       have to run tiggit.exe in the update/ dir, because it will
       depend on the updated DLL files.

       If there are no new dlls however, we have to move it to
       this_path/update.exe, because it depends on the OLD dll files.
    */
    string run = (up_dest / "tiggit.exe").string();
    if(!newDlls)
      {
        log.copyLog(run, updater_exe);
        log.copyLog(up_dest/"version", this_path/"version");
        run = updater_exe;
      }

    // On unix later:
    //wxShell(("chmod a+x " + run).c_str());

    // Add command line parameter
    run += " --update=\"" + this_exe.string() + "\"";

    // Run the new exe, and let it figure out the rest
    log.log("Running " + run);
    int res = wxExecute(wxString(run.c_str(), wxConvUTF8));
    if(res == -1)
      return false;

    // If things went as planned, exit so the new version can do its thang.
    return true;
  }

  // Unpack a zip file from url into up_dest, while updating the given
  // progress dialog. Returns true on success.
  bool doUpdate(const std::string &url,
                const std::string &up_dest,
                const std::string &vermsg)
  {
    setMsg(vermsg + "\n" + url);

    // Start downloading the latest version
    std::string zip = gett.getPath("update.zip");
    DownloadJob getter(url, zip);
    getter.run();

    // Poll-loop until it's done
    while(true)
      {
        yield();
        wxMilliSleep(40);

        bool res;
        if(getter.total != 0)
          {
            // Calculate progress
            int prog = (int)(getter.current*100.0/getter.total);
            // Avoid auto-closing the window
            if(prog >= 100) prog=99;
            res = update(prog);
          }
        else
          res = pulse();

        // Did the user click 'Cancel'?
        if(!res)
          // Abort download thread
          getter.abort();

        // Did we finish, one way or another?
        if(getter.isFinished())
          break;
      }

    // If something went wrong, just forget about it and continue
    // running the old version instead.
    if(getter.isNonSuccess())
      return false;

    setMsg(vermsg + "\nUnpacking...");

    // Download complete! Start unpacking
    ZipJob install(zip, up_dest);
    install.run();

    // Do another semi-busy loop
    while(true)
      {
        yield();
        wxMilliSleep(40);

        // Disabled this because it looks like crap on windows. On
        // linux/gtk it works exactly like it should though.
        //pulse();

        // Exit when done
        if(install.isFinished())
          break;
      }

    // Give up if there were errors
    if(install.isNonSuccess())
      return false;

    return true;
  }
};
#endif
