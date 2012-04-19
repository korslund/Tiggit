#include "readjson.hpp"

int main(int argc, char** argv)
{
  if(argc != 3) return 1;
  writeJson(argv[2], readJson(argv[1]));
  return 0;
}
