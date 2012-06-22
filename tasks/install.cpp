#include "install.hpp"

#include "tasks/download.hpp"
#include "tasks/unpack.hpp"
#include <boost/filesystem.hpp>

namespace bs = boost::filesystem;
using namespace Tasks;

void InstallTask::doJob()
{
  add(new DownloadTask(url, zip));
  if(dir != "")
    add(new UnpackTask(zip, dir));

  try
    { MultiTask::doJob(); }
  catch(std::exception &e)
    { setError(e.what()); }
  catch(...)
    { setError("Unknown error"); }
}

void InstallTask::cleanup()
{
  // Remove archive on exit, if a directory was the final destination,
  // or if the download failed.
  if(dir != "" || info->isError())
    bs::remove(zip);
}
