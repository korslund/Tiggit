#ifndef __UNPACK_PROGRESS_HPP
#define __UNPACK_PROGRESS_HPP

#include <stdint.h>

namespace Unpack
{
  // Callback used for progress reports
  struct Progress
  {
    // Return true if OK, false to abort unpacking.
    virtual bool progress(int64_t total, int64_t now) = 0;
  };
}
#endif
