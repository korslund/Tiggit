#ifndef __GAMEINFO_HPP_
#define __GAMEINFO_HPP_

#include "datalist.hpp"

#include <wx/string.h>
#include "filegetter.hpp"
#include "downloadjob.hpp"
#include "zipjob.hpp"
#include "jobqueue.hpp"
#include "screenshot_loader.hpp"

#include <time.h>
#include <assert.h>

/* GameInfo is the active manager of game information. It links to but
   is separate from DataList::Entry, which holds the actual game data
   from the tigfile and data list.

   GameInfo represents the application specific and dynamic
   functionality related to the data, while the Entry just represents
   the static data itself.
 */
struct GameInfo
{
  DataList::Entry &entry;

  /* Cached wxString versions of often used strings. TimeString is a
     string representation of entry.add_time.
  */
  wxString urlname, idname, name, tigurl, timeString;

  bool isInstalled() const { return status == 2; }
  bool isWorking() const { return status == 1 || status == 3; }
  bool isUnpacking() const { return status == 3; }
  bool isDownloading() const { return status == 1; }
  bool isNone() const { return status == 0; }

  // Start downloading this game
  void startDownload()
  {
    assert(!job);
    assert(isNone());

    // Construct the destination file name
    boost::filesystem::path dir = "incoming";
    dir /= entry.idname + ".zip";
    std::string out = get.getPath(dir.string());

    // Start the download job
    job = new DownloadJob(entry.tigInfo.url, out);
    job->run();

    // Set status
    setDownloading();
  }

  /* Called regularly to update status. Returns true if the game
     changed to/from non-installed status.

     May also throw exceptions. These should also be counted as status
     changes (same as when returning 'true'.)
  */
  bool updateStatus()
  {
    // Are we doing anything at the moment?
    if(!isWorking())
      return false;

    assert(job);

    bool statusChange = false;
    bool error = false;
    StatusJob *newJob = NULL;
    std::string errMsg = "";

    if(job->isFinished())
      {
        if(job->isSuccess())
          {
            if(isDownloading())
              {
                // Download succeeded. Start unpacking.
                DownloadJob *g = dynamic_cast<DownloadJob*>(job);
                assert(g);

                // Get install path
                boost::filesystem::path dir = "data";
                dir /= entry.idname;

                // Set up installer
                newJob = new ZipJob(g->file, get.getPath(dir.string()));
                jobQueue.queue(newJob);
                setUnpacking();

                setStatus(wxT("Unpacking..."));
              }
            else
              {
                // The game has finished installing, and is now ready
                // for use.
                assert(isUnpacking());
                setInstalled();
                statusChange = true;
              }
          }
        else
          {
            // The job did not finish
            statusChange = true;
            setUninstalled();

            // Did it fail?
            if(job->isError())
              {
                // Set errMsg depending on job type
                error = true;
                if(isDownloading())
                  errMsg = "Downloading failed for " + entry.name + ":\n" +
                    job->getError();
                else
                  errMsg = "Unpacking failed for " + entry.name + ":\n" +
                    job->getError();
              }
          }

        // Since the job is done, delete the object.
        jobQueue.unlink(job);
        delete job;
        job = newJob;
      }
    else
      {
        // Our job is still running. Update download stats.
        if(isDownloading())
          {
            DownloadJob *g = dynamic_cast<DownloadJob*>(job);
            assert(g);

            int percent = 0;
            if(g->total > 0)
              percent = (int)((100.0*g->current)/g->total);

            // Update the visible progress message.
            wxString status;
            status << percent << wxT("% (");
            status << sizify(g->current) << wxT(" / ");
            status << sizify(g->total) << wxT(")");

            setStatus(status);
          }
      }

    if(error)
      throw std::runtime_error(errMsg);

    return statusChange;
  }

  /* Request the screenshot of this game. The parameter callback will
     be invoked once the screenshot is ready.

     The callback might be invoked immediately (during this call), or
     it might be invoked later (eg. if it needs to be downloaded.) And
     if the screenshot is missing, the callback will not be invoked at
     all.

     However, if it IS invoked, it is always invoked in the main
     thread, possibly through jobQueue.update().

     It's OK for cb to be NULL. In that case this function simply
     preloads the image for later use.
   */
  void requestShot(ScreenshotCallback *cb = NULL)
  {
    // Use exact sized image if it exists
    std::string url = entry.tigInfo.shot300x260;

    // If not, try the general screenshot
    if(url == "") url = entry.tigInfo.shot;

    // Exit if there's no valid screenshot to use
    if(url == "")
      return;

    // Figure out the cache filename
    std::string filename = get.getPath("cache/shot300x260/" + entry.idname);

    // Let the screenshot loader handle all the details internally
    screen.handleRequest(entry.idname, filename, url, cb);
  }

  // Abort current job
  void abortJob()
  {
    assert(job);
    assert(isWorking());

    // This is enough. House keeping actions elsewhere will take care
    // of changing our status.
    job->abort();
  }

  const wxString &getStatus() const { return msg; }

  // Since we haven't fully moved all functionality to this class yet,
  // we do allow some external state manipulation, although this
  // should be phased out in the long run.
  void setInstalled() { status = 2; }
  void setUninstalled() { status = 0; }

  GameInfo(DataList::Entry &e)
    : entry(e), job(NULL), status(0)
  {
    e.info = this;

    urlname = wxString(e.urlname.c_str(), wxConvUTF8);
    idname = wxString(e.idname.c_str(), wxConvUTF8);
    name = wxString(e.name.c_str(), wxConvUTF8);
    tigurl = wxString(e.tigurl.c_str(), wxConvUTF8);

    time_t t = e.add_time;
    char buf[50];
    strftime(buf,50, "%Y-%m-%d", gmtime(&t));
    timeString = wxString(buf, wxConvUTF8);
  }

  static GameInfo& conv(DataList::Entry &e)
  {
    if(e.info == NULL)
      e.info = new GameInfo(e);

    return *((GameInfo*)e.info);
  }

private:
  /* Status
     0 - not installed
     1 - downloading
     2 - ready to play
     3 - unpacking
  */
  int status;

  void setDownloading() { status = 1; }
  void setUnpacking() { status = 3; }
  void setStatus(const wxString &m) { msg = m; }

  // Current download / install job, if any
  StatusJob *job;

  // Status message displayed when isWorking() == true
  wxString msg;

  // Screenshot fetcher and holder
  ScreenshotLoader screen;

  // Create a nice size string
  static wxString sizify(int size)
  {
    wxString tmp;
    if(size > 1024*1024)
      tmp << wxString::Format(wxT("%.1f"), size/(1024.0*1024)) << wxT("Mb");
    else if(size > 1024)
      tmp << wxString::Format(wxT("%.1f"), size/1024.0) << wxT("Kb");
    else
      tmp << size;

    return tmp;
  }
};

#endif
