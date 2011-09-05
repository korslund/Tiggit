#ifndef _DATALIST_HPP_
#define _DATALIST_HPP_

#include <wx/wx.h>
#include <string>
#include <vector>

struct DataList
{
  struct Entry
  {
    int status;
    wxString idname, name, desc, fpshot, tigurl, tigfile;
  };

  std::vector<Entry> arr;

  void add(int status, const wxString &idname, const wxString &name,
           const wxString &desc, const wxString &fpshot,
           const wxString &tigurl, const wxString &tigfile)
  {
    Entry e = { status, idname, name, desc, fpshot, tigurl, tigfile };
    arr.push_back(e);
  }

  void add(int status,
           const std::string &idname,
           const std::string &name,
           const std::string &desc,
           const std::string &fpshot,
           const std::string &tigurl,
           const std::string &tigfile)
  {
    add(status,
        wxString(idname.c_str(), wxConvUTF8),
        wxString(name.c_str(), wxConvUTF8),
        wxString(desc.c_str(), wxConvUTF8),
        wxString(fpshot.c_str(), wxConvUTF8),
        wxString(tigurl.c_str(), wxConvUTF8),
        wxString(tigfile.c_str(), wxConvUTF8));
  }
};

#endif
