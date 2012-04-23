#ifndef __TABBASE_HPP_
#define __TABBASE_HPP_

#include <wx/wx.h>
#include <wx/notebook.h>

struct TabBase : wxPanel
{
  wxNotebook *book;

  // Current tab placement in the wxNotebook. MUST be set to -1 when
  // not inserted!
  int tabNum;

  TabBase(wxNotebook *parent)
    : wxPanel(parent), book(parent), tabNum(-1)
  {}

  // Called when the tab is selected, letting the tab to direct focus
  // to a sub-element.
  virtual void takeFocus() = 0;

  // Called regularly to update information. Currently called for all
  // tabs, will soon only be called for the visible tab. Default is to
  // do nothing.
  virtual void tick() {}

  // Called whenever the root data table has changed, basically
  // instructing a complete reset on everything that depends on the
  // game database.
  virtual void dataChanged() = 0;

  // Insert this tab into the wxNotebook, if it has any content
  virtual void insertMe()
  {
    assert(book);
    book->AddPage(this, wxT(""));
    tabNum = book->GetPageCount() - 1;
  }

  // Select this tab, if it is inserted into the book. Returns true if
  // selection was successful.
  bool selectMe()
  {
    assert(book);
    if(tabNum >= 0)
      {
        book->ChangeSelection(tabNum);
        return true;
      }
    return false;
  }
};

#endif
