#ifndef __JOBIFY_JOB_HPP
#define __JOBIFY_JOB_HPP

#include <string>
#include <boost/smart_ptr.hpp>
#include <stdint.h>

namespace Jobify
{
  enum JobStatus
    {
      ST_NONE,          // Job not yet created
      ST_CREATED,       // Job object created but not started
      ST_BUSY,          // Job is running
      ST_DONE,          // Job completed successfully
      ST_ERROR,         // Job failed
      ST_ABORT          // Job was aborted by user
    };

  /* Communication structure between a running job and the outside
     world.
   */
  struct JobInfo
  {
    // Progress. Only meant for informative purposes, not guaranteed
    // to be accurate.
    int64_t current, total;

    // See JobStatus
    int status;

    // Set to true if the outside world is requesting an abort
    bool doAbort;

    // Status or error message describing the current operation. Only
    // valid when isBusy() or isError() are true.
    std::string message;

    JobInfo() { reset(); }
    virtual ~JobInfo() {}

    void reset();

    virtual int64_t getCurrent() { return current; }
    virtual int64_t getTotal() { return total; }

    virtual void abort() { doAbort = true; }

    bool isCreated() const { return status >= ST_CREATED; }
    bool hasStarted() const { return status >= ST_BUSY; }
    bool isBusy() const { return status == ST_BUSY; }
    bool isSuccess() const { return status == ST_DONE; }
    bool isError() const { return status == ST_ERROR; }
    bool isAbort() const { return status == ST_ABORT; }
    bool isNonSuccess() const { return isError() || isAbort(); }
    bool isFinished() const { return isSuccess() || isNonSuccess(); }

    // These two may be used outside the Job class to signal external
    // status, if there is no running Job attached to this JobInfo
    // instance.
    void setDone() { status = ST_DONE; }
    void setError(const std::string &what);

    bool checkStatus();
  };

  typedef boost::shared_ptr<JobInfo> JobInfoPtr;

  struct Job
  {
    /* Status note: the constructor resets the info struct and sets
       the ST_CREATED status. Jobs may be run in separate threads, but
       will often be CREATED in one parent thread, meaning that this
       constructor is run in that thread.

       If that is the case, you can safely use info->isCreated() as a
       locking mechanism to avoid creating multiple jobs per Info
       struct.

       If you don't provide a JobInfo struct, one will be created.
     */
    Job(JobInfoPtr i = JobInfoPtr());
    virtual ~Job() {}

    void run();
    JobInfoPtr getInfo() { return info; }

  protected:
    /* Run the actual job. Overwrite this in child classes.
     */
    virtual void doJob() = 0;

    // Do optional exit handling. Called both on normal returns and
    // exceptions. Check 'info' for error status
    virtual void cleanup() {}

    JobInfoPtr info;

    // Check current status. If this returns true, doJob() should
    // immediately exit.
    bool checkStatus() { return info->checkStatus(); }

    void setBusy(const std::string &what = "");
    void setDone() { info->setDone(); }
    void setAbort();
    void setError(const std::string &what)
    { info->setError(what); }
  };
}

#endif
