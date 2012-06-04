#include "downloadjob.hpp"

#include "curl_get.hpp"
#include <boost/filesystem.hpp>

void DownloadJob::executeJob()
{
  // We need local variables since in principle, the object may be
  // deleted after the finish status functions (setDone() etc) are
  // called.
  std::string fname = file;
  bool success = false;

  try
    {
      CurlGet::get(url, file, &curl_progress, this);

      if(abortRequested()) setAbort();
      else
        {
          setDone();
          success = true;
        }
    }
  catch(std::exception &e)
    {
      setError(e.what());
    }

  if(!success)
    boost::filesystem::remove(fname);
}
