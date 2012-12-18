#ifndef __APPWX_IMPORTER_GUI_HPP_
#define __APPWX_IMPORTER_GUI_HPP_

#include <spread/spread.hpp>

namespace ImportGui
{
  bool importRepoGui(const std::string &from, const std::string &to,
                     Spread::SpreadLib *spread, bool doCleanup);

  bool copyFilesGui(const std::string &from, const std::string &to,
                    Spread::SpreadLib *spread, const std::string &text);

  void doUserCleanup(const std::string &repoDir);
}

#endif
