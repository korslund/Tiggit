#ifndef __LOCKFILE_HPP_
#define __LOCKFILE_HPP_

#include <string>

/* A lockfile is used to secure sole ownership of a resource. It's
   used to lock repositories so multiple processes won't manipulate it
   at the same time.
 */

namespace Misc
{
  struct LockFile
  {
    LockFile(const std::string &_file = "")
      : file(_file), locked(false)
    { ID = counter++; }
    ~LockFile() { unlock(); }

    /* Lock function. Returns false if the lock failed.

       In case of a stale lock file (the dead remains of a terminated
       process that failed to clean up after itself), you can attempt
       to force the lock by ignoring existing files. Use with caution.
     */
    bool lock(bool force=false);
    void unlock();
    bool lock(const std::string &file, bool force=false)
    { setFile(file); return lock(force); }

    void setFile(const std::string &_file)
    { unlock(); file = _file; }
    bool isLocked() const { return locked; }

  private:

    std::string file;
    bool locked;
    int ID;

    static int counter;
  };
}

#endif
