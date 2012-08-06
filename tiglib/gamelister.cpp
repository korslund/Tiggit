#include "gamelister.hpp"
#include "sorters.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <set>
#include <map>

using namespace TigLib;

static DLSort dlSort;
static TitleSort titleSort;
static RateSort rateSort;
static DateSort dateSort;

void GameLister::sortTitle() { setSort(&titleSort); }
void GameLister::sortDate() { setSort(&dateSort); }
void GameLister::sortRating() { setSort(&rateSort); }
void GameLister::sortDownloads() { setSort(&dlSort); }

struct SearchPicker : GamePicker
{
  std::string str;

  bool include(const LiveInfo *info)
  {
    return boost::algorithm::icontains(info->ent->title, str);
  }
};

void GameLister::setSearch(const std::string &str)
{
  if(!searchPick)
    searchPick = new SearchPicker;

  if(str != "")
    {
      ((SearchPicker*)searchPick)->str = str;
      search.setPick(searchPick);
    }
  else
    search.setPick(NULL);
}

/* A list of tag synonyms makes working with tags easier, by equating
   eg. 'role-playing', 'roleplaying' and 'rpg' as the same tag for
   search purposes.

   Right now the list is hard-coded, but we might change that later.
 */
typedef std::map<std::string, std::string> StrMap;
static StrMap synonyms;

static void synSet(const std::string &canon, const std::string &tag1,
                  const std::string &tag2 = "", const std::string &tag3 = "")
{
  synonyms[tag1] = canon;
  if(tag2 != "") synonyms[tag2] = canon;
  if(tag3 != "") synonyms[tag3] = canon;
}

static void synReplace(std::string &tag)
{
  StrMap::iterator it = synonyms.find(tag);
  if(it == synonyms.end()) return;
  tag = synonyms[tag];
}

static void initSynonyms()
{
  if(synonyms.size() != 0) return;

  synSet("pnc", "point-and-click", "point-n-click");
  synSet("racing", "racer");
  synSet("rogue-like", "roguelike");
  synSet("role-playing", "roleplaying", "rpg");
  synSet("sport", "sports");
  synSet("shooting", "shooter");
  synSet("platform", "platformer");
  synSet("single", "single-player", "singleplayer");
  synSet("multi", "multi-player", "multiplayer");
}

typedef std::set<std::string> TagSet;

static void tokenize(const std::string &input, TagSet &output)
{
  using namespace boost;

  char_separator<char> sep(", ");
  tokenizer< char_separator<char> > tokens(input, sep);
  BOOST_FOREACH (std::string data, tokens)
    {
      // Convert to lower case
      boost::algorithm::to_lower(data);

      // Replaces synonyms 
      synReplace(data);

      output.insert(data);
    }
}

struct TagPicker : GamePicker
{
  TagSet searchFor;

  TagPicker() { initSynonyms(); }

  void setTags(const std::string &tags)
  {
    searchFor.clear();
    tokenize(tags, searchFor);
  }

  bool include(const LiveInfo *info)
  {
    TagSet gameTags;
    tokenize(info->ent->tags, gameTags);

    // Search for tag mismatches
    BOOST_FOREACH(const std::string &tag, searchFor)
      {
        if(gameTags.count(tag) == 0)
          return false;
      }

    // No mismatches found, must mean everything matches
    return true;
  }
};

void GameLister::setTags(const std::string &str)
{
  if(!tagPick)
    tagPick = new TagPicker;

  if(str == "")
    {
      tags.setPick(NULL);
      return;
    }

  TagPicker &tmp = *((TagPicker*)tagPick);
  tmp.setTags(str);
  tags.setPick(&tmp);
}

int GameLister::countTags(const std::string &str)
{
  List::PickList lst(&base);
  TagPicker tmp;
  tmp.setTags(str);
  lst.setPick(&tmp);

  return lst.getList().size();
}

/*
#include <iostream>

void GameLister::dumpTags()
{
  using namespace std;
  typedef map<string, int> List;
  List list;

  for(int i=0; i<size(); i++)
    {
      TagSet tags;
      tokenize(get(i).ent->tags, tags);

      BOOST_FOREACH(const std::string &s, tags)
        {
          list[s]++;
        }
    }

  BOOST_FOREACH(const List::value_type &t, list)
    {
      cout << t.first << ": " << t.second << endl;
    }
}
*/
