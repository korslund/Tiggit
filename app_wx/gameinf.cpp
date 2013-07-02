#include "gameinf.hpp"
#include "notifier.hpp"
#include "jobprogress.hpp"
#include "wx/boxes.hpp"

#include <time.h>

using namespace TigData;
using namespace wxTigApp;
using namespace TigLib;

// Create a nice size string
static wxString sizify(uint64_t size)
{
  wxString tmp;
  if(size > 1024*1024*1024)
    tmp << wxString::Format(wxT("%.1f"), size/(1024.0*1024*1024)) << wxT("Gb");
  else if(size > 1024*1024)
    tmp << wxString::Format(wxT("%.1f"), size/(1024.0*1024)) << wxT("Mb");
  else if(size > 1024)
    tmp << wxString::Format(wxT("%.1f"), size/1024.0) << wxT("Kb");
  else
    tmp << size;
  return tmp;
}

// Friendly time formatting, spits out strings like "3 days ago"
static wxString ago(time_t when, time_t now=0)
{
  if(now == 0)
    now = time(NULL);

  time_t diff = now-when;

  wxString res;

  int times[6] = {60,60,24,7,4,12};
  wxString names[6] = {wxT("second"),wxT("minute"),wxT("hour"),wxT("day"),
                       wxT("week"),wxT("month")};
  for(int i=0;i<6;i++)
    {
      if(diff < times[i])
        {
          res = wxString::Format(wxT("%d ") + names[i], diff);
          if(diff > 1) res += wxT("s");
          res += wxT(" ago");
          break;
        }
      diff /= times[i];
    }

  if(!res.IsEmpty())
    return res;

  // If it's more than a year ago, use dates instead
  char buf[50];
  strftime(buf,50, "%Y-%m-%d", gmtime(&when));
  return wxString(buf, wxConvUTF8);
}

void GameInf::updateStatus()
{
  titleStatus = title;

  if(isWorking())
    {
      int64_t current, total;
      info.progress(current, total);

      int percent = 0;
      if(total > 0)
        percent = (int)((100.0*current)/total);

      // Update visible progress strings
      wxString status;
      status << percent << wxT("%");
      titleStatus += wxT(" [") + status + wxT("]");

      status << wxT(" (") << sizify(current) << wxT(" / ")
             << sizify(total) << wxT(")");
      statusStr = status;
    }
  else if(isInstalled())
    titleStatus += wxT(" [installed]");
}

void GameInf::updateAll()
{
  const TigEntry *ent = info.ent;

  title = strToWx(ent->title);
  desc = strToWx(ent->desc);

  if(info.instSize) instSize = strToWx(sizify(info.instSize));
  else instSize = wxT("Unknown");

  if(info.version != "") version = strToWx(info.version);
  else version = wxT("unknown");

  dlStr = wxString::Format(wxT("%d"), ent->dlCount);
  if(ent->rateCount > 0 && ent->rating > 0)
    {
      rateStr = wxString::Format(wxT("%3.2f"), ent->rating);
      rateStr2 = rateStr + wxString::Format(wxT(" (%d)"), ent->rateCount);
    }

  timeStr = ago(ent->addTime);

  updateStatus();
}

std::string GameInf::getHomepage() const
{ return info.ent->homepage; }
std::string GameInf::getIdName() const
{ return info.ent->idname; }
std::string GameInf::getDir() const
{ return info.getInstallDir(); }
int GameInf::myRating() const
{ return info.getMyRating(); }
void GameInf::rateGame(int i)
{ info.setMyRating(i); }

const wxImage &GameInf::getShot()
{
  if(!shotIsLoaded)
    {
      // Load the image file, and ignore error messages
      std::string file = info.getScreenshot();

      wxLogNull dontLog;
      if(file != "" && screenshot.LoadFile(strToWx(file)))
        shotIsLoaded = true;
    }

  return screenshot;
}

void GameInf::installGame()
{
  info.install();
  updateStatus();

  // Add ourselves to the watch list
  notify.watchMe(this);
}

void GameInf::uninstallGame()
{
  info.uninstall();
  updateStatus();

  notify.statusChanged();
}

void GameInf::abortJob() { info.abort(); }
void GameInf::launchGame()
{
  /* Simple ad-hoc solution for now, improve it later.
   */
  Spread::JobInfoPtr job = info.update();
  bool run = true;
  if(job)
    {
      JobProgress prog(job);

      if(!prog.start("Downloading update for " + info.ent->title + "..."))
        {
          if(job->isError())
            run = Boxes::ask("Update failed: " + job->getMessage() + "\n\nRun anyway?");
        }
    }

  if(run)
    info.launch();
}
