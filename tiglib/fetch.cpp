#include "fetch.hpp"
#include <boost/filesystem.hpp>
#include "tasks/download.hpp"
#include <time.h>

void TigLib::fetchFile(const std::string &url, const std::string &outfile)
{
  using namespace Jobify;

  JobInfoPtr info(new JobInfo);
  Tasks::DownloadTask dl(url, outfile, info);
  dl.run();
  if(!info->isSuccess())
    throw std::runtime_error("Failed to download " + url);
}

bool TigLib::fetchIfOlder(const std::string &url,
                          const std::string &outfile,
                          int minutes)
{
  namespace bs = boost::filesystem;

  // Check age if file exists
  if(bs::exists(outfile))
    {
      time_t ft = bs::last_write_time(outfile);
      time_t now = time(0);

      // If we haven't expired, keep existing file
      if(difftime(now,ft) < 60*minutes)
        return false;
    }

  // Otherwise, go get it!
  fetchFile(url, outfile);
  return true;
}

