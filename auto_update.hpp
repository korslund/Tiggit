#ifndef _AUTO_UPDATE_HPP
#define _AUTO_UPDATE_HPP

#include <wx/wx.h>
#include <wx/progdlg.h>
#include "data_reader.hpp"

struct Updater
{
  // Current program version.
  std::string version;

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

    // Get current version
    version = "none";
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
                           100, NULL, wxPD_APP_MODAL|wxPD_CAN_ABORT);
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
    string dest = get.getPath("update/");
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
    dlg->Destroy();

    // Give up if there were errors
    if(status >= 3)
      return false;

    // Success! Launch the NEW install.exe, after copying it to a
    // better location first.
    string install = get.getPath("update/install.exe");
    string cmd = get.copyTo(install, "install.exe");

    // On unix only
    //wxShell(("chmod a+x " + cmd).c_str());

    // Add parameters
    cmd += " " + dest + " " + get.getPath("bin/") + " tiggit.exe";

    // Run it!
    int res = wxExecute(wxString(cmd.c_str(), wxConvUTF8));
    if(res == -1)
      return false;

    // If things went as planned, exit quickly so the installer can do
    // its thang.
    return true;
  }
};
#endif
