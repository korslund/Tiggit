#ifndef __RUN_WINDOWS_HPP_
#define __RUN_WINDOWS_HPP_

#ifdef _WIN32

#include <string>

namespace Launcher
{
  void win32_run(const std::string &command, const std::string &workdir = "");
}

#endif
#endif
