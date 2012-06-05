#ifndef __UNPACK_AUTO_HPP
#define __UNPACK_AUTO_HPP

#include "base.hpp"

namespace Unpack
{
  /* An unpacker that auto-detects archive format.

     Currently supported formats:
     - ZIP (unpack_zip.hpp)

     Throws an error if the file is missing, or if no supported format
     was detected.

     NOTE: some formats (especially EXE unpackers) will have wildly
     different performance statistics depending on whether or not a
     FileList is included.

     It is therefore recommended that you pre-generate a FileList
     using index(), and pass it along to unpackers if possible.
   */
  struct AutoUnpack : UnpackBase
  {
    void unpack(const std::string &file,
                Mangle::VFS::StreamFactoryPtr output,
                Progress *prog = NULL,
                const FileList &list = FileList());

    using UnpackBase::unpack;
  };
}
#endif
