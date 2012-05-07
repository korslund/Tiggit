MODULE: Jobs, threads, downloads and unpacking.

This module takes care of tasks that happen in background threads,
currently downloading and unpacking.

Dependencies are: CURL, zzip, boost and wx/thread.h.

Don't bother making big improvements to this code. The entire thing
will soon be replaced by the new auto-updater system, which will be
doing all this through an external library.
