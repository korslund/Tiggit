#ifndef __TIGGIT_DIALOGS_HPP_
#define __TIGGIT_DIALOGS_HPP_

#ifndef wxUSE_UNICODE
#define wxUSE_UNICODE 1
#endif

#include <wx/wx.h>
#include <string>
#include <vector>

struct BrowseDialog : wxDialog
{
  BrowseDialog(wxWindow *parent, const wxString &title,
               int width, int height, bool file=false);

protected:
  void addTo(wxSizer *sizer, const wxString &value=wxT(""));

  wxTextCtrl *edit;

private:
  bool browseFile; // Find file or directory. Default is directory.
  void onBrowse(wxCommandEvent &event);
};

struct OutputDirDialog : BrowseDialog
{
  OutputDirDialog(wxWindow *parent, const std::string &old_dir);

  bool ok, move, changed;
  std::string path;
};

/*
struct ImportDialog : BrowseDialog
{
  ImportDialog(wxWindow *parent, const std::string &maindir);

  bool ok, copy;
  std::string source;
};

struct ExportDialog : BrowseDialog
{
  ExportDialog(wxWindow *parent, const std::vector<std::string> &games);

  bool ok, launcher;
  std::string output;
  std::vector<int> selected;
};

struct AddExternalDialog : BrowseDialog
{
  AddExternalDialog(wxWindow *parent);

  bool ok;
  std::string name, exe;
};
*/
#endif
