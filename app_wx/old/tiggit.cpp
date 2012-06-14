#include <wx/hyperlink.h>
#include <wx/imaglist.h>
#include <wx/stdpaths.h>

DataList data;
JsonInstalled jinst;
TigListReader tig_reader;
TagSorter tagSorter;

// Ask the user an OK/Cancel question.
bool ask(const wxString &question)
{
  return wxMessageBox(question, wxT("Please confirm"),
                      wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
}

// Display an error message box
void errorBox(const wxString &msg)
{
  wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
}

// Update data from all_games.json. If the parameter is true, force
// download. If not, download only if the existing file is too old.
void updateData(bool download)
{
  tig_reader.loadData(lstfile, data);
  jinst.read(data);
  tagSorter.process(data);
}
