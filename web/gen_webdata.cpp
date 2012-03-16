#include <iostream>
#include "filegetter.hpp"
#include "data_reader.hpp"
#include "readjson.hpp"
#include "tag_sorter.hpp"

using namespace std;

/* This program isn't intended to by run by anyone but me at the
   moment, so I have hard-coded my own paths. This stop-gap system
   will likely be entirely replaced at some point in the near future
   anyway.
 */
const string basedir = "/home/mortennk/.tiggit/";
const bool writeFast = true;

DataList data;
TigListReader reader;

struct SortBase
{
  DataList &data;
  SortBase(DataList &d) : data(d) {}

  // Return true if 'a' should come before 'b' on the list
  virtual bool isLess(DataList::Entry &a, DataList::Entry &b) = 0;

  bool operator()(int a, int b)
  {
    return isLess(data.arr[a], data.arr[b]);
  }
};

struct TitleSort : SortBase
{
  TitleSort(DataList &d) : SortBase(d) {}
  bool isLess(DataList::Entry &a, DataList::Entry &b)
  { return isLessStatic(a, b); }

  // Kludge city, but it works for now
  static bool isLessStatic(DataList::Entry &a, DataList::Entry &b)
  {
    return boost::algorithm::ilexicographical_compare(a.tigInfo.title,
                                                      b.tigInfo.title);
  }
};

struct RateSort : SortBase
{
  RateSort(DataList &d) : SortBase(d) {}
  bool isLess(DataList::Entry &a, DataList::Entry &b)
  {
    if(a.rateCount == 0) a.rating = -1;
    if(b.rateCount == 0) b.rating = -1;

    // Does 'a' have a rating? If so, sort by rating. This also
    // covers the case where 'b' has no rating (b.rating = -1).
    if(a.rating >= 0)
      {
        // Sort equal ratings by name
        if(a.rating == b.rating)
          return TitleSort::isLessStatic(a, b);
        else
          return a.rating > b.rating;
      }

    // 'a' has no rating. What about b?
    else if(b.rating >= 0)
      // If yes, then always put 'b' first.
      return false;

    // Neither has a rating. Sort by title instead.
    else return TitleSort::isLessStatic(a, b);
  }
};

void write(const std::string &file, const Json::Value &v)
{
  cout << "Writing " << file << endl;
  writeJson(file, v, writeFast);
}

int main()
{
  get.setBase(basedir);
  reader.loadData(get.getPath("all_games.json"), data);
  cout << "Loaded " << data.arr.size() << " games\n";

  Json::Value tigdata;
  for(int i=0; i<data.arr.size(); i++)
    {
      const DataList::Entry &e = data.arr[i];
      const DataList::TigInfo &t = e.tigInfo;
      Json::Value val;

      val["title"] = t.title;

      string shrt = t.desc;
      if(shrt.size() > 100)
        shrt = shrt.substr(0,100) + "...";

      val["desc"] = shrt;
      val["shot300"] = t.shot300x260;
      val["homepage"] = t.homepage;
      val["rating"] = e.rating;
      val["dev"] = t.devname;
      if(t.isDemo)
        val["isdemo"] = true;

      tigdata[e.urlname] = val;
    }


  // Create a list sorted by rating
  vector<int> games(data.arr.size());
  for(int i=0; i<games.size(); i++)
    games[i] = i;

  sort(games.begin(), games.end(), RateSort(data));

  Json::Value allByRate;
  for(int i=0; i<games.size(); i++)
    allByRate.append(data.arr[games[i]].urlname);

  // Create category lists
  TagSorter tags;
  vector<TagSorter::Entry> taglist;

  tags.process(data);
  tags.makeTagList(games, taglist);
  Json::Value tagsByRate;
  for(int i=0; i<taglist.size(); i++)
    {
      const TagSorter::Entry &e = taglist[i];

      if(!e.important) continue;

      Json::Value list;
      for(int i=0; i<e.games.size(); i++)
        list.append(data.arr[games[e.games[i]]].urlname);
      tagsByRate[e.tag] = list;
    }

  Json::Value output;
  output["gamedata"] = tigdata;
  output["all_by_rate"] = allByRate;
  output["tags_by_rate"] = tagsByRate;

  write("web_gamedata.json", output);

  return 0;
}
