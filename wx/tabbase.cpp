#include "tabbase.hpp"
#include <assert.h>

using namespace wxTiggit;

TabBase::TabBase(wxNotebook *parent)
  : wxPanel(parent), book(parent)
{
  assert(book);
  book->AddPage(this, wxT(""));
  tabNum = book->GetPageCount() - 1;
}

void TabBase::updateTitle()
{
  book->SetPageText(tabNum, getTitle());
}
