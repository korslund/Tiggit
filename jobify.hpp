#ifndef __JOBIFY_HPP_
#define __JOBIFY_HPP_

// We're currently hard-coded to use wxThread, but there's no reason
// why we can't abstract this later to get identical functionality
// with different libraries.
#ifndef wxUSE_UNICODE
#define wxUSE_UNICODE 1
#endif
#include <wx/thread.h>
#include <string>

struct StatusJob
{
  StatusJob() : status(ST_NONE), doAbort(false) {}

  // Call this to abort the job
  void abort() { doAbort = true; }

  // Get current status
  bool hasStarted() const { return status != ST_NONE; }
  bool isBusy() const { return status == ST_BUSY; }
  bool isSuccess() const { return status == ST_DONE; }
  bool isError() const { return status == ST_ERROR; }
  bool isAbort() const { return status == ST_ABORT; }
  bool isNonSuccess() const { return isError() || isAbort(); }
  bool isFinished() const { return isSuccess() || isNonSuccess(); }
  bool abortRequested() const { return doAbort; }

  std::string getError() const { return errMsg; }

  // Entry point for this job.
  virtual void executeJob() = 0;
  virtual ~StatusJob() {}

  // For non-thread jobs, just run the job directly
  virtual void run() { executeJob(); }

protected:
  void setError(const std::string &msg)
  {
    errMsg = msg;
    status = ST_ERROR;
  }

  void setBusy() { status = ST_BUSY; }
  void setDone() { status = ST_DONE; }
  void setAbort() { status = ST_ABORT; }

private:
  enum Status
    {
      ST_NONE   = 0,    // Job not yet started
      ST_BUSY   = 1,    // Job is running
      ST_DONE   = 2,    // Job completed successfully
      ST_ERROR  = 3,    // Job failed
      ST_ABORT  = 4     // Job was aborted by user
    };

  int status;
  bool doAbort;

  std::string errMsg;
};

struct ThreadJob : StatusJob
{
  // Run this job. Will create a new thread that runs executeJob.
  void run()
  {
    new wxJobRunner(this);
  }

private:
  // This is where the magic happens
  struct wxJobRunner : wxThread
  {
    StatusJob *job;

    wxJobRunner(StatusJob *j) : job(j)
    {
      assert(job);
      Create();
      Run();
    }

    ExitCode Entry()
    {
      job->executeJob();
      return 0;
    }
  };
};

#endif
