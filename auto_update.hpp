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

    // Our own exe path
    string this_exe = string(wxStandardPaths::Get().GetExecutablePath().mb_str());
    // Canonical path
    path canon_path = get.getPath("bin");
    string canon_exe = (canon_path/"tiggit.exe").string();
    if(!boost::iequals(this_exe, canon_exe))
        {
            string str = this_exe + " " + canon_exe;
            wxString msg(str.c_str(), wxConvUTF8);
            wxMessageBox(msg, wxT("Dirs"), wxOK);
            return false;
        }

    // Kill the update/ folder if there is one
    string dest = get.getPath("update/");
    if(exists(dest))
        remove_all(dest);

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

    // Set up the progress dialog
    wxProgressDialog *dlg =
      new wxProgressDialog(wxT("Updating Tiggit"),
                           wxT("Downloading latest update, please wait..."),
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
    void *handle = inst.queue(zip, dest);

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
    dlg->Update(100);
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
