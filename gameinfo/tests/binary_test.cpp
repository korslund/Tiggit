#include "binary_tigfile.hpp"
#include <iostream>

using namespace std;
using namespace TigData;

void print(TigList &list)
{
  cout << "\nChannel: " << list.channel
       << "\nDescription: " << list.desc
       << "\nHomepage: " << list.homepage << endl;

  for(int i=0; i<list.list.size(); i++)
    {
      const TigEntry &ent = list.list[i];
      cout << "\n" << ent.urlname << ":\n";
      cout << "  Title: " << ent.title
           << "\n  Homepage: " << ent.homepage
           << "\n  Tags: " << ent.tags
           << "\n  Channel: " << ent.channel
           << "\n  Rating: " << ent.rating << " (" << ent.rateCount << " votes)";
      cout << "\n  Demo: " << (ent.isDemo()?"yes":"no") << endl;
    }
}

int main(int argc, char**argv)
{
  TigList list;

  BinLoader::readBinary("data.bin", list);
  print(list);
  BinLoader::writeBinary("_data.bin", list);

  return 0;
}
