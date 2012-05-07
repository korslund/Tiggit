#ifndef _INSTALL_HPP
#define _INSTALL_HPP

#include "jobify.hpp"

struct ZipJob : ThreadJob
{
  int status;
  std::string zip, dir;

  ZipJob(const std::string &_zip, const std::string &_dir)
    : zip(_zip), dir(_dir)
  {}

  void executeJob();
};

#endif
