#ifndef __WXAPP_GAMEINF_HPP_
#define __WXAPP_GAMEINF_HPP_

#include "tiglib/liveinfo.hpp"
#include "gameconf.hpp"
#include "libraries.hpp"

using namespace wxTiggit;

namespace wxTigApp
{
  struct GameInf : wxGameInfo
  {
    GameInf(TigLib::LiveInfo *_info, GameConf *_conf, LibraryHandler *_libs)
      : info(*_info), shotIsLoaded(false), conf(_conf), libs(_libs)
    { updateAll(); }

    bool isInstalled() const { return info.isInstalled(); }
    bool isUninstalled() const { return info.isUninstalled(); }
    bool isWorking() const { return info.isWorking(); }
    bool isDemo() const { return info.ent->isDemo(); }
    bool isNew() const { return info.isNew(); }

    // Update status strings
    void updateStatus();

    TigLib::LiveInfo &info;

  private:
    // Screenshot data
    wxImage screenshot;
    bool shotIsLoaded;

    GameConf *conf;
    LibraryHandler *libs;

    wxString title, titleStatus, timeStr, rateStr, rateStr2, dlStr, statusStr, desc, instSize, version;

    // Used to update cached wxStrings from source data
    void updateAll();

    wxString getTitle(bool includeStatus=false) const
    { return includeStatus?titleStatus:title; }
    wxString timeString() const { return timeStr; }
    wxString dlString() const { return dlStr; }
    wxString statusString() const { return statusStr; }
    wxString getDesc() const { return desc; }
    wxString getSize() const { return instSize; }
    wxString getVersion() const { return version; }
    wxString rateString() const
    { return conf->show_votes?rateStr2:rateStr; }

    std::string getHomepage() const;
    std::string getIdName() const;
    std::string getDir() const;
    int myRating() const;

    void rateGame(int i);
    const wxImage &getShot();

    void installGame();
    void uninstallGame();
    void launchGame();
    void abortJob();
  };
}
#endif
