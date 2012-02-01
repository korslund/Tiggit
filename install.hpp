#ifndef _INSTALL_HPP
#define _INSTALL_HPP

#include <queue>
#include <boost/filesystem.hpp>

#include "unzip.hpp"

/*
  Multi-threaded zip installer.

  Since there's little to gain in terms of multi-threaded unpacking
  (CPU and disk bandwidth are shared, not to mention disk
  fragmentation issues), this struct only uses one installer thread at
  any given time.

  However, doing this in a thread does free up the rest of the program
  to work while unpacking is taking place.

  Install commands are queued by queue(), and are performed serially
  by one worker thread.

  We avoid locking and all the other the other complicated
  inter-thread communication issues by simply starting a new thread
  per operation, and waiting until it finishes.
 */
class Installer
{
  struct Job
  {
    /*
      0 - not started
      1 - working
      2 - done
      3 - error
      4 - user abort
     */
    int status;
    std::string zip, dir;

    Job(const std::string &_zip,
        const std::string &_dir)
      : status(0), zip(_zip), dir(_dir)
    {}
  };

  Job *current;

  std::queue<Job*> list;

  struct Worker : wxThread
  {
    Job *job;

    Worker(Job *j) : job(j)
    {
      assert(j);
      Create();
      Run();
    }

    ExitCode Entry()
    {
      using namespace boost::filesystem;
      using namespace std;
      job->status = 1;

      string zip = job->zip;
      string dir = job->dir;

      // Make sure the directory exists
      create_directories(dir);

      // Unzip the file
      UnZip unzip;

      bool error = false;
      try { unzip.unpack(zip,dir); }
      catch(std::exception &e)
        {
          cout << "Error: " << e.what() << endl;
          error = true;
        }

      // We're done
      if(error)
        {
          job->status = 3;

          // Remove install directory
          remove_all(dir);
        }
      else
        job->status = 2;

      // Delete the zip file
      remove(zip);

      return 0;
    }
  };

public:

  Installer() : current(NULL) {}

  /* Queue a zip file for threaded installation in the given
     destination dir.

     Returns a handle usable for calling check()
  */
  void *queue(const std::string &zip,
              const std::string &where)
  {
    Job *e = new Job(zip, where);
    list.push(e);
    return e;
  }

  /* Check the status of a given install process.

     Return values:
     0 - still queued
     1 - currently unpacking
     2 - done
     3 - error
     4 - user abort

     After returning values 2 and up, the given pointer handle is no
     longer valid and should not be used again.

     Besides checking the status of this specific download, this
     function also does thread polling and other household work, and
     thus you should make sure to call it regularly if you want the
     queued installs to proceed smoothly.
   */
  int check(void* p)
  {
    // Do house keeping first. Is there an install in progress?
    if(current)
      {
        // Yup, check it.
        if(current->status >= 2)
          // It's done, reset the current pointer. The object will be
          // deleted later, in this or a subsequent call to check().
          current = NULL;
      }

    // Is there room for a new thread, and is there anything more to
    // do?
    if(current == NULL &&
       !list.empty())
      {
        // Yup, so get going!
        Job *ne = list.front();
        list.pop();

        // Check if the user has already aborted this action
        if(ne->status != 4)
          {
            // If not, go ahead with it.
            current = ne;
            new Worker(ne);
          }

        // If the action was aborted, don't bother looping around to
        // find a good one. check() will be called again soon
        // enough. The abort() function has already deleted the file.
      }

    // Next, handle the parameter element
    assert(p);
    Job *e = (Job*)p;

    int stat = e->status;

    // Delete the object if it's done.
    if(stat >= 2)
      {
        // It's possible (due to threaded execution order) that e ==
        // current. If so, remember to reset current as well, to make
        // sure we remove all references.
        if(current == e)
          current = NULL;

        delete e;
      }

    return stat;
  }

  // Abort a given unpack operation
  void abort(void* p)
  {
    assert(p);
    Job *e = (Job*)p;

    // Unless we're running, we can safely delete the zip file at this
    // point.
    if(e != current)
      {
        e->status = 4;
        boost::filesystem::remove(e->zip);
      }

    // At the moment, running installs cannot be aborted
  }
};

Installer inst;

#endif
