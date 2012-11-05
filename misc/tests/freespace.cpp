#include "freespace.hpp"

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
  if(argc != 2)
    {
      cout << "freespace <path>\n\n";
      return 1;
    }
  int64_t free, total;

  Misc::getDiskSpace(argv[1], free, total);
  cout << "Free: " << free << "\nTotal: " << total << endl;

  return 0;
}
