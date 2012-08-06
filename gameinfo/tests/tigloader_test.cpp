#include "tigloader.hpp"
#include <iostream>

using namespace GameInfo;
using namespace std;

int main()
{
  TigLoader tig;

  tig.addChannel("data.bin");

  const TigLoader::Lookup &list = tig.getList();
  TigLoader::Lookup::const_iterator it;
  for(it = list.begin(); it != list.end(); it++)
    {
      const TigData::TigEntry &ent = *it->second;
      cout << "\n" << ent.urlname << ":\n";
      cout << "  Title: " << ent.title
           << "\n  Homepage: " << ent.homepage
           << "\n  Tags: " << ent.tags
           << "\n  Channel: " << ent.channel
           << "\n  Rating: " << ent.rating << " (" << ent.rateCount << " votes)\n";
      cout << "  Demo: " << (ent.isDemo()?"yes":"no") << endl;
    }

  return 0;
}
