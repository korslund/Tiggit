#include "json_tigdata.hpp"
#include <spread/misc/readjson.hpp>
#include <iostream>

using namespace std;
using namespace TigData;

struct Fetch : FetchTig
{
  Json::Value fetchTig(const std::string &idname,
                       const std::string &url)
  {
    string file = "data/" + idname + ".tig";

    cout << "Fetching " << idname << " from url=" << url << endl;
    cout << "   Using cache in " << file << endl;

    return ReadJson::readJson(file);
  }
};

Fetch fetch;

int main()
{
  TigList list;
  cout << "\nLoading data:\n";
  fromJson(list, ReadJson::readJson("data/all_games.json"), fetch);

  cout << "\nChannel: " << list.channel
       << "\nDescription: " << list.desc
       << "\nHomepage: " << list.homepage << endl;

  for(int i=0; i<list.list.size(); i++)
    {
      const TigEntry &ent = list.list[i];
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
