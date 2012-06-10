#include "tabbase.hpp"
#include <assert.h>

using namespace wxTiggit;

TabBase::TabBase(wxNotebook *parent, const wxString &name)
  : wxPanel(parent), book(parent), tabName(name)
{
  assert(book);
  book->AddPage(this, wxT(""));
  tabNum = book->GetPageCount() - 1;

  Connect(wxEVT_SET_FOCUS, wxFocusEventHandler(TabBase::onFocus));
}

void TabBase::updateTitle()
{
  book->SetPageText(tabNum, getTitle());
}

wxString TabBase::getTitle()
{
  int num = getTitleNumber();
  wxString res = tabName;
  if(num) res += wxString::Format(wxT(" (%d)"), num);
  return res;
}
