#include "curl_post.hpp"
#include <curl/curl.h>
#include <assert.h>
#include <stdexcept>

using namespace cURL;

/* Dummy writer that discards returned data from the server. May redo
   this later to match spread/tasks/curl.cpp if we want to use
   returned data at some point.
 */
static size_t streamWrite(void *buffer, size_t size, size_t num, void *strm)
{
  return 0;
}

void PostRequest::upload(const std::string &url, const std::string &useragent, bool disableExpect100)
{
  CURL *curl = curl_easy_init();
  assert(curl);

  curl_httppost *formpost=NULL;
  curl_httppost *lastptr=NULL;
  curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";

  for(StrMap::const_iterator it=fields.begin(); it != fields.end(); it++)
    {
      curl_formadd(&formpost, &lastptr,
                   CURLFORM_PTRNAME, it->first.c_str(),
                   CURLFORM_PTRCONTENTS, it->second.c_str(),
                   CURLFORM_END);
    }

  for(StrMap::const_iterator it=files.begin(); it != files.end(); it++)
    {
      curl_formadd(&formpost, &lastptr,
                   CURLFORM_PTRNAME, it->first.c_str(),
                   CURLFORM_FILE, it->second.c_str(),
                   CURLFORM_END);
    }

  for(BufMap::const_iterator it=buffers.begin(); it != buffers.end(); it++)
    {
      curl_formadd(&formpost, &lastptr,
                   CURLFORM_PTRNAME, it->first.c_str(),
                   CURLFORM_BUFFER, it->second.filename.c_str(),
                   CURLFORM_BUFFERPTR, it->second.ptr,
                   CURLFORM_BUFFERLENGTH, it->second.size,
                   CURLFORM_END);
    }

  if(disableExpect100)
    {
      headerlist = curl_slist_append(headerlist, buf);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    }

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

  // URL
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  // Set write function so CURL doesn't dump return data to stdout
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, streamWrite);

  // For https. Ignore security and just trust the server blindly.
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

  // This is required for multithreading
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

  // We need to be able to follow redirects
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 30);

  // Don't silently accept failed requests
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

  // Pass along referer information whenever we're following a
  // redirect.
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);

  // Set user agent string
  curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());

  CURLcode res = curl_easy_perform(curl);

  // Cleanup
  curl_easy_cleanup(curl);
  curl_formfree(formpost);
  if(headerlist)
    curl_slist_free_all(headerlist);

  if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)
    {
      std::string msg = curl_easy_strerror(res);
      throw std::runtime_error("Error fetching " + url + ":\n" + msg);
    }
}
