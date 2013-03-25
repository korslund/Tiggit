#ifndef __APPWX_IMPORTER_HPP_
#define __APPWX_IMPORTER_HPP_

#include <spread/spread.hpp>
#include <spread/misc/logger.hpp>
#include <vector>

namespace Import
{
  void importConfig(const std::string &from, const std::string &to, Misc::Logger &logger);

  void getGameList(std::vector<std::string> &games, const std::string &from,
                   Misc::Logger &logger);

  Spread::JobInfoPtr importGame(const std::string &game,
                                const std::string &from, const std::string &to,
                                Spread::SpreadLib *spread, Misc::Logger &logger,
                                bool async=true);

  Spread::JobInfoPtr importShots(const std::string &from, const std::string &to,
                                 Spread::SpreadLib *spread, Misc::Logger &logger,
                                 bool async=true);

  void cleanup(const std::string &from, const std::vector<std::string> &games,
               Misc::Logger &logger);

  Spread::JobInfoPtr copyFiles(const std::string &from, const std::string &to,
                               Spread::SpreadLib *spread,
                               Misc::Logger *logger = NULL);
}

#endif
