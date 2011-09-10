#include <boost/filesystem.hpp>
#include <stdlib.h>

using namespace boost::filesystem;

// ARGS: install.exe base/ dest-dir new-dir exe-name
int main(int argc, char **args)
{
  if(argc != 5)
    return 1;

  path base = args[1];
  path old = base / "old";
  path dest = base / args[2];
  path src = base / args[3];
  path exe = dest / args[4];

  try
    {
      // Kill existing backup folder, if any
      if(exists(old))
        remove_all(old);

      // TODO: Wait a second to allow our caller to exit (not a very
      // robust solution, but OK for now.)

      // Take backup of our old installation, if any
      if(exists(dest))
        rename(dest, old);

      // And move the new one into place
      rename(src, dest);
    }
  catch(...)
    {
      return 2;
    }

  // TODO (unix only): set executable permission before running

  // Run the new version
  int res = system(exe.string().c_str());

  return 0;
}
