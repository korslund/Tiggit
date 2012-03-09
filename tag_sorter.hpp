#ifndef __TAG_SORTER_HPP_
#define __TAG_SORTER_HPP_

#include "datalist.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <boost/algorithm/string.hpp>

const char* itags[] =
  {
    "arcade",
    "puzzle",
    "action",
    "platform",
    "strategy",
    "music",
    "casual",
    "cards",
    "simulation",
    "fps",
    "fighter",
    "adventure",
    "arcade",
    "racing",
    "role-playing",
    "\0"
  };

struct TagSorter
{
  std::vector<std::vector<std::string> > allTags;

  struct Entry
  {
    std::string tag;
    std::vector<int> games;
    bool important;

    bool operator<(const Entry &other) const
    {
      if(important != other.important)
        // Always put important tags above non-important ones
        return important;

      if(games.size() == other.games.size() || important)
        // If the tags have the same frequency, sort by
        // name. Important (top-level) tags are always sorted by name.
        return boost::algorithm::ilexicographical_compare(tag, other.tag);

      // We should come before 'other' if our frequency is higher
      return games.size() > other.games.size();
    }
  };

  // Create a frequency-sorted lookup of tags from any given sub-set
  // of games
  void makeTagList(const std::vector<int> &selection,
                   std::vector<Entry> &output)
  {
    using namespace std;

    output.resize(0);

    // Create lookup of all tags first
    typedef std::map<string,vector<int> > ELookup;
    ELookup lookup;
    for(int i=0; i<selection.size(); i++)
      {
        int game = selection[i];
        assert(game >= 0 && game < allTags.size());
        vector<string> &tags = allTags[game];

        for(int t=0; t<tags.size(); t++)
          lookup[tags[t]].push_back(i);
      }

    // Next move this list over to the output
    output.resize(lookup.size());
    int index = 0;
    for(ELookup::iterator it = lookup.begin();
        it != lookup.end(); it++)
      {
        Entry &e = output[index++];
        e.tag = it->first;
        e.games = it->second;
        e.important = false;

        // Mark important tags.
        for(const char **ip = itags; **ip != 0; ip++)
          if(e.tag == *ip)
            {
              e.important = true;
              break;
            }

        if(e.important)
          e.tag[0] = toupper(e.tag[0]);
      }

    // Finally, sort the output
    sort(output.begin(), output.end());
  }

  // Set up the allTags list from base data
  void process(const DataList &data)
  {
    using namespace std;

    allTags.resize(0);
    allTags.resize(data.arr.size());

    for(int i=0; i<data.arr.size(); i++)
      {
        string s=data.arr[i].tigInfo.tags;

        vector<string> &tags = allTags[i];

        while(true)
          {
            boost::algorithm::trim(s);
            if(s == "") break;

            // Find the end of the first tag
            int end = s.find(' ');
            if(end == string::npos)
              end = s.size();

            string tag = s.substr(0,end);

            /* TODO: Do tag massaging here. This may include:
               - collapsing common synonyms (such as single-player and
                 singleplayer)
               - converting everything to lower-case
               - doing other filtering

               We should also cull repeated tags - whether they are
               repeated in the file itself or become repeated as a
               result of our filtering. The best way to handle this
               may be to simply use a set here, rather than a vector.
             */

            tags.push_back(tag);

            s = s.substr(end);
          }
        
      }
  }
};

#endif
