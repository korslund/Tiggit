#ifndef _CURL_GET_HPP_
#define _CURL_GET_HPP_

/* This class uses CURL to fetch a file from an URL and write it to a
   stream.
 */

#include <string>
#include <curl/curl.h>
#include <assert.h>
#include <stdexcept>
#include <fstream>

struct CurlGet
{
  typedef int (*ProgFunc)(void*,double,double,double,double);

  static size_t stream_write(void *buffer, size_t size, size_t num, void *strm)
  {
    assert(strm);
    std::ostream *str = (std::ostream*)strm;

    str->write((char*)buffer, size*num);

    if(str->fail()) return 0;
    return size*num;
  }

  // Download and write directly to the given file
  static void get(const std::string &url, const std::string &file,
                  ProgFunc fn = NULL, void *data = NULL)
  {
    std::ofstream of(file.c_str(), std::ios::binary | std::ios::out);
    get(url, of, fn, data);
  }

  // Download a file and write it to the given stream
  static void get(const std::string &url, std::ostream &out,
                  ProgFunc fn = NULL, void *data = NULL)
  {
    CURL *curl = curl_easy_init();
    assert(curl);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set up callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stream_write);

    // For https. Ignore security and just get the file.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    // This is required for multithreading
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    // We need to be able to follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);

    // Don't silently accept failed downloads
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    // Pass along referer information whenever we're following a
    // redirect.

    /* NOTE: around feb. 2012 sourceforge downloads started breaking -
       commmenting out this fixed it. However it might be required for
       other downloads, so let's only selectively disable it for
       sourceforge for now.
    */
    if(url.find("sourceforge") == std::string::npos)
      curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);

    // Progress reports
    if(fn)
      {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, fn);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);
      }

    // Set user agent string
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
                     "Tiggit client/1.0 - see http://tiggit.net/");

    // Can also use CURLOPT_REFERER if you want to pretend to be a
    // website.

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)
      {
        std::string msg = curl_easy_strerror(res);
        throw std::runtime_error("Error fetching " + url + ":\n" + msg);
      }
  }
};
#endif
