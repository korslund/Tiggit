#ifndef _DATALIST_HPP_
#define _DATALIST_HPP_

#include <wx/wx.h>
#include <string>
#include <vector>
#include <stdint.h>
#include <time.h>

struct DataList
{
  struct TigInfo
  {
    std::string url, launch, subdir, version, title, desc,
      shot, shot80x50, location, devname, homepage;
  };

  struct Entry
  {
    /*
      0 - not installed
      1 - downloading
      2 - ready to play
      3 - unpacking
     */
    int status;

    // idname = channel/urlname
    wxString urlname, idname, name, desc, tigurl;

    // Time added to the channel database
    int64_t add_time;

    // Used for caching add_time as a string
    wxString timeString;

    TigInfo tigInfo;

    // Extra data, used for file downloads and other status
    // information
    void *extra;

    // Status message used in some cases
    wxString msg;
  };

  std::vector<Entry> arr;

  void add(int status, const wxString &urlname, const wxString &idname,
           const wxString &name, const wxString &desc,
           const wxString &tigurl, int64_t add_time,
           const TigInfo &tigInfo)
  {
    Entry e;
    e.status = status;
    e.urlname = urlname;
    e.idname = idname;
    e.name = name;
    e.desc = desc;
    e.tigurl = tigurl;
    e.add_time = add_time;
    e.tigInfo = tigInfo;
    e.extra = NULL;

    time_t t = add_time;
    char buf[50];
    strftime(buf,50, "%F"/*"%F %T"*/, gmtime(&t));
    e.timeString = wxString(buf, wxConvUTF8);

    arr.push_back(e);
  }

  void add(int status,
           const std::string &urlname,
           const std::string &idname,
           const std::string &name,
           const std::string &desc,
           const std::string &tigurl,
           int64_t add_time,
           const TigInfo &tiginfo)
  {
    add(status,
        wxString(urlname.c_str(), wxConvUTF8),
        wxString(idname.c_str(), wxConvUTF8),
        wxString(name.c_str(), wxConvUTF8),
        wxString(desc.c_str(), wxConvUTF8),
        wxString(tigurl.c_str(), wxConvUTF8),
        add_time, tiginfo);
  }
};

#endif
