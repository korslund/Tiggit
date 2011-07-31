#include <stdlib.h>

#include "tmpdir.hpp"

struct FileGetter
{
  TmpDir *tmp;

  boost::filesystem::path base;

  FileGetter()
  {
#ifdef __linux__
    base = getenv("HOME");
    base /= ".tiggit";
#else
    base = "./";
#endif

    // Make sure the base dir exists
    boost::filesystem::create_directories(base);

    tmp = new TmpDir(base / "tmp");
  }

  ~FileGetter() { delete tmp; }

  // Copy a source file to a given position within the base
  // dir. Returns final path.
  std::string copyTo(const std::string &from, const std::string &to)
  {
    using namespace boost::filesystem;

    path dest = base / to;

    // Make sure the destination dir exists
    create_directories(dest.branch_path());

    // Do explicit check/delete, since overwrite_if_exists is buggy.
    if(exists(dest))
      remove(dest);
    copy_file(from, dest);

    return dest.string();
  }

  // Cache a given file. Returns the final path.
  std::string cache(const std::string &file)
  {
    using namespace boost::filesystem;

    path dest = "cache";
    dest /= path(file).leaf();
    return copyTo(file, dest.string());
  }

  /* Takes an input file or URL, decodes it, downloads it if necessary,
     then returns with the local file system file name.
  */
  std::string getFile(const std::string &from)
  {
    assert(tmp);

    DecodeURL loc(from);
    std::string file;

    if(loc.isUrl)
      {
        // Download to a temporary file
        file = tmp->get(loc.basename);
        CurlGet::get(loc.location, file);
      }
    else file = loc.location;

    return file;
  }
};

FileGetter get;
