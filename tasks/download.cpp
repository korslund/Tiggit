#include "download.hpp"
#include "curl.hpp"
#include <boost/filesystem.hpp>

using namespace Tasks;

std::string DownloadTask::userAgent = "Tiggit/1.0 - see http://tiggit.net/";

struct DLProgress : cURL::Progress
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

void DownloadTask::doJob()
{
  DLProgress prog;
  prog.info = info;

  if(stream)
    {
      setBusy("Downloading " + url);
      cURL::get(url, stream, userAgent, &prog);
    }
  else
    {
      // Use a temporary output file to avoid potentially overwriting
      // existing data with a botched download, and also to signal
      // that this is not the completed file.
      const std::string tmp = file + ".part";

      setBusy("Downloading " + url + " to " + tmp);
      cURL::get(url, tmp, userAgent, &prog);

      // Move the completed file into place
      using namespace boost::filesystem;
      remove(file);
      rename(tmp, file);
    }

  // All error handling is done through exceptions. If we get here,
  // everything is OK.
  setDone();
};
