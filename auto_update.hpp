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

  Updater() : version("unknown") {}

  /*
    This funtion performs all our auto-update steps, transparently to
    the rest of the application.

    If it returns true, you should immediately exit the application.
  */
  bool doAutoUpdate(wxApp *app)
  {
    using namespace Json;
    using namespace boost::filesystem;
    using namespace std;

    // Unless we're on windows, there's not much more to do right now.
    if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
      return false;

    // Update destination
    string up_dest = get.getPath("update/");

    // Our own exe path
    wxString this_wx = wxStandardPaths::Get().GetExecutablePath();
    string this_exe = string(this_wx.mb_str());
    // Canonical path
    path canon_path = get.getPath("bin");
    string canon_exe = (canon_path/"tiggit.exe").string();

    wxMessageBox(this_wx, wxT("New version"), wxOK);

    if(!boost::iequals(this_exe, canon_exe))
      {
        // Check if there is a download update available
        if(exists(up_dest))
          {
            // Yup. Most likely we are running update/tiggit.exe right
            // now. In any case, since the bin/ version is not
            // running, we can overwrite it.

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
        // Kill the update/ folder if there is one
        if(exists(up_dest))
          {
            // Wait a sec to give the program a shot to exit
            wxSleep(1);
            remove_all(up_dest);
          }
      }

    // Get current version
    {
      ifstream inf(get.getPath("bin/version").c_str());
      if(inf)
        inf >> version;
    }

    // Fetch the latest client information
    string tig = get.getTo("http://tiggit.net/client/latest.tig", "latest.tig");

    DataList::TigInfo ti;
    if(!TigListReader::decodeTigFile(tig, ti))
      return false;

    // Do we have the latest version?
    if(version == ti.version)
      // Yup. Nothing more to do.
      return false;

    string vermsg = "Downloading latest update, please wait...\n"
      + version + " -> " + ti.version;
    wxString xvermsg = wxString(vermsg.c_str(), wxConvUTF8);

    // Set up the progress dialog
    wxProgressDialog *dlg =
      new wxProgressDialog(wxT("Updating Tiggit"), xvermsg,
                           //wxT("Downloading latest update, please wait..."),
                           100, NULL, wxPD_APP_MODAL|wxPD_CAN_ABORT|wxPD_AUTO_HIDE);
    dlg->Show(1);

    // Start downloading the latest version
    ThreadGet getter;
    string zip = get.getPath("update.zip");
    getter.start(ti.url, zip);

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
          getter.status = 4;

        // Did we finish, one way or another?
        if(getter.status > 1)
          break;
      }

    // If something went wrong, just forget about it and continue
    // running the old version instead.
    if(getter.status > 2)
      {
        dlg->Destroy();
        return false;
      }

    // Download complete! Start unpacking
    void *handle = inst.queue(zip, up_dest);

    // Do another semi-busy loop
    int status;
    while(true)
      {
        app->Yield();
        wxMilliSleep(40);

        // Ignore 'cancel' commands while unpacking.
        dlg->Pulse();

        status = inst.check(handle);

        // Are we done?
        if(status >= 2)
          break;
      }

    // Shut down the window
    dlg->Update(100); // Trigger auto-hide
    dlg->Destroy();

    // Give up if there were errors
    if(status >= 3)
      return false;

    // On unix only
    //wxShell(("chmod a+x " + run).c_str());

    // Run the new exe, and let it figure out the rest
    string run = get.getPath("update/tiggit.exe");
    int res = wxExecute(wxString(run.c_str(), wxConvUTF8));
    if(res == -1)
      return false;

    // If things went as planned, exit so the new version can do its thang.
    return true;
  }
};
#endif
