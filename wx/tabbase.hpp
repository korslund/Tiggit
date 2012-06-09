#ifndef __TABBASE_HPP_
#define __TABBASE_HPP_

#include "wxcommon.hpp"
#include <wx/notebook.h>

namespace wxTiggit
{
  struct TabBase : wxPanel
  {
    TabBase(wxNotebook *parent);

    // Refresh tab title
    void updateTitle();

    // Called when the tab is selected
    virtual void gotFocus() {}

  protected:
    // Override to return tab title
    virtual wxString getTitle() = 0;

  private:
    wxNotebook *book;
    int tabNum; // Current tab placement
  };
}
#endif
