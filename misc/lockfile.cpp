#include "lockfile.hpp"

#include <boost/filesystem.hpp>
#include <fstream>
#include <assert.h>

#ifdef _WIN32
#include "windows.h"
int getPID() { return GetCurrentProcessId(); }
#else
#ifdef __unix
#include <unistd.h>
int getPID() { return getpid(); }
#endif
#endif

using namespace Misc;
using namespace boost::filesystem;
using namespace std;

int LockFile::counter = 0;

bool LockFile::lock(bool force)
{
  assert(!locked);
  assert(file != "");

  int lockValue = (getPID()&0xffffff) | (ID << 24);

  // Is the resource already locked?
  if(exists(file) && !force)
    return false;

  // Write our lock value
  {
    ofstream ofs(file.c_str(), ios::binary);

    // Could we create the lock file?
    if(!ofs)
      return false;

    ofs.write((char*)&lockValue, 4);
  }

  // Confirm our lock value
  ifstream ifs(file.c_str(), ios::binary);
  if(!ifs)
    return false;

  int rval;
  ifs.read((char*)&rval, 4);

  locked = (rval == lockValue);
  return locked;
}

void LockFile::unlock()
{
  if(locked)
    {
      remove(file);
      locked = false;
    }
}
