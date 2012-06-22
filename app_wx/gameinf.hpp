#ifndef __WXAPP_GAMEINF_HPP_
#define __WXAPP_GAMEINF_HPP_

#include "tiglib/liveinfo.hpp"
#include "gameconf.hpp"

using namespace wxTiggit;

namespace wxTigApp
{
  struct GameInf : wxGameInfo, TigLib::ShotIsReady
  {
    GameInf(TigLib::LiveInfo *_info, GameConf *_conf)
      : info(*_info), loaded(0), conf(_conf),
        shotHandler(NULL)
    { updateAll(); }

    bool isInstalled() const { return info.isInstalled(); }
    bool isUninstalled() const { return info.isUninstalled(); }
    bool isWorking() const { return info.isWorking(); }
    bool isDemo() const { return info.ent->tigInfo.isDemo; }
    bool isNew() const { return info.isNew(); }

    // Update status strings
    void updateStatus();

    TigLib::LiveInfo &info;

  private:
    // Screenshot data
    wxImage screenshot;
    int loaded;
    wxEvtHandler *shotHandler;

    GameConf *conf;

    wxString title, titleStatus, timeStr, rateStr, rateStr2, dlStr, statusStr, desc;

    // Used to update cached wxStrings from source data
    void updateAll();

    // Inherited from TigLib::ShotIsReady
    void shotIsReady(const std::string &idname,
                     const std::string &file);

    wxString getTitle(bool includeStatus=false) const
    { return includeStatus?titleStatus:title; }
    wxString timeString() const { return timeStr; }
    wxString dlString() const { return dlStr; }
    wxString statusString() const { return statusStr; }
    wxString getDesc() const { return desc; }
    wxString rateString() const
    { return conf->show_votes?rateStr2:rateStr; }

    std::string getHomepage() const;
    std::string getTiggitPage() const;
    std::string getIdName() const;
    std::string getDir() const;
    int myRating() const;

    void rateGame(int i);
    void requestShot(wxEvtHandler*);

    void installGame();
    void uninstallGame();
    void launchGame();
    void abortJob();
  };
}
#endif
