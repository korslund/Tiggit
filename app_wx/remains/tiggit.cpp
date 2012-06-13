#define wxUSE_UNICODE 1

#include <wx/wx.h>
#include <wx/listctrl.h>
//#include <wx/hyperlink.h>
#include <wx/imaglist.h>
#include <wx/stdpaths.h>

#include <iostream>
#include <assert.h>
#include <time.h>

#include "image_viewer.hpp"
#include "decodeurl.hpp"
#include "repo.hpp"
#include "data_reader.hpp"
#include "downloadjob.hpp"
#include "zipjob.hpp"
#include "jobqueue.hpp"
#include "json_installed.hpp"
#include "auto_update.hpp"
#include "listkeeper.hpp"
#include "auth.hpp"
#include "json_rated.hpp"
#include "gameinfo.hpp"
#include "cache_fetcher.hpp"
#include "tag_sorter.hpp"
#include "tabbase.hpp"
#include "newstab.hpp"
#include "adpicker.hpp"
#include "wx/dialogs.hpp"

using namespace std;

DataList data;
JsonInstalled jinst;
TigListReader tig_reader;
TagSorter tagSorter;

// Ask the user an OK/Cancel question.
bool ask(const wxString &question)
{
  return wxMessageBox(question, wxT("Please confirm"),
                      wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
}

// Display an error message box
void errorBox(const wxString &msg)
{
  wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
}

// Update data from all_games.json. If the parameter is true, force
// download. If not, download only if the existing file is too old.
void updateData(bool download)
{
  // This is bad and kinda leaks memory like hell (because of all the
  // GameInfo instances.) But refreshes are kinda not an integral part
  // of the application at the moment, so just ignore it for now.
  data.arr.resize(0);

  string lstfile = get.getPath("all_games.json");

  // Do we check file age?
  if(!download && boost::filesystem::exists(lstfile))
    {
      // Yup, check file age
      time_t ft = boost::filesystem::last_write_time(lstfile);
      time_t now = time(0);

      // update game list once an hour
      if(difftime(now,ft) > 60*60*1)
        download = true;
    }
  else
    // If the file didn't exist, download it.
    download = true;

  if(download && !conf.offline)
    {
      try
        {
          // Get the latest list from the net
          string url = "http://tiggit.net/api/all_games.json";
          get.getTo(url, "all_games.json");
        }
      catch(std::exception &e)
        {
          // Warn the user that download failed
          wxString msg(e.what(), wxConvUTF8);
          errorBox(msg + wxT("\nAttempting to proceed anyway..."));
        }
    }

  try
    {
      tig_reader.loadData(lstfile, data);
      jinst.read(data);
      tagSorter.process(data);
    }
  catch(std::exception &e)
    {
      // Did we already try downloading, or are we running in offline
      // mode?
      if(download || conf.offline)
        {
          // Then fail, nothing more to do
          wxString msg(e.what(), wxConvUTF8);

          // Make sure we delete the file - no point in keeping an
          // invalid cache.
          boost::filesystem::remove(lstfile);

          errorBox(msg);
        }
      else
        // Nope. Try again, this time force a download.
        updateData(true);
    }
}
