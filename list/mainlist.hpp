#ifndef __MAINLIST_HPP_
#define __MAINLIST_HPP_

#include "listbase.hpp"

namespace List
{
  /* A main list gets all its elements from an external list. It has
     no parent, but is used as parent for the other list classes.
   */
  class MainList : public ListBase
  {
  public:
    // Use this to set up the list
    PtrList &fillList() { return list; }

    // Then call this when the list is complete, if you want children
    // to be automatically updated.
    void done() { updateChildren(); }

  private:
    void updateList() {}
  };
}
#endif
