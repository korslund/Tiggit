#include "readjson.hpp"
#include <stdexcept>

#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include <mangle/stream/clients/io_stream.hpp>

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
using namespace Mangle::Stream;

namespace ReadJson
{

void writeJson(StreamPtr strm, const Json::Value &value, bool fast)
{
  MangleOStream of(strm);

  if(!of)
    throw std::runtime_error("Unable to open output file");

  if(fast)
    {
      Json::FastWriter w;
      of << w.write(value);
    }
  else
    of << value;
}

Json::Value readJson(StreamPtr strm)
{
  MangleIStream inf(strm);

  Json::Value root;
  Json::Reader reader;

  if(!reader.parse(inf, root))
    throw std::runtime_error(reader.getFormatedErrorMessages());

  return root;
}

void writeJson(const std::string &file, const Json::Value &value, bool fast)
{
  try
    {
      writeJson(StreamPtr(new OutFileStream(file)), value, fast);
    }
  catch(std::exception &e)
    {
      throw std::runtime_error(e.what() + (" (writing " + file + ")"));
    }
}

Json::Value readJson(const std::string &file)
{
  Json::Value res;

  try
    {
      res = readJson(StreamPtr(new FileStream(file)));
    }
  catch(std::exception &e)
    {
      throw std::runtime_error(e.what() + (" (reading " + file + ")"));
    }
  return res;
}

}
