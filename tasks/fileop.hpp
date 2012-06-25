#ifndef __TASKS_FILEOP_HPP_
#define __TASKS_FILEOP_HPP_

#include <job/job.hpp>
#include <list>

namespace Tasks
{
  /* File operations task. Can be used to run large file operations
     (moving, copying or deleting files or directories) in a
     background thread.

     The class works by setting up a list of tasks, then running them
     with run().

     Error handling is not terribly advanced - any error will cause
     the task to fail, and abort any operations set to follow it.

     In move and delete operations, the source and destination paths
     may refer to either single files or directories, whatever makes
     sense.

     Copying is currently only implemented for files, not directories.
   */
  enum FileOpTaskOps { FILEOP_COPY, FILEOP_MOVE, FILEOP_DELETE };

  struct FileOpTask : Jobify::Job
  {
    struct FileOp
    {
      int type;
      std::string source, dest; // Use 'dest' for delete operations
    };

    FileOpTask(Jobify::JobInfoPtr info = Jobify::JobInfoPtr())
      : Jobify::Job(info) {}

    void addOp(const FileOp &op);
    void addOp(int type, const std::string &source, const std::string &dest);

    // Convenience functions for adding operations.
    void copy(const std::string &source, const std::string &dest)
    { addOp(FILEOP_COPY, source, dest); }
    void move(const std::string &source, const std::string &dest)
    { addOp(FILEOP_MOVE, source, dest); }
    void del(const std::string &target)
    { addOp(FILEOP_DELETE, "", target); }

  private:
    void doJob();
    typedef std::list<FileOp> OpList;
    OpList ops;
  };
}

#endif
