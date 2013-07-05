#ifndef __CURL_POST_HPP_
#define __CURL_POST_HPP_

#include <string>
#include <map>

/* Send POST information to an online web form using libCURL.
 */
namespace cURL
{
  struct PostRequest
  {
    struct FileBuf
    {
      std::string filename;
      void *ptr; // NOTE: Data must be kept in memory until upload is
                 // finished.
      long size;
    };

    typedef std::map<std::string, std::string> StrMap;
    typedef std::map<std::string, FileBuf> BufMap;

    StrMap fields; // Form fields listed in fieldname-value format
    StrMap files; // Files to upload, in fieldname-filename format
    BufMap buffers; // Alternative way to specify files using memory
                    // buffers.

    /* Perform POST request. Fill out the above members before calling
       this function. All referenced data must be kept available until
       the function returns.

       The disableExpect100 value, if set, disables the "Expect:
       100-continue" header (by setting a blank "Expect:" header
       instead.) Disabling it is apparently necessary on some servers
       that do not implement HTTP 1.1 correctly.
     */
    void upload(const std::string &url, const std::string &useragent,
                bool disableExpect100=true);
  };
}

#endif
