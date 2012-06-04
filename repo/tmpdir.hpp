#ifndef _TMPDIR_H
#define _TMPDIR_H

#include <boost/filesystem.hpp>
#include <string>

struct TmpDir
{
  boost::filesystem::path dir;
  bool created;

  TmpDir(const boost::filesystem::path &_dir) : dir(_dir), created(false) {}
  ~TmpDir()
  {
    if(created)
      boost::filesystem::remove_all(dir);
  }

  std::string get(const std::string &file)
  {
    if(!created)
      {
        create_directory(dir);
        created = true;
      }
    return (dir / file).string();
  }
};

#endif
