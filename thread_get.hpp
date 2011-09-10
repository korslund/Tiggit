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
    : url(_url), file(_file), data(tg)
  {
    Create();
    Run();
  }

  ExitCode Entry();

  static int progress(void *data,
                      double dl_total,
                      double dl_now,
                      double up_total,
                      double up_now);
};

struct ThreadGet
{
  std::string url, file;

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
  void start(const std::string &_url, const std::string &_file)
  {
    url = _url;
    file = _file;

    status = current = total = 0;

    new MyThread(url, file, this);
  }

};

wxThread::ExitCode MyThread::Entry()
{
  CurlGet get;
  try
    {
      get.get(url, file, &progress, data);
    }
  catch(...)
    {
      // TODO: Log error somewhere
      data->status = 3;
    }

  if(data->status >= 3)
    boost::filesystem::remove(file);
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
