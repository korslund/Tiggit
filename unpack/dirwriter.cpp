#include "dirwriter.hpp"

#include <boost/filesystem.hpp>
#include <stdexcept>
#include <mangle/stream/servers/outfile_stream.hpp>

using namespace boost::filesystem;
using namespace Mangle::Stream;

static void fail(const std::string &where, const std::string &msg)
{
  throw std::runtime_error("Error writing to " + where + ": " + msg);
}

StreamPtr Unpack::DirWriter::open(const std::string &name)
{
  if(name == "")
    fail(base, "Empty filename specified");

  path file(base);
  file /= name;

  // Is this a directory?
  char last = name[name.size()-1];
  if(last == '/' || last == '\\')
    {
      // If so, just create it and exit
      create_directories(file);
      return StreamPtr();
    }

  // Create parent directory
  create_directories(file.parent_path());

  // Create and return the file stream
  return StreamPtr(new OutFileStream(file.string()));
}
