#ifndef __PICKLIST_HPP_
#define __PICKLIST_HPP_

#include "listbase.hpp"

namespace List
{
  struct Picker
  {
    virtual bool include(const void* x) = 0;
    virtual ~Picker() {}
  };

  /* A picklist is used to pick out elements from a parent list, based
     on an externally set criterium.
   */
  class PickList : public ListBase
  {
    Picker *pck;

  public:
    PickList(ListBase *par);

    // Set picker. A NULL value means 'pick everything'.
    void setPick(Picker *p = NULL);

  private:
    void updateList();
  };
}
#endif
