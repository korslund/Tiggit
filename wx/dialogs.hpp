#ifndef __TIGGIT_DIALOGS_HPP_
#define __TIGGIT_DIALOGS_HPP_

#include "wxcommon.hpp"
//#include <vector>

namespace wxTiggit
{
  // Base class for dialogs that have a "browse file/directory" entry.
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

  /* Select a new repository directory.

     Parameters:

       old_dir    - old directory (in case of moving), or suggestion for new
                    directory (in case of a fresh install)

       legacy_dir - If non-empty, prompt the user if they want to
                    import from the given directory. The answer is
                    recorded in doImport.

       writeFailed - display a message saying that the previously selected directory was not                      writable

       freshInstall - treat this as a fresh install, where the user gets to select the
                      directory for the first time

   */
  struct OutputDirDialog : BrowseDialog
  {
    OutputDirDialog(wxWindow *parent, const std::string &old_dir,
                    const std::string &legacy_dir = "",
                    bool writeFailed=false, bool freshInstall=false);

    bool ok, move, changed, doImport;
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
}
#endif
