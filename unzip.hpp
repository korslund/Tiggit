#ifndef _UNZIP_HPP
#define _UNZIP_HPP

#include <zzip/zzip.h>
#include <string>
#include <stdexcept>
#include <boost/filesystem.hpp>

struct UnZip
{
  std::string file;

  void fail(const std::string &msg)
  {
    throw std::runtime_error("Error unpacking " + file + ": " + msg);
  }

  void checkErr(int err)
  {
    if(err != ZZIP_NO_ERROR)
      fail("ZIP error");
  }

  void unpack(const std::string &_file, const std::string &where)
  {
    using namespace boost::filesystem;

    zzip_error_t err;
    ZZIP_DIR *root;

    file = _file;

    root = zzip_dir_open(file.c_str(), &err);
    checkErr(err);

    path dest_dir = where;

    ZZIP_DIRENT ent;
    while(zzip_dir_read(root, &ent))
      {
        path dest_file = dest_dir / ent.d_name;

        // Make sure the directory exists
        path dest_branch = dest_file.parent_path();

        boost::filesystem::create_directories(dest_branch);

        std::string outname = dest_file.string();

        // Is it a directory?
        char last = outname[outname.size()-1];
        if(last == '/' || last == '\\')
          // If so, skip it
          continue;

        //std::cout << "Writing " << outname << "\n";

        std::ofstream out(outname.c_str(), std::ios::binary);

        ZZIP_FILE *zf = zzip_file_open(root, ent.d_name, 0);

        if(!zf)
          {
            checkErr(zzip_error(root));
            assert(0);
          }

        while(true)
          {
            char buf[1024];
            int r = zzip_file_read(zf, buf, 1024);

            if(r<0)
              checkErr(zzip_error(root));

            out.write(buf, r);

            if(!out)
              fail("Couldn't write " + outname);

            if(r < 1024)
              break;
          }

        zzip_file_close(zf);
      }

    zzip_dir_close(root);
  }
};
#endif
