#include "readjson.hpp"

using namespace ReadJson;

#include <iostream>
using namespace std;

int main()
{
  Json::Value v = readJson("test.json");
  writeJson("test.json", v);
  cout << v;
}
