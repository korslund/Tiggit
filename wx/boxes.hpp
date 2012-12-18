#ifndef __WX_BOXES_HPP_
#define __WX_BOXES_HPP_

#include "wxcommon.hpp"

namespace wxTiggit
{
  struct Boxes
  {
    // Ask the user an OK/Cancel question.
    static bool ask(const wxString &question)
    {
      return wxMessageBox(question, wxT("Please confirm"),
                          wxOK | wxCANCEL | wxICON_QUESTION) == wxOK;
    }

    // Display an error message box
    static void error(const wxString &msg)
    {
      wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
    }

    static void say(const wxString &msg)
    {
      wxMessageBox(msg, wxT("Information"), wxOK | wxICON_INFORMATION);
    }

    static bool ask(const std::string &q) { return ask(strToWx(q)); }
    static void error(const std::string &q) { error(strToWx(q)); }
    static void say(const std::string &q) { say(strToWx(q)); }
  };
}
#endif
