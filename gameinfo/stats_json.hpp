#ifndef __TIGDATA_JSON_HPP_
#define __TIGDATA_JSON_HPP_

#include "tigloader.hpp"
#include <mangle/stream/stream.hpp>
#include <json/json.h>

namespace Stats
{
  void fromJson(GameInfo::TigLoader &out, const Json::Value &list);
  void fromJson(GameInfo::TigLoader &out, Mangle::Stream::StreamPtr strm);
  void fromJson(GameInfo::TigLoader &out, const std::string &file);
}
#endif
