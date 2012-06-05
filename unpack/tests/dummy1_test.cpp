#include "unpack_base.hpp"
#include <iostream>
#include <assert.h>

using namespace std;
using namespace Unpack;

// A dummy "unpacker" that just writes some random files
struct DummyUnpack : UnpackBase
{
  void unpack(const std::string &file,
              Mangle::VFS::StreamFactoryPtr out,
              const FileList &list = FileList())
  {
    Mangle::Stream::StreamPtr s;

    out->open("test1." + file + ".empty");
    s = out->open("test2.txt");
    assert(s);
    s->write("Hello dolly\n", 12);
    s = out->open("dir1/");
    assert(!s);
    s = out->open("dir2/test3.txt");
    s->write("This is Louis, Dolly\n", 21);
  }

  using UnpackBase::unpack;
};

int main()
{
  cout << "Unpacking some fake files:\n";
  DummyUnpack dummy;

  dummy.unpack("dummy", "_outdir");

  cout << "Done\n";
  return 0;
}
