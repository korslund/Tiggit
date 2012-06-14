#include <iostream>
#include "picklist.hpp"
#include "sortlist.hpp"
#include "mainlist.hpp"
#include "haschanged.hpp"

using namespace std;
using namespace List;

int ii(const void*p) { return *((int*)p); }
void *vp(int i)
{
  int *ip = new int;
  *ip = i;
  return ip;
}

void print(const ListBase &l)
{
  const PtrList &lst = l.getList();
  for(int i=0; i<lst.size(); i++)
    cout << " " << ii(lst[i]);
  cout << endl;
}

void print(const std::string &msg, const ListBase &l)
{
  cout << msg << ":";
  print(l);
}

struct PrintChange : HasChanged
{
  std::string what;

  PrintChange(ListBase *l, const std::string &w)
    : HasChanged(l), what(w) {}

  void notify()
  {
    cout << what << " was updated\n";
  }
};

struct TestSort : Sorter
{
  bool isLess(const void *a, const void *b)
  {
    int A = ii(a), B = ii(b);

    if(A == 8 && B != 8) return true;
    if(B == 8) return false;

    if(A == 13 && B != 13) return false;
    if(B == 13) return true;

    return A < B;
  }
} tsort;

struct IntPicker : Picker
{
  bool include(const void *p) { return include(ii(p)); }
  virtual bool include(int i) = 0;
};

struct EvenPick : IntPicker
{
  bool include(int i)
  { return (i % 2) == 0; }
} even;

struct Pick5 : IntPicker
{
  bool include(int i)
  { return i > 5; }
} pick5;

int main()
{
  MainList ml;
  PickList pl(&ml);
  SortList sl(&pl);
  pl.setPick(&even);

  PrintChange p1(&ml, "Main list");
  PrintChange p2(&pl, "Pick list");
  PrintChange p3(&sl, "Sort list");

  cout << "Initial values:\n";
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);

  cout << "\nFilling in main list:\n";

  PtrList &lst = ml.fillList();
  for(int i=1;i<=10;i++)
    {
      lst.push_back(vp(i));
    }
  ml.done();
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);

  cout << "\nReversing sort:\n";
  sl.setReverse(true);
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);

  cout << "\nAdding some new elements:\n";
  lst.push_back(vp(111));
  lst.push_back(vp(200));
  lst[6] = vp(13);
  ml.done();
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);

  cout << "\nChanging sorter:\n";
  sl.setSort(&tsort);
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);

  cout << "\nChanging picker:\n";
  pl.setPick(&pick5);
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);

  cout << "\nReversing sort again:\n";
  sl.setReverse(true);
  print("Main", ml);
  print("Picker", pl);
  print("Sorter", sl);
  return 0;
}
