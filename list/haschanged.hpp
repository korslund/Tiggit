#ifndef __HASCHANGED_HPP_
#define __HASCHANGED_HPP_

#include "listbase.hpp"

namespace List
{
  /* A simple callback class you can use to get notified when a list
     has changed. Just set the list you want to "monitor" as the
     parent.

     This class is so simple it really isn't necessary - it provides
     convenient class and member function names more than anything
     else.
   */
  class HasChanged : public ListBase
  {
  protected:
    HasChanged(ListBase *par)
      : ListBase(par) {}

    virtual void notify() = 0;

  private:
    void updateList() { notify(); }
  };
}
#endif
