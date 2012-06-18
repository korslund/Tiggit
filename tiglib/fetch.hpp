#ifndef __TIGLIB_FETCH_HPP_
#define __TIGLIB_FETCH_HPP_

#include <string>

namespace TigLib
{
  void fetchFile(const std::string &url, const std::string &outfile);
  bool fetchIfOlder(const std::string &url, const std::string &outfile,
                    int minutes);
}

#endif
