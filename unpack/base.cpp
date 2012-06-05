#include "base.hpp"

#include "dirwriter.hpp"

using namespace Mangle::VFS;
using namespace Mangle::Stream;
using namespace Unpack;

// "Convert" directory name to Mangle::StreamFactory
void UnpackBase::unpack(const std::string &file,
                        const std::string &dir,
                        Progress *prog,
                        const FileList &list)
{
  StreamFactoryPtr ptr(new Unpack::DirWriter(dir));
  unpack(file, ptr, prog, list);
}

struct ListMaker : StreamFactory
{
  UnpackBase::FileList *list;
  StreamFactoryPtr redir;

  StreamPtr open(const std::string &name)
  {
    list->insert(name);
    if(redir)
      return redir->open(name);
    return StreamPtr();
  }
};

void UnpackBase::index(const std::string &file,
                       FileList &output,
                       Progress *prog,
                       StreamFactoryPtr write)
{
  ListMaker *m = new ListMaker;
  m->list = &output;
  m->redir = write;
  StreamFactoryPtr mp(m);

  // This basically unpacks the file normally, and pumps all the data
  // through ListMaker. The ListMaker makes note of all the filenames
  // given, and store them in 'output'.
  unpack(file, mp, prog);
}
