#include "listbase.hpp"

using namespace List;

void ListBase::updateChildren()
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
