#ifndef _UNZIP_HPP
#define _UNZIP_HPP

#include <string>

class UnZip
{
  std::string file;
  void fail(const std::string &msg);
  void checkErr(int err);

public:

  void unpack(const std::string &_file, const std::string &where);
};
#endif
