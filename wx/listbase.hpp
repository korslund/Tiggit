#ifndef __WX_LISTBASE_HPP_
#define __WX_LISTBASE_HPP_

#include "wxcommon.hpp"
#include <wx/listctrl.h>

namespace wxTiggit
{
  struct ListBase : public wxListCtrl
  {
    ListBase(wxWindow *parent, int id);

  protected:
    wxListItemAttr *OnGetItemAttr(long item) const { return NULL; }
    int OnGetItemImage(long item) const { return -1; }
    int OnGetColumnImage(long item, long column) const { return -1; }

  private:
    void onKeyDown(wxKeyEvent &evt);
  };
}
#endif
