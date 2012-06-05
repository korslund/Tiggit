#include "unpack_base.hpp"

#include "dirwriter.hpp"

// "Convert" directory name to Mangle::StreamFactory
void Unpack::UnpackBase::unpack(const std::string &file,
                                const std::string &dir,
                                const FileList &list)
{
  using namespace Mangle::VFS;
  StreamFactoryPtr ptr(new Unpack::DirWriter(dir));
  unpack(file, ptr, list);
}
