#ifndef __JSON_RATED_HPP_
#define __JSON_RATED_HPP_

#include <string>
#include <assert.h>
#include <iostream>

#include "filegetter.hpp"
#include "readjson.hpp"

/*
  This class maintains "ratings.json", the database of how the local
  user has rated various games.
 */

class JsonRated
{
  Json::Value rated;

  void write()
  {
    std::string filename = get.getPath("ratings.json");
    writeJson(filename, rated);
  }

public:
  ~JsonRated() { rated = 0; }

  int getRating(const std::string &game)
  {
    int rate = rated.get(game, -1).asInt();
    if(rate < -1 || rate > 4) rate = -1;
    return rate;
  }

  void setRating(const std::string &game, int rate)
  {
    assert(rate >= 0 && rate <= 4);
    rated[game] = rate;
    write();
  }

  void read()
  {
    std::string filename = get.getPath("ratings.json");

    try
      {
        rated = readJson(filename);
      }
    // A missing file is normal, just ignore it.
    catch(...) {}
  }
};

JsonRated ratings;
#endif
