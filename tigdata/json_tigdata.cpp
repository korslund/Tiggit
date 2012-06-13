#include "json_tigdata.hpp"
#include <stdlib.h>
#include <stdexcept>

using namespace TigData;

void TigData::fromJson(TigData::TigInfo& out, const Json::Value &v)
{
  out.url = v["url"].asString();
  out.launch = v["launch"].asString();
  out.version = v["version"].asString();
  out.title = v["title"].asString();
  out.desc = v["desc"].asString();
  out.devname = v["devname"].asString();
  out.homepage = v["homepage"].asString();
  out.tags = v["tags"].asString();

  // The game is a demo if it has "type":"demo" in the tigfile.
  out.isDemo = (v["type"].asString() == "demo");
}

void TigData::fromJson(TigData::TigEntry& out, const Json::Value &game)
{
  out.addTime = atoll(game["add_time"].asString().c_str());

  out.rating = game.get("rating", -1).asDouble();
  out.rateCount = game["rate_count"].asUInt();
  out.dlCount = game["downloads"].asUInt();
  if(out.rating > 5 || out.rating < 0) out.rating = -1;

  out.tigurl = game["tigurl"].asString();
}

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void TigData::fromJson(TigData::TigList &out, const Json::Value &root,
                       FetchTig &fetch)
{
  using namespace Json;

  if(root["type"] != "tiglist 1.2")
    fail("Not a valid tiglist");

  out.channel = root["channel"].asString();
  out.desc = root["desc"].asString();
  out.homepage = root["homepage"].asString();

  if(out.channel.find('/') != std::string::npos)
    fail("Channel names may not contain slashes: " + out.channel);

  // This must be present, the others are optional
  if(out.channel == "") fail("Missing or invalid channel name");

  // Traverse the list
  Value list = root["list"];

  Value::Members keys = list.getMemberNames();
  Value::Members::iterator it;
  out.list.reserve(keys.size());
  for(it = keys.begin(); it != keys.end(); it++)
    {
      const std::string &key = *it;

      // Load entry
      TigData::TigEntry e;
      fromJson(e, list[key]);

      e.urlname = key;
      e.channel = out.channel;
      e.idname = out.channel + "/" + key;

      // Get tigfile data
      Value tig = fetch.fetchTig(e.idname, e.tigurl);

      // No data? Skip.
      if(tig.isNull())
        continue;

      // Decode JSON data
      fromJson(e.tigInfo, tig);

      // Skip if not enough info was found
      if(e.tigInfo.launch == "" || e.tigInfo.title == "")
        continue;

      // Entry accepted, add it
      out.list.push_back(e);
    }
}
