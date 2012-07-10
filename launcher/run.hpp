#ifndef __LAUNCHER_RUN_HPP_
#define __LAUNCHER_RUN_HPP_

#include <string>

namespace Launcher
{
  /* Run the given command in the specified working directory. If no
     dir is specified, use the current process' working dir.
   */
  void run(const std::string &command, const std::string &workdir = "");
}

#endif
