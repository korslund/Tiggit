#ifndef __TASKS_DOWNLOAD_HPP_
#define __TASKS_DOWNLOAD_HPP_

#include <job/job.hpp>
#include <mangle/stream/stream.hpp>

namespace Tasks
{
  /* Download a file. You can download either to a filename or to a
     Mangle::Stream.

     You can also download to nowhere (ignore the data), by specifying
     either an empty StreamPtr or an empty filename. This can be
     useful to sending signals to a server via GET commands, for
     example.
   */

  struct DownloadTask : Jobify::Job
  {
    DownloadTask(const std::string &_url, const std::string &_file,
                 Jobify::JobInfoPtr info)
      : Jobify::Job(info), url(_url), file(_file) {}

    DownloadTask(const std::string &_url, Mangle::Stream::StreamPtr _stream,
                 Jobify::JobInfoPtr info)
      : Jobify::Job(info), url(_url), stream(_stream) {}

    static std::string userAgent;

  private:
    void doJob();

    std::string url, file;
    Mangle::Stream::StreamPtr stream;
  };
}

#endif
