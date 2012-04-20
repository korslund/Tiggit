#ifndef __TAG_SORTER_HPP_
#define __TAG_SORTER_HPP_

#include "datalist.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
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
    "racing",
    "shooter",
    "rogue-like",
    "rpg",
    "\0"
  };

struct TagSorter
{
  std::vector<std::set<std::string> > allTags;

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
        set<string> &tags = allTags[game];

        set<string>::iterator it;
        for(it = tags.begin(); it != tags.end(); it++)
          lookup[*it].push_back(i);
      }

    // Next move this list over to the output
    //output.resize(lookup.size());
    //int index = 0;
    for(ELookup::iterator it = lookup.begin();
        it != lookup.end(); it++)
      {
        //Entry &e = output[index++];
        Entry e;
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

        /* Cleanup hack: The tag list was too big and messy, so as a
           temporary solution, cut out everything but the most
           relevant tags. This means the ones marked 'important',
           pluss single-player and multi-player (which I want to
           appear below the others and non-capitalized, that's why
           they aren't marked important.)
         */
        if(e.important || e.tag == "single-player" || e.tag == "multi-player")
          output.push_back(e);
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

        set<string> &tags = allTags[i];

        while(true)
          {
            boost::algorithm::trim(s);
            if(s == "") break;

            // Find the end of the first tag
            int end = s.find(' ');
            if(end == string::npos)
              end = s.size();

            string tag = s.substr(0,end);

            if(tag == "role-playing")
              tag = "rpg";
            else if(tag == "shooting")
              tag = "shooter";
            else if(tag == "roguelike")
              tag = "rogue-like";
            else if(tag == "single" || tag == "singleplayer")
              tag = "single-player";
            else if(tag == "multi" || tag == "multiplayer")
              tag = "multi-player";

            /* TODO:
               - add other synonyms as we need them
               - maybe make a more general system for synonyms
               - converting everything to lower-case before starting
               - possibly doing other filtering

               We should also cull repeated tags - whether they are
               repeated in the file itself or become repeated as a
               result of our filtering. The best way to handle this
               may be to simply use a set here, rather than a vector.
             */

            tags.insert(tag);

            s = s.substr(end);
          }
        
      }
  }
};

#endif
