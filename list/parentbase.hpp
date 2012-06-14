#ifndef __PARENTBASE_HPP_
#define __PARENTBASE_HPP_

#include <set>

#ifndef NULL
#define NULL 0
#endif

namespace List
{
  class ParentBase
  {
  protected:
    ParentBase *parent;
    std::set<ParentBase*> children;

  public:
    ParentBase(ParentBase *_parent=NULL);
    virtual ~ParentBase();

  private:
    void addChild(ParentBase *b) { children.insert(b); }
    void remChild(ParentBase *b) { children.erase(b); }
  };
}
#endif
