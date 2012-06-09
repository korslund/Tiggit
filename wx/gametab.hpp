#ifndef __GAMETAB_HPP_
#define __GAMETAB_HPP_

#include "tabbase.hpp"

namespace wxTiggit
{
  struct ImageViewer;

  struct GameTab : TabBase
  {
    GameTab(wxNotebook *parent, const wxString &name, int listType);

  private:
    void gotFocus();
    wxString getTitle();

    wxString tabName;

    // Controls
    wxButton *b1, *b2;
    wxTextCtrl *textView;
    ImageViewer *screenshot;
    wxListBox *tags;
    wxChoice *rateBox;
    wxStaticText *rateText;
    wxString rateString[7];
  };
}
#endif
