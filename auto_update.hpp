#ifndef _AUTO_UPDATE_HPP
#define _AUTO_UPDATE_HPP

#include <wx/wx.h>
#include <wx/progdlg.h>
#include <boost/algorithm/string.hpp>
#include "data_reader.hpp"

struct Updater
{
  // Current program version.
  std::string version;
  wxApp *app;
  wxProgressDialog *dlg;

  Updater(wxApp *_app)
    : version("unknown"), app(_app), dlg(NULL)
  {}

  ~Updater()
  {
    // Shut down the window
    if(dlg)
      {
        dlg->Update(100);
        dlg->Destroy();
        app->Yield();
      }
  }

  void setMsg(const wxString &str, int value=0)
  {
    if(!dlg)
      {
        // Crank up the ol' progress bar. Give the text some space to grow.
        dlg = new wxProgressDialog(wxT("Information"), wxT("Tiggit is checking for updates, please wait...        \n\n "),
                                   100, NULL, wxPD_APP_MODAL|wxPD_CAN_ABORT|wxPD_AUTO_HIDE);
        dlg->Show(1);
      }

    dlg->Show(true);
    dlg->Update(value, str);
    app->Yield();
  }

  // Check if a given version is current. Returns true if no update is
  // needed.
  bool checkVersion(const std::string &url,
                    DataList::TigInfo &ti,
                    const std::string &ver)
  {
    // Fetch the latest client information
    std::string tig = get.getFile(url);

    if(!TigListReader::decodeTigFile(tig, ti))
      return true;

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
  bool doAutoUpdate()
  {
    using namespace Json;
    using namespace boost::filesystem;
    using namespace std;

    // Unless we're on windows, there's not much more to do right now.
    if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
      return false;

    // Our own exe path
    wxString this_wx = wxStandardPaths::Get().GetExecutablePath();
    string this_exe = string(this_wx.mb_str());

    // Is there an override file in our current dir?
    if(exists(path(this_exe).branch_path() / "override"))
      // If so, skip auto update and just run the current version.
      return false;

    // Canonical path
    path canon_path = get.getPath("bin");
    string canon_exe = (canon_path/"tiggit.exe").string();

    // Temporary exe used for updates
    string tmp_exe = (canon_path/"update.exe").string();

    // Update destination. (Don't use getPath, that creates the
    // directory!)
    string up_dest = (get.base / "update").string();

    // Are we running from the canonical path?
    if(!boost::iequals(this_exe, canon_exe))
      {
        // Check if there is a download update available
        if(exists(up_dest))
          {
            // Yup. Most likely we are running update/tiggit.exe right
            // now. In any case, since the bin/ version is not
            // running, we can overwrite it.

            setMsg(wxT("Installing update..."));

            // Wait a little while in case bin/ launched us, to give
            // it time to exit. (Not a terribly robust solution, I
            // know, fix it later.)
            wxSleep(1);

            // Copy files over
            directory_iterator iter(up_dest), end;
            for(; iter != end; ++iter)
              {
                path p = iter->path();

                // Only process files
                if(!is_regular_file(p)) continue;

                // Destination
                path dest = canon_path / p.leaf();

                // Remove destination, if it exists
                if(exists(dest))
                  remove(dest);

                // Copy the file
                copy_file(p, dest);
              }
          }

        // In any case, run the canonical path and exit the current
        // instance
        wxExecute(wxString(canon_exe.c_str(), wxConvUTF8));
        return true;
      }
    else
      {
        // We are running a correctly installed exe

        // TODO: At some later point, we will use this spot to check
        // for updates previously unpacked at runtime. This is less
        // disruptive than downloading updates at startup, as we are
        // currently doing below.

        // Kill update remains if there are any
        if(exists(up_dest) || exists(tmp_exe))
          {
            setMsg(wxT("Cleaning up..."));

            // Wait a sec to give the program a shot to exit
            wxSleep(1);

            // Kill update/
            if(exists(up_dest))
              remove_all(up_dest);

            // Ditto for tmp_exe
            if(exists(tmp_exe))
              remove(tmp_exe);
          }
      }

    // Get current version
    {
      ifstream inf(get.getPath("bin/version").c_str());
      if(inf)
        inf >> version;
    }

    // Fetch the latest client information
    DataList::TigInfo ti;
    if(checkVersion("http://tiggit.net/client/latest.tig", ti, version))
      // Current version is current, nothing more to do.
      return false;

    string vermsg = "Downloading latest update, please wait...\n"
      + version + " -> " + ti.version;

    bool ok = doUpdate(ti.url, up_dest, vermsg);

    if(!ok)
      return false;

    // Check if there are any new dll files as well
    string dll_version;
    {
      // Current dll version
      ifstream inf(get.getPath("bin/dll_version").c_str());
      if(inf)
        inf >> dll_version;
    }
    ok = false;
    if(!checkVersion("http://tiggit.net/client/dlls.tig", ti, dll_version))
      {
        // Get the DLL files as well
        ok = doUpdate(ti.url, up_dest, vermsg);
      }

    // Figure out what to run at this point
    string run = get.getPath("update/tiggit.exe");

    if(!ok)
      {
        // No new DLLs. We need to copy the new exe in with the old
        // DLLs to make sure it runs.
        copy_file(run, tmp_exe);
        run = tmp_exe;
      }

    // On unix only
    //wxShell(("chmod a+x " + run).c_str());

    // Run the new exe, and let it figure out the rest
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
    setMsg(wxString((vermsg + "\n" + url).c_str(), wxConvUTF8));

    // Start downloading the latest version
    std::string zip = get.getPath("update.zip");
    DownloadJob getter(url, zip);
    getter.run();

    // Poll-loop until it's done
    while(true)
      {
        app->Yield();
        wxMilliSleep(40);

        bool res;
        if(getter.total != 0)
          {
            // Calculate progress
            int prog = (int)(getter.current*100.0/getter.total);
            // Avoid auto-closing the window
            if(prog >= 100) prog=99;
            res = dlg->Update(prog);
          }
        else
          res = dlg->Pulse();

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

    dlg->Update(0, wxString((vermsg + "\nUnpacking...").c_str(), wxConvUTF8));

    // Download complete! Start unpacking
    ZipJob install(zip, up_dest);
    install.run();

    // Do another semi-busy loop
    while(true)
      {
        app->Yield();
        wxMilliSleep(40);

        // Disabled this because it looks like crap on windows. On
        // linux/gtk it works exactly like it should though.
        //dlg->Pulse();

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
