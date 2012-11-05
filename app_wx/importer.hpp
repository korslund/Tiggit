#ifndef __APPWX_IMPORTER_HPP_
#define __APPWX_IMPORTER_HPP_

#include <spread/spread.hpp>
#include "../misc/logger.hpp"

namespace wxTigApp
{
  void copyTest(const std::string &from, const std::string &to, bool addPng,
                Spread::SpreadLib *spread, Spread::JobInfoPtr info, Misc::Logger *logger);
}

#endif
