#ifndef __TIGLIB_FETCH_HPP_
#define __TIGLIB_FETCH_HPP_

#include <string>

namespace Fetch
{
  // Fetch a file from an URL. Throws on failure.
  void fetchFile(const std::string &url, const std::string &outfile);

  // Fetch a file only if the outfile doesn't exist or is older than
  // 'minutes'.
  bool fetchIfOlder(const std::string &url, const std::string &outfile,
                    int minutes);

  /*
    Fetch the contents of the URL and return it as a string. Useful
    for REST APIs.

    Can also be used to send "fire-and-forget" style one-way signals
    by setting async=true. The task is then performed in a thread, the
    return data is discarded, and errors are ignored.
  */
  std::string fetchString(const std::string &url, bool async=false);
}

#endif
