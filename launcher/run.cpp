#include "run.hpp"

#include <stdexcept>

void noImpl(const std::string &command, const std::string &workdir = "")
{
  throw std::runtime_error("Cannot run " + command + ": Launching is not yet implemented on this platform");
}

#ifdef _WIN32
#include "run_windows.hpp"
#define RUN win32_run
#else
#define RUN noImpl
#endif

void Launcher::run(const std::string &command, const std::string &workdir)
{
  RUN (command, workdir);
}
