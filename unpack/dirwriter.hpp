#ifndef __UNPACK_DIRWRITER_HPP
#define __UNPACK_DIRWRITER_HPP

#include <mangle/vfs/stream_factory.hpp>

namespace Unpack
{
  /* A stream factory that writes files to a file system directory.
     Directories will be created as necessary. Fully implements all
     UnpackBase necessities.
  */
  struct DirWriter : Mangle::VFS::StreamFactory
  {
    DirWriter(const std::string &dir)
      : base(dir) {}

    Mangle::Stream::StreamPtr open(const std::string &name);

    std::string base;
  };
}

#endif
