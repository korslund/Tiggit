#ifndef __CACHE_FETCHER_HPP_
#define __CACHE_FETCHER_HPP_

#include "progress_holder.hpp"
#include "downloadjob.hpp"
#include "zipjob.hpp"

// This one-shot struct (really just a lazily packaged function)
// fetches a cache zip file containing an initial setup of tigfiles
// and screenshots.
struct CacheFetcher : ProgressHolder
{
  CacheFetcher(wxApp *app)
    : ProgressHolder(app)
  {}

  void goDoItAlready()
  {
    setMsg("Downloading first time data...");

    // Start downloading the latest version
    std::string zip = get.getPath("cache.zip");
    std::string url = "http://tiggit.net/dl/client/cache";
    DownloadJob getter(url, zip);
    getter.run();

    // Poll-loop until it's done
    while(true)
      {
        yield();
        wxMilliSleep(40);

        bool res;
        if(getter.total != 0)
          {
            // Calculate progress
            int prog = (int)(getter.current*100.0/getter.total);
            // Avoid auto-closing the window
            if(prog >= 100) prog=99;
            res = update(prog);
          }
        else
          res = pulse();

        // Did the user click 'Cancel'?
        if(!res)
          // Abort download thread
          getter.abort();

        // Did we finish, one way or another?
        if(getter.isFinished())
          break;
      }

    // If something went wrong, just forget about it. The cache file
    // is only an optimization, not critically important.
    if(getter.isNonSuccess())
      return;

    // Download complete! Start unpacking

    setMsg("Unpacking cache...");
    ZipJob install(zip, get.base.string());
    install.run();

    // Do another semi-busy loop
    while(true)
      {
        yield();
        wxMilliSleep(40);

        // Disabled this because it looks like crap on windows. On
        // linux/gtk it works exactly like it should though.
        //pulse();

        // Exit when done
        if(install.isFinished())
          break;
      }
  }
};

#endif
