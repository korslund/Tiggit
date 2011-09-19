#ifndef _DATALIST_HPP_
#define _DATALIST_HPP_

#include <wx/wx.h>
#include <string>
#include <vector>

struct DataList
{
  struct TigInfo
  {
    std::string url, launch, subdir, version;
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
    wxString urlname, idname, name, desc, fpshot, tigurl;

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
           const wxString &fpshot, const wxString &tigurl,
           const TigInfo &tiginfo)
  {
    Entry e = { status, urlname, idname, name, desc, fpshot,
                tigurl, tiginfo, NULL, wxT("") };
    arr.push_back(e);
  }

  void add(int status,
           const std::string &urlname,
           const std::string &idname,
           const std::string &name,
           const std::string &desc,
           const std::string &fpshot,
           const std::string &tigurl,
           const TigInfo &tiginfo)
  {
    add(status,
        wxString(urlname.c_str(), wxConvUTF8),
        wxString(idname.c_str(), wxConvUTF8),
        wxString(name.c_str(), wxConvUTF8),
        wxString(desc.c_str(), wxConvUTF8),
        wxString(fpshot.c_str(), wxConvUTF8),
        wxString(tigurl.c_str(), wxConvUTF8),
        tiginfo);
  }
};

#endif
