#ifndef __TASKS_UNPACK_HPP_
#define __TASKS_UNPACK_HPP_

#include <job/job.hpp>
#include <mangle/vfs/stream_factory.hpp>

/*
  This class takes any compressed archive file and unpacks it into the
  given stream factory or file system directory.

  The file may be in any known archive format, and the format is
  auto-detected. The backend code is found in the unpack/ module.
 */

namespace Tasks
{
  struct UnpackTask : Jobify::Job
  {
    UnpackTask(const std::string &_file, const std::string &_dir,
               Jobify::JobInfoPtr info)
      : Jobify::Job(info), file(_file), dir(_dir) {}

    UnpackTask(const std::string &_file, Mangle::VFS::StreamFactoryPtr _writeTo,
                 Jobify::JobInfoPtr info)
      : Jobify::Job(info), file(_file), writeTo(_writeTo) {}

  private:
    void doJob();

    std::string file, dir;
    Mangle::VFS::StreamFactoryPtr writeTo;
  };
}

#endif
