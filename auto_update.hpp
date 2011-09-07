#ifndef _AUTO_UPDATE_HPP
#define _AUTO_UPDATE_HPP

#include <wx/wx.h>
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
  bool doAutoUpdate()
  {
    using namespace Json;
    using namespace boost::filesystem;
    using namespace std;

    string f_updated = get.getPath("updated");
    string f_version = get.getPath("version");
    string f_bin = get.getPath("bin/tiggit.exe");
    string f_newbin = get.getPath("bin/new.exe");

    // First, check if this is an update run
    if(exists(f_updated))
      {
        // Get new version
        {
          ifstream inf(f_updated.c_str());
          if(inf)
            inf >> version;
        }
        remove(f_updated);

        // Copy ourselves in place
        remove(f_bin);
        copy_file(f_newbin, f_bin);

        // Store new version number
        ofstream of(f_version.c_str());
        of << version;

        // Continue running.
        return false;
      }

    // Get current version
    version = "none";
    {
      ifstream inf(f_version.c_str());
      if(inf)
        inf >> version;
    }

    // Updates are only available for windows at the moment.
    if((wxGetOsVersion() & wxOS_WINDOWS) == 0)
      return false;

    // Fetch the latest client information
    string tig = get.getTo("http://tiggit.net/client/latest.tig", "latest.tig");

    DataList::TigInfo ti;
    if(!TigListReader::decodeTigFile(tig, ti))
      return false;

    // Do we have the latest version?
    if(version == ti.version)
      // Yup. Nothing more to do.
      return false;

    // Download the latest version
    string bin = get.getTo(ti.url, "bin/new.exe");

    // Tell it that we're upgrading
    ofstream of(f_updated.c_str());
    of << ti.version;

    // On unix only
    //system(("chmod a+x " + bin).c_str());

    // Launch the new binary
    int res = wxExecute(wxString(bin.c_str(), wxConvUTF8));
    if(res == -1)
      return false;

    return true;
  }
};
#endif
