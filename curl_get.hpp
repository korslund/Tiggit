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
  static size_t stream_write(void *buffer, size_t size, size_t num, void *strm)
  {
    assert(strm);
    std::ostream *str = (std::ostream*)strm;

    str->write((char*)buffer, size*num);

    if(str->fail()) return 0;
    return size*num;
  }

  // Download and write directly to the given file
  static void get(const std::string &url, const std::string &file)
  {
    std::ofstream of(file.c_str(), std::ios::binary | std::ios::out);
    get(url, of);
  }

  // Download a file and write it to the given stream
  static void get(const std::string &url, std::ostream &out)
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

    // Progress bar (not currently in use)
    /*
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, 0);
    */
 
    // Set user agent string
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
                     "MUU-updater/1.0 (linux) http://tigflow.com/muu");

    // Can also use CURLOPT_REFERER if you want to pretend to be a
    // website.

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK)
      throw std::runtime_error("Error fetching " + url);
  }
};
#endif
