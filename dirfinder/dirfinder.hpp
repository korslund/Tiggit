#ifndef __DIRFINDER_HPP_
#define __DIRFINDER_HPP_

#include <string>

/*
  This class find an appropriate writable data path for the
  application.

  It can propose platform-dependent standard paths, or accept a
  user-provided value. It will remember settings between sessions.

  We only accept directories that are writable, and we test dirs for
  writability.

  You can have multiple data directories and store independent
  settings for each, if you provide different names to the
  constructor. The default name is 'default'.

  Usage:

  A good practice is to call getStoredPath() first, and ALWAYS prompt
  the user if it returns false. The path returned by getStandardPath()
  may then be presented to the user as a default value.

  Whatever the user supplies, return it to DirFinder through
  setStoredPath().
 */

namespace DirFinder
{
  struct Finder
  {
    /* The vendor_name, app_name and dirname should be filesystem and
       windows registry compatible names. Preferably short, lower-case
       and without spaces.
     */
    Finder(const std::string &vendor_name,
           const std::string &app_name,
           const std::string &dirname = "default")
      : vname(vendor_name), aname(app_name), dname("path-" + dirname) {}

    /* Get stored path. Returns true if found and writable, false
       otherwise.

       If a path is set but the return value is false, it means the
       stored path was invalid or not writable.
     */
    bool getStoredPath(std::string &path);

    /* Get OS-dependent standard path for applications. Will only
       return true if an appropriate writable path was found.

       This does NOT change the stored path, but it may create the
       given path if it does not exist. Use setStoredPath() to use
       this path.
     */
    bool getStandardPath(std::string &path);

    /* Set stored path. Will return true if the path was accepted,
       ie. it was writable, and we were able to store the information
       in a retrievable way.

       If the path was accepted, future requests to getStoredPath()
       will return this path.
     */
    bool setStoredPath(const std::string &path);

    /* Tests if a given directory is valid and writable as a data
       path. Will create directories if they don't exist.
     */
    static bool isWritable(const std::string &path);

  private:
    std::string vname,aname,dname;
  };
}

#endif
