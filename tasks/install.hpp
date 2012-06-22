#ifndef __TASKS_INSTALL_HPP_
#define __TASKS_INSTALL_HPP_

#include "multi.hpp"

/* The InstallTask is a convenience wrapper around MultiTask,
   DownloadTask and UnpackTask. Downloads a file from _url, stores it
   in _zip, then unpacks it into _dir. Despite the variable name, the
   archive type may be any type supported by unpack/, not just zip
   files.

   To support special cases under a unified interface, the class also
   supports the case where _dir = "". This will just download the
   archive file and exit.

   The _zip file is deleted on completion, except when _dir=="". The
   _dir directory is NOT removed on error, because we have no way of
   knowing if we are the sole user of the directory. It's up to the
   user to clean it up on error.

   Like with MultiTask, it's highly recommended that the given
   JobInfoPtr (if any) points to a struct created with
   MultiTask::makeInfo(). If not, neither aborting nor progress
   reports will work properly.
 */

namespace Tasks
{
  struct InstallTask : MultiTask
  {
    InstallTask(const std::string &_url, const std::string &_zip,
                const std::string &_dir,
                Jobify::JobInfoPtr _info = Jobify::JobInfoPtr())
      : MultiTask(_info), url(_url), zip(_zip), dir(_dir) {}

    void doJob();
    void cleanup();

  private:
    std::string url, zip, dir;
  };
}
#endif
