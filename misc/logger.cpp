#include "logger.hpp"
#include <iostream>
#include <boost/filesystem.hpp>

using namespace Misc;
namespace bf = boost::filesystem;

Logger::Logger(const std::string &file)
  : filename(file), print(false)
{
  if(bf::exists(file))
    {
      std::string old = file + ".old";
      if(bf::exists(old))
        bf::remove(old);
      bf::rename(file, old);
    }
  log.open(file.c_str());
}

void Logger::operator()(const std::string &msg)
{
  if(print)
    std::cout << filename << ": " << msg << "\n";
  char buf[100];
  time_t now = std::time(NULL);
  std::strftime(buf, 100, "%Y-%m-%d %H:%M:%S", gmtime(&now));
  log << buf << ":   " << msg << "\n";
  // Flush after each line in case of crashes
  log.flush();
}

