#ifndef __PARENTBASE_HPP_
#define __PARENTBASE_HPP_

#include <set>

namespace List
{
  class ParentBase
  {
  protected:
    ParentBase *parent;
    std::set<ParentBase*> children;

  public:
    ParentBase(ParentBase *_parent=NULL)
      : parent(_parent)
    { if(parent) parent->addChild(this); }
    virtual ~ParentBase()
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

  private:
    void addChild(ParentBase *b) { children.insert(b); }
    void remChild(ParentBase *b) { children.erase(b); }
  };
}
#endif
