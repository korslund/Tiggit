#ifndef _THREAD_GET_HPP
#define _THREAD_GET_HPP

#include "curl_get.hpp"
#include <wx/wx.h>

/* Download a file in a separate thread.
 */

struct ThreadGet;

struct MyThread : wxThread
{
  std::string url, file;
  ThreadGet *data;

  MyThread(const std::string &_url, const std::string &_file, ThreadGet *tg)
    : url(_url), file(_file), data(tg) {}

  ExitCode Entry();

  static int progress(void *data,
                      double dl_total,
                      double dl_now,
                      double up_total,
                      double up_now);
};

struct ThreadGet
{
  // Current status
  int current, total;

  /*
    0 - no download
    1 - in progress
    2 - done
    3 - error
    4 - (set externally) user requested abort
   */
  int status;

  // Main entrance function
  void start(const std::string &url, const std::string &file)
  {
    status = current = total = 0;

    MyThread *thr = new MyThread(url, file, this);
    thr->Create();
    thr->Run();
  }

};

wxThread::ExitCode MyThread::Entry()
{
  CurlGet get;
  get.get(url, file, &progress, data);
}

int MyThread::progress(void *data,
                       double dl_total,
                       double dl_now,
                       double up_total,
                       double up_now)
{
  ThreadGet &g = *((ThreadGet*)data);

  g.current = (int)dl_now;
  g.total = (int)dl_total;

  if(g.current == g.total && g.total != 0)
    g.status = 2;

  else if(g.status == 4)
    // This will abort the download
    return 1;

  else g.status = 1;

  return 0;
}

#endif
