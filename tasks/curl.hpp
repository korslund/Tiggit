#ifndef __CURL_HPP_
#define __CURL_HPP_

/* This module uses CURL to download a file. The data can either be
   written to a Mangle stream or directly to a file name.

   It's also possible to specify an optional progress functor.

   The class throws an exception or error.

   NOTE: that connection that hang may never time out (will hang
   forever), since CURLs timeout mechanism doesn't support multi-
   threaded applications.
 */

#include <mangle/stream/stream.hpp>
#include <stdint.h>
#include <string>

namespace cURL
{
  // Pass along a subclass of this to get progress reports.
  struct Progress
  {
    // Return true if OK, false to abort download.
    virtual bool progress(int64_t dl_total, int64_t dl_now) = 0;
  };

  // Download to a Mangle output stream
  void get(const std::string &url, Mangle::Stream::StreamPtr output,
           const std::string &useragent, Progress *prog = NULL);

  // Download directly to an output file
  void get(const std::string &url, const std::string &outfile,
           const std::string &useragent, Progress *prog = NULL);
}
#endif
