#ifndef _DOWNLOAD_JOB_HPP
#define _DOWNLOAD_JOB_HPP

#include "jobify.hpp"

/* Download a file in a separate thread.
 */
struct DownloadJob : ThreadJob
{
  std::string url, file;

  // Current progress
  int current, total;

  // Main entrance function
  DownloadJob(const std::string &_url, const std::string &_file)
    : url(_url), file(_file), current(0), total(0)
  {}

private:
  void executeJob();

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
