#ifndef _DOWNLOAD_JOB_HPP
#define _DOWNLOAD_JOB_HPP

#include "curl_get.hpp"
#include "jobify.hpp"
#include <boost/filesystem.hpp>

/* Download a file in a separate thread.
 */
struct DownloadJob : ThreadJob
{
  std::string url, file;

  // Current progress
  int current, total;

  /* True if the last run of progress() reported a full download count
     (meaning current = total amount of bytes.)

     This in itself is not enough to mark the entire download as
     finished though, since what we 'completed' may only be a redirect
     page, not the file itself. Thus this is only used as an
     indicator.
   */
  bool completed;

  // Main entrance function
  DownloadJob(const std::string &_url, const std::string &_file)
    : url(_url), file(_file), current(0), total(0), completed(false)
  {}

private:
  void executeJob()
  {
    // We need local variables since in principle, the object may be
    // deleted after the finish status functions (setDone() etc) are
    // called.
    std::string fname = file;
    bool success = false;

    try
      {
        CurlGet::get(url, file, &curl_progress, this);

        if(abortRequested()) setAbort();
        else
          {
            setDone();
            success = true;
          }
      }
    catch(std::exception &e)
      {
        setError(e.what());
      }

    if(!success)
      boost::filesystem::remove(fname);
  }

  // Static progress function passed to CURL
  static int curl_progress(void *data,
                           double dl_total,
                           double dl_now,
                           double up_total,
                           double up_now)
  {
    DownloadJob *g = (DownloadJob*)data;
    return g->progress((int)dl_total, (int)dl_now);
  }

  int progress(int dl_total, int dl_now)
  {
    current = dl_now;
    total = dl_total;

    // Did the user request an abort?
    if(abortRequested())
      // This will make CURL abort the download
      return 1;

    return 0;
  }
};

#endif
