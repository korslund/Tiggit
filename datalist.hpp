#ifndef _DATALIST_HPP_
#define _DATALIST_HPP_

#include <string>
#include <vector>
#include <stdint.h>

struct DataList
{
  struct TigInfo
  {
    std::string url, launch, subdir, version, title, desc,
      shot, shot80x50, shot300x260, location, devname, homepage,
      buypage;

    float price;
    bool isDemo, hasPaypal;
  };

  struct Entry
  {
    // Time added to the channel database
    int64_t add_time;

    // This game is new (add_time newer than last list refresh.)
    bool isNew;

    // Rating and download count info
    float rating;
    int rateCount, dlCount;

    // Idname = channel/urlname
    std::string urlname, idname, name, tigurl;

    TigInfo tigInfo;

    // Links to any custom external structure connected to this
    // data. In Tiggit this is used for the GameInfo class.
    void *info;
  };

  std::vector<Entry> arr;

  void add(const std::string &urlname, const std::string &idname,
           const std::string &name, const std::string &tigurl,
           int64_t add_time, bool isNew, const TigInfo &tigInfo,
           float rating, int rateCount, int dlCount)
  {
    Entry e;
    e.urlname = urlname;
    e.idname = idname;
    e.name = name;
    e.tigurl = tigurl;
    e.add_time = add_time;
    e.isNew = isNew;
    e.tigInfo = tigInfo;
    e.info = NULL;
    e.rating = rating;
    e.rateCount = rateCount;
    e.dlCount = dlCount;

    arr.push_back(e);
  }
};

#endif
