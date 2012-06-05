#include "download.hpp"
#include "curl.hpp"

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
      setBusy("Downloading " + url + " to " + file);
      cURL::get(url, file, userAgent, &prog);
    }

  // All error handling is done through exceptions. If we get here,
  // everything is OK.
  setDone();
};
