#ifndef __UNPACK_BASE_HPP
#define __UNPACK_BASE_HPP

#include <mangle/vfs/stream_factory.hpp>
#include <set>

#include "progress.hpp"

namespace Unpack
{
  /*
    How the VFS::StreamFactory is used:

    - open("filename") is called for all files in the archive. The
      returned stream is expected to be writable, except for directory
      entries. The data for each file is written to the stream, and
      the StreamPtr is dereferenced.

    - Returning an empty StreamPtr is allowed, and will cause the file
      to be skipped. This can be used to index the archive.

    - Filenames may include paths. The reciever is expected to create
      necessary parent directories.

    - Directories (paths ending with a slash) may be sent to
      open(). The StreamFactory is expected to create empty
      directories in those cases, and to return an empty StreamPtr.
      Both forward and backward slashes should be expected.

    - The StreamFactory itself is responsible for throwing exceptions
      on writing errors. The unpacker will only throw on unpacking
      errors.

    - Instead of using StreamFactory directly, you may use the
      directory version to write to a file system directory.

    About the FileList list:

    - If the 'list' parameter is specified, only the files listed
      there will be unpacked. Files in the list which do not match any
      in the archive, will cause an error.

      Names must match exactly with the files in the archive,
      including case and slash type. For that reason, it is recommened
      to ONLY use filenames that have previously been generated with
      the same unpacker.

    - If the list is missing or empty, all files are unpacked.

    - Some implementations may behave differently depending on whether
      the list is present or not. Some EXE unpackers for example will
      enter a slow "explore mode" when a list is missing, but will be
      in a much faster "unpack mode" when given the right data. In
      those cases, the list names may contain additional meta-data
      that do not really represent actual filenames.
   */
  struct UnpackBase
  {
    typedef std::set<std::string> FileList;

    // Unpack the given archive file into the stream factory.
    virtual void unpack(const std::string &file,
                        Mangle::VFS::StreamFactoryPtr output,
                        Progress *prog = NULL,
                        const FileList &list = FileList()) = 0;

    // Directory writer version
    void unpack(const std::string &file, const std::string &dir,
                Progress *prog = NULL,
                const FileList &list = FileList());

    /* Generate an index of an archive. This is the same as running
       unpack() and gathering up the names generated through the
       StreamFactory.

       An optional StreamFactoryPtr may be provided to accept output
       data. If it is empty, no data is extracted.
    */
    void index(const std::string &file, FileList &output,
               Progress *prog = NULL,
               Mangle::VFS::StreamFactoryPtr write =
               Mangle::VFS::StreamFactoryPtr());
  };
}
#endif
