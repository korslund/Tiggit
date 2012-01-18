#ifndef __READ_JSON_HPP
#define __READ_JSON_HPP

// FIXME / TODO: This is VERY UNCLEAN, and only works as long as we
// only have one cpp file. Once this changes, this has to be
// outsourced to a cpp as well.

#include <json/json.h>
#include <fstream>
#include <stdexcept>

/*
Json::Value parseJsonString(const std::string &string)
{
  using namespace Json;

  Value root;

  Reader reader;
  if(!reader.parse(string, root))
    throw std::runtime_error(reader.getFormatedErrorMessages());

  return root;
}
*/

Json::Value readJson(const std::string &file)
{
  using namespace Json;

  std::ifstream inf(file.c_str());
  if(!inf)
    throw std::runtime_error("Cannot read " + file);

  Value root;

  Reader reader;
  if(!reader.parse(inf, root))
    throw std::runtime_error(reader.getFormatedErrorMessages() + " reading " + file);

  return root;
}

#endif
