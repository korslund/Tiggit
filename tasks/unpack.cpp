#include "unpack.hpp"
#include <unpack/auto.hpp>

using namespace Tasks;

struct UProgress : Unpack::Progress
{
  Jobify::JobInfoPtr info;

  bool progress(int64_t total, int64_t now)
  {
    info->current = now;
    info->total = total;

    // Abort the download if the user requested it.
    if(info->checkStatus())
      return false;

    return true;
  }
};

void UnpackTask::doJob()
{
  UProgress prog;
  prog.info = info;

  Unpack::AutoUnpack unp;

  if(dir != "")
    {
      setBusy("Unpacking " + file + " to " + dir);
      unp.unpack(file, dir, &prog, list);
    }
  else
    {
      setBusy("Unpacking " + file);
      unp.unpack(file, writeTo, &prog, list);
    }

  // All error handling is done through exceptions. If we get here,
  // everything is OK.
  setDone();
};
