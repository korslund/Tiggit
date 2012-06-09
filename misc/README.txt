Various stand-alone utility modules:

dirfinder - Finds and stores a writable data path for the application,
            in a system-independent way. Depends on boost::filesystem,
            and standard Windows headers on Windows.

lockfile - Create and manage a resource lock in the filesystem. Used
           to prevent multiple processes from accessing a single
           repository concurrently. Depends on boost::filesystem and
           platform-dependent standard headers.

self_update - WIP. Intended as a cross-platform tool for helping in
              updating the current running process executable.
              Dependencies are not yet clear.
