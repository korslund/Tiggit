#ifndef __UNPACK_ZIP_HPP
#define __UNPACK_ZIP_HPP

#include "base.hpp"

namespace Unpack
{
  /* ZIP file unpacker.

     This implementation uses the ZZIP library.
   */
  struct UnpackZip : UnpackBase
  {
    void unpack(const std::string &file,
                Mangle::VFS::StreamFactoryPtr output,
                Progress *prog = NULL,
                const FileList &list = FileList());

    using UnpackBase::unpack;
  };
}
#endif
