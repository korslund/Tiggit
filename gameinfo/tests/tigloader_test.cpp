#include "tigloader.hpp"
#include <iostream>

using namespace GameInfo;
using namespace std;

int main()
{
  TigLoader tig;

  tig.addChannel("data2/gamelist.json", "data2");
  tig.loadBinary("data.bin");

  const TigLoader::Lookup &list = tig.getList();
  TigLoader::Lookup::const_iterator it;
  for(it = list.begin(); it != list.end(); it++)
    {
      const TigData::TigEntry &ent = *it->second;
      cout << "\n" << ent.urlname << ":\n";
      cout << "  Title: " << ent.tigInfo.title
           << "\n  Homepage: " << ent.tigInfo.homepage
           << "\n  Tags: " << ent.tigInfo.tags
           << "\n  Channel: " << ent.channel
           << "\n  Rating: " << ent.rating << " (" << ent.rateCount << " votes)\n";
      cout << "  Demo: " << (ent.tigInfo.isDemo?"yes":"no") << endl;
    }

  return 0;
}
