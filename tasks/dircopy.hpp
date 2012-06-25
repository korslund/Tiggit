#ifndef __TASKS_DIRCOPY_HPP_
#define __TASKS_DIRCOPY_HPP_

#include "job/job.hpp"
#include <string>
#include <list>

namespace Tasks
{
  /* DirCopyTask is used to copy or move files or entire directories
     into a single target directory.

     Set up the destination path with setDest("my/path/"), where you
     also specify whether this is a "copy" or a "move" operation in
     the second parameter.

     Then add source files or directories with addSource().

     Example:

     setDest("destDir/");
     addSource("somepath/myfile1.txt");
     addSource("/some/other/directory/");
     run()

     => will create:

        destDir/myfile1.txt
        destDir/directory/  (complete recursive copy)
   */
  struct DirCopyTask : Jobify::Job
  {
    DirCopyTask(Jobify::JobInfoPtr info = Jobify::JobInfoPtr())
      : Jobify::Job(info) {}

    void setDest(const std::string &dst, bool move=false)
    { dest = dst; doMove = move; }

    void addSource(const std::string &path)
    { sources.push_back(path); }

  private:
    void doJob();
    void cleanup();
    std::list<std::string> sources;
    std::string dest;
    bool doMove;
  };
}
#endif
