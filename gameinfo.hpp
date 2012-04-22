#ifndef __GAMEINFO_HPP_
#define __GAMEINFO_HPP_

#include "datalist.hpp"

#include <wx/string.h>
#include "filegetter.hpp"
#include "downloadjob.hpp"
#include "zipjob.hpp"
#include "jobqueue.hpp"
#include "screenshot_loader.hpp"
#include "json_rated.hpp"
#include "config.hpp"

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
  wxString urlname, idname, name, tigurl, timeString, rating, dlCount, price;

  bool isInstalled() const { return status == 2; }
  bool isWorking() const { return status == 1 || status == 3; }
  bool isUnpacking() const { return status == 3; }
  bool isDownloading() const { return status == 1; }
  bool isNone() const { return status == 0; }

  // Rate the game and send the rating off to the server
  void rateGame(int rating)
  {
    assert(rating >= 0 && rating <= 5);

    // No point in voting more than once, the server will filter it
    // out.
    if(myRating() != -1) return;

    // Send vote to server
    char str[2];
    str[0] = '0' + rating;
    str[1] = 0;

    DownloadJob *notify = new DownloadJob("http://tiggit.net/api/count/" + entry.urlname + "&rate=" + str, get.tmp->get("rate-" + entry.urlname));
    notify->runAndDelete();

    // Update and save local voting database
    ratings.setRating(entry.idname, rating);
  }

  // Return the rating we given this game, or -1 if we haven't yet
  // rated it.
  int myRating()
  {
    return ratings.getRating(entry.idname);
  }

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

    // Also give the server an anonymous update to statistics
    DownloadJob *notify = new DownloadJob("http://tiggit.net/api/count/" + entry.urlname + "&download", get.tmp->get("count-" + entry.urlname));
    notify->runAndDelete();

    // Set status
    setDownloading();
  }

  /* Called regularly to update status. Returns:

     0 - status has not changed
     1 - status has changed and the install list should be updated
     2 - status has changed, display lists and install list should be
         updated

     May also throw exceptions. These should also be counted as status
     changes (same as when returning 2.)
  */
  bool updateStatus()
  {
    // Are we doing anything at the moment?
    if(!isWorking())
      return false;

    assert(job);

    int exitStatus = 0;
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
                exitStatus = 1;
                assert(isUnpacking());
                setInstalled();
              }
          }
        else
          {
            // The job did not finish
            exitStatus = 2;
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

    return exitStatus;
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

    if(e.rateCount > 0 && e.rating > 0)
      {
        rating = wxString::Format(wxT("%3.2f"), e.rating);

        if(conf.debug)
          rating += wxString::Format(wxT(" (%d)"), e.rateCount);
      }

    dlCount << e.dlCount;
    price = wxString::Format(wxT("$%3.2f"), e.tigInfo.price);

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
