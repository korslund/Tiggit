#include "curl.hpp"

#include <curl/curl.h>
#include <assert.h>
#include <stdexcept>
#include <mangle/stream/servers/outfile_stream.hpp>

using namespace cURL;
using namespace Mangle::Stream;

// Convert from CURL progress function to Progress::progress()
static int progFunc(void *p, double dl_total, double dl_now,
                    double up_total, double up_now)
{
  assert(p);
  Progress *pr = (Progress*)p;

  bool ret = pr->progress((int64_t)dl_total, (int64_t)dl_now);

  if(ret) return 0;
  return 1;
}

// Convert from CURL stream writing to Mangle::Stream
static size_t streamWrite(void *buffer, size_t size, size_t num, void *strm)
{
  assert(strm);
  Stream *s = (Stream*)strm;
  return s->write(buffer, size*num);
}

// Filename version of get() sets up a Mangle::OutFileStream and
// passes it to the Stream version.
void cURL::get(const std::string &url, const std::string &outfile,
               const std::string &useragent, Progress *prog)
{
  StreamPtr outs(new OutFileStream(outfile));
  get(url, outs, useragent, prog);
}

// Main CURL function
void cURL::get(const std::string &url, Mangle::Stream::StreamPtr output,
               const std::string &useragent, Progress *prog)
{
  /* Use this to test offline mode. Will cause all net connections to
     hang indefinitely, so it's a good test to see if you've got them
     all.
  */
  /*
  printf("Downloading %s\n", url.c_str());
  while(true) {}
  //*/

  CURL *curl = curl_easy_init();
  assert(curl);

  // URL
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  // Set up callback
  Stream *s = output.get();
  assert(s->isWritable);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, streamWrite);

  // For https. Ignore security and just get the file.
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

  // This is required for multithreading
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

  // We need to be able to follow redirects
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 30);

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
  if(prog)
    {
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progFunc);
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, prog);
    }

  // Set user agent string
  curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());

  // You could also set CURLOPT_REFERER if you want to pretend to be a
  // website.

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)
    {
      std::string msg = curl_easy_strerror(res);
      throw std::runtime_error("Error fetching " + url + ":\n" + msg);
    }
}
