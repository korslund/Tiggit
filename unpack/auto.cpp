#include "auto.hpp"
#include "unpack_zip.hpp"

#include <stdexcept>
#include <fstream>

using namespace Unpack;
using namespace std;

/* We could generalize this stuff further, and create a Mangle::VFS
   based unpacker system with complete directory reading, instead of
   the unpack-and-dump approach we are using now.

   This would also lend itself to a plugin-based auto-detecter, where
   you inserted handlers instead of having hard-coded autodetect
   code. That would allow the code to be used even where some
   libraries are not available, and also makes it easy for the user to
   add their own unpackers.

   This isn't a priority right now.
 */

static void fail(const std::string &msg, const std::string &file)
{
  throw std::runtime_error("Error unpacking "+file+": " + msg);
}

void AutoUnpack::unpack(const std::string &file, Mangle::VFS::StreamFactoryPtr output,
                        Progress *prog, const FileList &list)
{
  UnpackBase *unp = NULL;

  // Magic number test
  {
    ifstream ifs(file.c_str());

    if(!ifs)
      fail("Cannot open file", file);

    int magic = 0;
    ifs.read((char*)&magic, 4);

    if(magic == 0x04034b50) unp = new UnpackZip;
    else if((magic & 0xffff) == 0x5a4d) // "MZ"
      fail("Cannot open EXE files yet", file);
  }

  if(!unp)
    fail("Not a known archive type", file);

  unp->unpack(file, output, prog, list);
  delete unp;
}
