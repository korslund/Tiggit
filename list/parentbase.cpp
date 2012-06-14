#include "parentbase.hpp"

using namespace List;

ParentBase::ParentBase(ParentBase *_parent)
  : parent(_parent)
{ if(parent) parent->addChild(this); }

ParentBase::~ParentBase()
{
  // Remove all references to ourselves
  if(parent)
    parent->remChild(this);
  if(children.size())
    {
      std::set<ParentBase*>::iterator it;
      for(it = children.begin(); it != children.end(); it++)
        (*it)->parent = NULL;
    }
}
