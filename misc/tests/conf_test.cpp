#include "jconfig.hpp"

#include <iostream>
#include <assert.h>

using namespace std;
using namespace Misc;

void test1()
{
  cout << "Writing to fresh object:\n";
  JConfig jc;
  jc.setBool("am_i_beautiful", true);
  jc.setBool("am_i_an_umbrella", false);

  cout << "  Beautiful: " << jc.getBool("am_i_beautiful") << endl;
  cout << "  Umbrella: " << jc.getBool("am_i_an_umbrella") << endl;
  cout << "  Spork: " << jc.getBool("spork") << endl;
  cout << "  A cloud: " << jc.getBool("cloud", true) << endl;

  jc.save("_test1.conf");

  JConfig jc2("_test1.conf");
  assert(jc2.getBool("am_i_beautiful"));
}

void test2()
{
  cout << "Writing to pre-loaded object:\n";
  JConfig jc("_test2.conf");

  jc.setInt("morning flower", 33);

  cout << "  Morning: " << jc.getInt("morning flower", 19) << endl;
  cout << "  Evening: " << jc.getInt("evening evil", 19) << endl;

  JConfig jc2("_test2.conf");
  assert(jc2.getInt("morning flower") == 33);
}

struct Tmp
{
  float f;
  int i;

  Tmp() : f(0), i(0) {}
};

void test3()
{
  cout << "Testing binary data:\n";
  JConfig jc("_test3.conf");

  Tmp t1, t2;
  t1.f = 3.14;
  t1.i = 103;
  jc.setData("fval", &t1, sizeof(t1));
  jc.getData("fval", &t2, sizeof(t2));

  cout << "  f: " << t2.f << endl;
  cout << "  i: " << t2.i << endl;
}

int main()
{
  test1();
  test2();
  test3();

  return 0;
}
