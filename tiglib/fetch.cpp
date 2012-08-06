#include "fetch.hpp"
#include <time.h>
#include <mangle/stream/servers/string_writer.hpp>
#include <boost/filesystem.hpp>
#include <spread/tasks/download.hpp>
#include <spread/job/thread.hpp>

using namespace Spread;

void Fetch::fetchFile(const std::string &url, const std::string &outfile)
{
  DownloadTask dl(url, outfile);
  JobInfoPtr info = dl.run();
  if(!info->isSuccess())
    throw std::runtime_error("Failed to download " + url);
}

// Fetch a file only if the outfile doesn't exist or is older than
// 'minutes'.
bool Fetch::fetchIfOlder(const std::string &url,
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

std::string Fetch::fetchString(const std::string &url, bool async)
{
  using namespace Mangle::Stream;

  StreamPtr out;
  std::string res;

  if(!async)
    out.reset(new StringWriter(res));

  Job *job = new DownloadTask(url, out);

  JobInfoPtr info = Thread::run(job, async);
  if(!async && !info->isSuccess())
    throw std::runtime_error("Failed to get " + url);

  return res;
}
