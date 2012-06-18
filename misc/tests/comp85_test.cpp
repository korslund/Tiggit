#include "comp85.hpp"

#include <vector>
#include <iostream>
#include <assert.h>
#include <stdint.h>

using namespace std;

void test(const char *p, int cnt)
{
  int aguess = Comp85::calc_ascii(cnt);
  string res = Comp85::encode(p, cnt);
  int bguess = Comp85::calc_binary(res.size());
  vector<char> buf(cnt);
  int len = Comp85::decode(res, &buf[0]);

  cout << "E:" << cnt << " > (" << aguess << ")" << res.size()
       << " res='" << res << "' > " << len << endl;
  assert(len == cnt);
  assert(cnt == bguess);
  assert(aguess == res.size() || aguess == res.size()+1);
  for(int i=0; i<cnt; i++)
    assert(p[i] == buf[i]);
}

uint32_t detest(const std::string &str)
{
  int bguess = Comp85::calc_binary(str.size());
  vector<char> buf(bguess);
  int len = Comp85::decode(str, &buf[0]);
  int aguess = Comp85::calc_ascii(len);
  string res = Comp85::encode(&buf[0], bguess);

  cout << "D:'" << str << "' > " << len << " > (" << aguess << ")"
       << res.size() << " > " << res << endl;

  assert(bguess == len);
  assert(aguess == str.size() || aguess == str.size()+1);
  int last = str.size()-1;
  assert(res == str || (res == str.substr(0,last) && str[last] == '!'));

  if(bguess >= 4)
    return *((uint32_t*)&buf[0]);
  else
    return 0;
}

void test(const std::string &str)
{
  cout << str << ": ";
  test(str.c_str(), str.size());
}

void testChar(char c)
{
  char buf[10];
  for(int i=0; i<10; i++) buf[i] = c;
  for(int i=0; i<10; i++)
    test(buf, i+1);
}

void detestChar(char c)
{
  char buf[10];
  for(int i=0; i<10; i++) buf[i] = c;
  for(int i=0; i<10; i++)
    detest(string(buf, i+1));
}

// Test for expected fail cases
void defail(const std::string &str)
{
  try { detest(str); }
  catch(...)
    {
      cout << "Caught expected failure on " << str << endl;
      return;
    }
  cout << "MISSED expected failure on " << str << endl;
  assert(0);
}

void test()
{
  test("");
  test("5adadA%&kjAEKJfaBLAJd");

  testChar('a');
  testChar(20);
  testChar(' ');
  testChar(0);
  testChar(0xff);
  testChar(84);
  testChar(85);

  detest("");
  detestChar('!');
  detestChar('(');
  detestChar(')');
  assert(detest("}rFg(")==89885203);
  assert(detest("{rFg(")==89885201);
  assert(detest("}rFg)")==89885203+85*85*85*85);
  assert(detest("!3_>|")==0xffffffff);
  defail("(3_>|");
  defail("~~");
  defail("~~~");
  defail("~~~~");
  defail("~~~~|");
}

int main(int argc, char** argv)
{
  if(argc != 2)
    test();
  else
    test(argv[1]);

  return 0;
}
