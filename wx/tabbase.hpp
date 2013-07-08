#ifndef __TABBASE_HPP_
#define __TABBASE_HPP_

#include "wxcommon.hpp"
#include <wx/notebook.h>

namespace wxTiggit
{
  struct TabBase : wxPanel
  {
    TabBase(wxNotebook *parent, const wxString &name);

    // Refresh tab title
    void updateTitle();

    // Call this when files on disk have changed (promo images, news
    // files etc)
    virtual void reloadData() {}

    // Called when the tab is selected
    virtual void gotFocus() {}

    void select()
    {
      book->SetSelection(tabNum);
      gotFocus();
    }

    bool isEmpty() { return getTitleNumber() == 0; }

  protected:
    // Override to return tab title. Will by default display "tabName
    // (number)".
    virtual wxString getTitle();

    // Produce the number seen in the tab title. Returning zero will
    // show no number.
    virtual int getTitleNumber() = 0;

  private:
    wxNotebook *book;
    wxString tabName;
    int tabNum; // Current tab placement
    void onFocus(wxFocusEvent &evt) { gotFocus(); }
  };
}
#endif
