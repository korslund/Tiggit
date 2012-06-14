#ifndef __LISTBASE_HPP_
#define __LISTBASE_HPP_

#include "parentbase.hpp"
#include <vector>

namespace List
{
  class ListBase : public ParentBase
  {
  public:
    typedef std::vector<void*> PtrList;

    ListBase(ListBase *par=NULL)
      : ParentBase(par) {}

    const PtrList &getList() const { return list; }

  protected:
    PtrList list;

    /* Used to update our own list. Will usually be called if our
       parent has changed, and for initial setup.
     */
    virtual void updateList() = 0;

    void updateChildren()
    {
      updateList();
      std::set<ParentBase*>::iterator it;
      for(it = children.begin(); it != children.end(); it++)
        {
          ListBase *b = dynamic_cast<ListBase*>(*it);
          if(b)
            b->updateChildren();
        }
    }
  };
}
#endif
