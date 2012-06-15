#ifndef __LISTBASE_HPP_
#define __LISTBASE_HPP_

#include "parentbase.hpp"
#include <vector>

namespace List
{
  typedef std::vector<void*> PtrList;

  class ListBase : public ParentBase
  {
  public:
    ListBase(ListBase *par=NULL)
      : ParentBase(par) {}

    const PtrList &getList() const { return list; }
    void refresh() { updateChildren(); }

  protected:
    PtrList list;

    /* Used to update our own list. Will usually be called if our
       parent has changed, and for initial setup.
     */
    virtual void updateList() = 0;

    void updateChildren();
  };
}
#endif
