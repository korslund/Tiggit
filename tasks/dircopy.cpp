#include "dircopy.hpp"

#include <boost/filesystem.hpp>
#include <assert.h>
#include <iostream>
#include <utility> // for std::pair

using namespace boost::filesystem;
using namespace Tasks;
using namespace std;

// HACK to make sure a path is not slash terminated
static string killSlash(const path &pt)
{
  return (pt/"tmp").parent_path().string();
}

typedef list<string> StrList;
typedef pair<path,path> Pair;
typedef list<Pair> PairList;

void DirCopyTask::doJob()
{
  assert(dest != "");
  StrList::iterator it;

  PairList files;

  for(it = sources.begin(); it != sources.end(); it++)
    {
      path s = *it;

      if(!exists(s))
        {
          setError("File or directory not found: " + s.string());
          return;
        }

      s = killSlash(s);

      // Get destination path
      path dpath = dest;
      dpath /= s.filename();

      if(is_directory(s))
        {
          // Length of path + slash
          int curlen = s.string().size() + 1;

          recursive_directory_iterator iter(s), end;
          for(; iter != end; ++iter)
            {
              path p = iter->path();
              if(!is_regular_file(p)) continue;

              /* Remove base path from the name, retaining only the
                 local path. Example:

                 s = c:\path\to\dir
                 p = c:\path\to\dir\some-file\in-here\somewhere.txt
                 local = some-file\in-here\somewhere.txt

                 dest = c:\new\path\
                 dpath = c:\new\path\dir

                 result = c:\new\path\dir\some-file\in-here\somewhere.txt
               */
              std::string local = p.string().substr(curlen);

              files.push_back(Pair(p, dpath/local));
            }
        }
      else
        if(is_regular_file(s))
          files.push_back(Pair(s, dpath));
    }

  info->total = files.size();
  info->current = 0;

  PairList::iterator pit;
  for(pit = files.begin(); pit != files.end(); pit++)
    {
      const path &src = pit->first;
      const path &dst = pit->second;

      try
        {
          create_directories(dst.parent_path());
          if(exists(dst)) remove(dst);
          copy_file(src, dst);
        }
      catch(...)
        {
          setError("Failed to copy " + src.string() + " => " + dst.string());
          return;
        }
      info->current++;
    }

  if(doMove)
    {
      // Delete the sources
      for(it = sources.begin(); it != sources.end(); it++)
        try { remove_all(*it); }
      // Ignore errors. Even if for some reason we failed to delete
      // all the old files, there's no turning back. We have to return
      // success status so the client can use the new files.
        catch(...){}
    }

  setDone();
}

void DirCopyTask::cleanup()
{
  if(info->isSuccess()) return;

  // On error/abort, delete the destination directories and files
  StrList::iterator it;
  for(it = sources.begin(); it != sources.end(); it++)
    {
      path s = *it;
      if(!exists(s)) continue;
      s = killSlash(s);
      path dpath = dest;
      dpath /= s.filename();
      remove_all(dpath);
    }
}
