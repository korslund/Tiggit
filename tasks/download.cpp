#include "download.hpp"
#include "curl.hpp"
#include <boost/filesystem.hpp>
#include <mangle/stream/servers/null_stream.hpp>

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

  // Check for the no-output case
  if(!stream && file == "")
    // Create a NULL writer
    stream.reset(new Mangle::Stream::NullStream);

  if(stream)
    {
      setBusy("Downloading " + url);
      cURL::get(url, stream, userAgent, &prog);
    }
  else
    {
      using namespace boost::filesystem;

      /* Add ".part" to the temporary output filename, to avoid
         potentially overwriting existing valid data with invalid new
         data, in case of a failed download. This also signals to
         anyone inspecting the directory that this is not the complete
         file.
      */
      const std::string tmp = file + ".part";
      setBusy("Downloading " + url + " to " + tmp);

      create_directories(path(tmp).parent_path());
      cURL::get(url, tmp, userAgent, &prog);

      // Move the completed file into place
      remove(file);
      rename(tmp, file);
    }

  // All error handling is done through exceptions. If we get here,
  // everything is OK.
  setDone();
};
