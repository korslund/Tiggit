#ifndef __JSON_RATED_HPP_
#define __JSON_RATED_HPP_

#include <string>
#include <assert.h>
#include <iostream>
#include <map>

#include "filegetter.hpp"
#include "readjson.hpp"

/*
  This class maintains "ratings.json", the database of how the local
  user has rated various games.
 */

class JsonRated
{
  typedef std::map<std::string,int> IntMap;
  IntMap ratings;

  /* Note: at first we tried using a Json::Value directly here, but
     this crashed on exit. I suspect this is the fault of the jsoncpp
     lib, as I've experienced other crash bugs with it in certain
     (avoidable) corner cases.
   */

  void write()
  {
    Json::Value v;

    for(IntMap::iterator it = ratings.begin();
        it != ratings.end(); it++)
      {
        v[it->first] = it->second;
      }

    std::string filename = get.getPath("ratings.json");
    writeJson(filename, v);
  }

public:
  int getRating(const std::string &game)
  {
    int rate = -1;
    if(ratings.find(game) != ratings.end())
      rate = ratings[game];
    if(rate < -1 || rate > 4) rate = -1;
    return rate;
  }

  void setRating(const std::string &game, int rate)
  {
    assert(rate >= 0 && rate <= 4);
    ratings[game] = rate;
    write();
  }

  void read()
  {
    using namespace Json;

    std::string filename = get.getPath("ratings.json");

    Value v;

    try { v = readJson(filename); }
    // A missing file is normal, just ignore it.
    catch(...) {}

    // Convert json value into std::map
    Value::Members keys = v.getMemberNames();
    Value::Members::iterator it;
    for(it = keys.begin(); it != keys.end(); it++)
      {
        ratings[*it] = v[*it].asInt();
      }
  }
};

JsonRated ratings;
#endif
