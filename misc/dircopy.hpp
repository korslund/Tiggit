#ifndef __MISC_DIRCOPY_HPP_
#define __MISC_DIRCOPY_HPP_

#include <string>

/* File system copy and move operations.

   The functions are built on boost::filesystem, but unlike their
   boost counterparts these functions can copy/move directories
   recursively, accross drives, and into directories that do not yet
   exist, without problem.

   The functions always overwrite existing data.
 */

namespace DirCopy
{
  void moveFile(const std::string &from, const std::string &to);
  void moveDir(const std::string &from, const std::string &to);
  void move(const std::string &from, const std::string &to);

  void copyFile(const std::string &from, const std::string &to);
  void copyDir(const std::string &from, const std::string &to);
  void copy(const std::string &from, const std::string &to);
}

#endif
