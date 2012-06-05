#ifndef __UNPACK_BASE_HPP
#define __UNPACK_BASE_HPP

#include <mangle/vfs/stream_factory.hpp>
#include <vector>

namespace Unpack
{
  /*
    How the VFS::StreamFactory is used:

    - open("filename") is called for all files in the archive. The
      returned stream is expected to be non-empty and writable, except
      for directory entries. The data for each file is written to the
      stream, and the StreamPtr is dereferenced.

    - Filenames may include paths. All slashes are converted to
      forward slashes (/) before being passed to open().

    - Directories (paths ending in slashes) may be sent to open(). It
      is expected to create empty directories in those cases, and to
      return an empty StreamPtr.

    - The StreamFactory itself is responsible for throwing exceptions
      on writing errors. The unpacker will throw on unpacking errors.

    Instead of using StreamFactory directly, you may use the directory
    version to write to a file system directory.
   */
  struct UnpackBase
  {
    typedef std::vector<std::string> FileList;

    // Unpack the given archive file into the stream factory. The
    // given lists contains the filenames to be unpacked. A
    // zero-length list means unpack everything.
    virtual void unpack(const std::string &file,
                        Mangle::VFS::StreamFactoryPtr output,
                        const FileList &list = FileList()) = 0;

    // Directory writer version
    void unpack(const std::string &file, const std::string &dir,
                const FileList &list = FileList());
  };
}
#endif
