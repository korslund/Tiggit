#include "dircopy.hpp"

#include <boost/filesystem.hpp>
#include <assert.h>
#include <stdexcept>

using namespace boost::filesystem;

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

static void process(const std::string &from, const std::string &to,
                    path &dest, bool copy, bool file)
{
  dest = to;

  std::string name;
  if(copy) name = "copy ";
  else name = "move ";

  if(!exists(from))
    fail("Cannot " + name + from + ": No such file or directory");

  if(file && !is_regular_file(from))
    fail("Cannot " + name + from + ": Not a regular file");
  if(!file && !is_directory(from))
    fail("Cannot " + name + from + ": Not a directory");

  if(exists(dest))
    remove_all(dest);

  create_directories(dest.parent_path());
}

void DirCopy::copyFile(const std::string &from, const std::string &to)
{
  path dest;
  process(from, to, dest, true, true);
  copy_file(from, dest);
}

void DirCopy::moveFile(const std::string &from, const std::string &to)
{
  path dest;
  process(from, to, dest, false, true);

  // Attempt a clean rename. This will fail on some OSes if moving
  // across roots (from C: to D: on Windows for example.)
  try { rename(from, dest); }
  catch(...)
    {
      // If that failed, try copying instead
      copy_file(from, dest);
      remove(from);
    }
}

void DirCopy::moveDir(const std::string &from, const std::string &to)
{
  path dest;
  process(from, to, dest, false, false);

  // Ditto the comments in moveFile
  try { rename(from, dest); }
  catch(...)
    {
      copyDir(from, dest.string());
      remove_all(from);
    }
}

void DirCopy::copy(const std::string &from, const std::string &to)
{
  if(is_directory(from))
    copyDir(from, to);
  else
    copyFile(from, to);
}

void DirCopy::move(const std::string &from, const std::string &to)
{
  if(is_directory(from))
    moveDir(from, to);
  else
    moveFile(from, to);
}

// HACK to make sure a path is not slash terminated
static std::string killSlash(const path &pt)
{
  return (pt/"tmp").parent_path().string();
}

void DirCopy::copyDir(const std::string &from, const std::string &to)
{
  path dest;
  process(from, to, dest, true, false);

  // Base path. See comment below.
  std::string base = killSlash(from);
  int curlen = base.size() + 1;

  // Recurse the directory and copy files
  recursive_directory_iterator iter(from), end;
  for(; iter != end; ++iter)
    {
      path p = iter->path();
      if(!is_regular_file(p)) continue;

      /* Remove base path from the name, retaining only the local
         path. Example:

         p = c:\path\to\dir\some-file\in-here\somewhere.txt
         base = c:\path\to\dir
           =>
         local = some-file\in-here\somewhere.txt
      */
      std::string local = p.string().substr(curlen);
      path outfile = dest/local;
      copyFile(p.string(), outfile.string());
    }
}
