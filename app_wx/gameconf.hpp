#ifndef __WXAPP_GAMECONF_HPP_
#define __WXAPP_GAMECONF_HPP_

#include "wx/wxgamedata.hpp"
#include <spread/misc/jconfig.hpp>

namespace wxTigApp
{
  struct GameConf : wxTiggit::wxGameConf
  {
    Misc::JConfig conf;

    // This is used frequently, so cache it
    bool show_votes;

    GameConf(const std::string &file)
      : conf(file)
    { show_votes = conf.getBool("show_votes"); }

    bool getShowVotes() { return show_votes; }
    void setShowVotes(bool b)
    {
      conf.setBool("show_votes", b);
      show_votes = b;
    }
  };
}
#endif
