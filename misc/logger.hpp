#ifndef __MISC_LOGGER_HPP_
#define __MISC_LOGGER_HPP_

#include <string>
#include <fstream>

namespace Misc
{
  struct Logger
  {
    std::ofstream log;
    std::string filename;

    // Set to true to write to stdout as well as to the log file
    bool print;

    Logger(const std::string &file);
    void operator()(const std::string &msg);
  };
}

#endif
