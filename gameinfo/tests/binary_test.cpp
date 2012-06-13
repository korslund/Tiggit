#include "json_tigdata.hpp"
#include "binary_tigdata.hpp"
#include <readjson/readjson.hpp>
#include <iostream>

using namespace std;
using namespace TigData;

struct Fetch : FetchTig
{
  std::string path;

  Json::Value fetchTig(const std::string &idname,
                       const std::string &url)
  {
    string file = path + idname + ".tig";
    return ReadJson::readJson(file);
  }
};

void load(TigList &list,
          const std::string &tigfile,
          const std::string &path)
{
  Fetch fetch;
  fetch.path = path;
  if(fetch.path[path.size()-1] != '/')
    fetch.path += '/';
  fromJson(list, ReadJson::readJson(tigfile), fetch);
}

void print(TigList &list)
{
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
}

int main(int argc, char**argv)
{
  std::string binfile = "_test.bin";
  if(argc < 2 || argc > 3)
    {
      cout << "Syntax: app tiglist.json tigdir/\n";
      cout << "- or -: app binfile.dat\n";
      return 1;
    }

  if(argc == 3)
    {
      TigList list;
      load(list, argv[1], argv[2]);
      toBinary(list, binfile);
    }
  else
    binfile = argv[1];

  TigList list;
  fromBinary(list, binfile);
  print(list);

  return 0;
}
