#include "gameinf.hpp"
#include "notifier.hpp"

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

  title = strToWx(ent->tigInfo.title);
  desc = strToWx(ent->tigInfo.desc);

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
{ return info.ent->tigInfo.homepage; }
std::string GameInf::getTiggitPage() const
{ return "http://tiggit.net/game/" + info.ent->urlname; }
std::string GameInf::getIdName() const
{ return info.ent->idname; }
std::string GameInf::getDir() const
{ return info.getInstallDir(); }
int GameInf::myRating() const
{ return info.getMyRating(); }
void GameInf::rateGame(int i)
{ info.setMyRating(i); }

// Called when a screenshot file is ready, possibly called by TigLib
// from a worker thread, but may also be called directly from
// requestShot().
void GameInf::shotIsReady(const std::string &idname,
                          const std::string &file)
{
  assert(idname == getIdName());

  // Are we in unloaded-mode?
  if(loaded == 1)
    {
      // Load the image file
      wxLogNull dontLog;
      if(!screenshot.LoadFile(strToWx(file)))
        return;

      loaded = 2;
    }

  // We should now be loaded
  assert(loaded == 2);

  // Pass the event to the wxEvtHandler. It can handle threaded
  // queuing.
  ScreenshotEvent evt;
  evt.id = idname;
  evt.shot = &screenshot;
  shotHandler->AddPendingEvent(evt);
}

void GameInf::requestShot(wxEvtHandler *cb)
{
  // Don't do anything if we are already working or if there was a
  // failure.
  if(loaded == 1) return;

  // Store the handler so shotIsReady() finds it
  shotHandler = cb;

  // If the shot was already loaded, just return it directly
  if(loaded == 2)
    {
      shotIsReady(getIdName(), "");
      return;
    }

  assert(loaded == 0);

  // Set 'working' status
  loaded = 1;

  // Delegate file fetching to TigLib
  info.requestShot(this);
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
void GameInf::launchGame() { info.launch(); }
