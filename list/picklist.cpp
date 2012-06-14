#include "picklist.hpp"
#include <assert.h>

using namespace List;

PickList::PickList(ListBase *par)
  : ListBase(par), pck(NULL)
{ assert(par); }

void PickList::setPick(Picker *p)
{
  pck = p;
  updateChildren();
}

void PickList::updateList()
{
  const PtrList &par = ((ListBase*)parent)->getList();

  list.resize(0);
  for(int i=0; i<par.size(); i++)
    if((pck == NULL) || pck->include(par[i]))
      list.push_back(par[i]);
}
