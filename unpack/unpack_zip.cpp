#include "unpack_zip.hpp"

#include <zzip/zzip.h>
#include <stdexcept>
#include <vector>

using namespace Unpack;
using namespace Mangle::Stream;

static void fail(const std::string &msg, const std::string &file)
{
  throw std::runtime_error("Error unpacking "+file+": " + msg);
}

static void checkErr(int err, const std::string &file)
{
  if(err != ZZIP_NO_ERROR)
    fail(zzip_strerror(err), file);
}

void UnpackZip::unpack(const std::string &file, Mangle::VFS::StreamFactoryPtr output,
                       Progress *prog, const FileList &list)
{
  assert(output);

  zzip_error_t err;
  ZZIP_DIR *root = zzip_dir_open(file.c_str(), &err);
  checkErr(err, file);

  // Build a directory of all the files in the archive, and count up
  // the total size
  std::vector<std::string> dir;
  int64_t total = 0, current = 0;
  {
    ZZIP_DIRENT ent;
    while(zzip_dir_read(root, &ent))
      {
        std::string name(ent.d_name);

        if(list.size())
          // Is this file on the extract list?
          if(list.count(name) == 0)
            // Nope. Skip it.
            continue;

        dir.push_back(name);
        total += ent.st_size;
      }
    checkErr(zzip_error(root), file);
  }

  if(list.size() > dir.size())
    fail("Missing files in archive", file);

  bool abort = false;

  // Update progress and check for abort status
  if(prog)
    abort = !prog->progress(total, current);

  for(int i=0; i<dir.size(); i++)
    {
      if(abort)
        break;

      std::string fname = dir[i];

      // Fetch a writable stream
      StreamPtr outs = output->open(fname);
      if(!outs) continue;
      assert(outs->isWritable);

      // Open the archive entry
      ZZIP_FILE *zf = zzip_file_open(root, fname.c_str(), 0);
      if(!zf)
        {
          checkErr(zzip_error(root), file);
          fail("Unknown ZIP error", file);
        }

      while(!abort)
        {
          char buf[10*1024];
          int r = zzip_file_read(zf, buf, 10*1024);

          if(r<0)
            checkErr(zzip_error(root), file);

          outs->write(buf, r);

          current += r;

          // Update progress and check for abort status
          if(prog)
            abort = !prog->progress(total, current);

          if(r < 1024)
            break;
        }
      zzip_file_close(zf);
    }

  zzip_dir_close(root);
}
