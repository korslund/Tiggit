#ifndef __PICKLIST_HPP_
#define __PICKLIST_HPP_

#include "listbase.hpp"
#include <assert.h>

namespace List
{
  struct Picker
  {
    virtual bool include(const void* x) = 0;
  };

  /* A picklist is used to pick out elements from a parent list, based
     on an externally set criterium.
   */
  class PickList : public ListBase
  {
    Picker *pck;

  public:
    PickList(ListBase *par)
      : ListBase(par), pck(NULL)
    { assert(par); }

    // Set picker. A NULL value means 'pick everything'.
    void setPick(Picker *p = NULL)
    {
      pck = p;
      updateChildren();
    }

  private:
    void updateList()
    {
      const PtrList &par = ((ListBase*)parent)->getList();

      list.resize(0);
      for(int i=0; i<par.size(); i++)
        if((pck == NULL) || pck->include(par[i]))
          list.push_back(par[i]);
    }
  };
}
#endif
