#ifndef __MISC_FREESPACE_HPP_
#define __MISC_FREESPACE_HPP_

#include <string>
#include <stdint.h>

namespace Misc
{
  /* Get total and free disk space for any given file or directory.
   */
  void getDiskSpace(const std::string &filePath, int64_t &free, int64_t &total);
}

#endif
