#ifndef __READ_JSON_HPP
#define __READ_JSON_HPP

#include <json/json.h>
#include <string>
#include <mangle/stream/stream.hpp>

namespace ReadJson
{
  void writeJson(const std::string &file, const Json::Value &value, bool fast=false);
  Json::Value readJson(const std::string &file);

  void writeJson(Mangle::Stream::StreamPtr strm, const Json::Value &value, bool fast=false);
  Json::Value readJson(Mangle::Stream::StreamPtr strm);
}

#endif
