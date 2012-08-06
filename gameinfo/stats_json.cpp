#include "stats_json.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <spread/misc/readjson.hpp>

using namespace TigData;
using namespace GameInfo;
using namespace Json;

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void Stats::fromJson(TigLoader &out, Mangle::Stream::StreamPtr strm)
{ fromJson(out, ReadJson::readJson(strm)); }

void Stats::fromJson(TigLoader &out, const std::string &file)
{ fromJson(out, ReadJson::readJson(file)); }

void Stats::fromJson(TigLoader &out, const Json::Value &root)
{
  if(root["type"] != "tigstats 1.0")
    fail("Not a valid stats list");

  std::string channel = root["channel"].asString();

  if(channel == "" || out.getChannel(channel) == NULL)
    fail("Invalid channel '" + channel + "'");

  if(channel.find('/') != std::string::npos)
    fail("Channel names may not contain slashes: " + channel);

  channel += "/";

  // Traverse the list
  Value list = root["list"];
  const Value::Members &keys = list.getMemberNames();
  for(int i=0; i<keys.size(); i++)
    {
      const std::string &key = keys[i];
      const std::string &idname = channel + key;
      TigEntry *e = out.editGame(idname);

      // Skip non-matching games
      if(e == NULL) continue;

      const Value &arr = list[key];

      if(!arr.isArray() || arr.size() != 3)
        fail("Invalid stats entry for " + idname);

      e->rating = arr[0u].asDouble();
      e->rateCount = arr[1u].asUInt();
      e->dlCount = arr[2u].asUInt();
      if(e->rating > 5 || e->rating < 0) e->rating = -1;
    }
}
