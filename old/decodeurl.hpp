#ifndef _DECODEURL_H
#define _DECODEURL_H

#include <string>

struct DecodeURL
{
  std::string location;
  bool isUrl;

  // Base file name, only set for URLs
  std::string basename;

  DecodeURL(const std::string &url)
  {
    // Assume a filename by default
    isUrl = false;
    location = url;

    // Does the url contain :// somewhere?
    if(url.find("://") != std::string::npos)
      {
        // Does the string start with file://?
        if(url.substr(0,7) == "file://")
          {
            // Yes. Cut off the file:// part
            location = url.substr(7);
          }
        else
          {
            // This is a URL, not a filename.
            isUrl = true;

            // If so, find the base name
            int pos = url.rfind('/');
            if(pos != std::string::npos)
              basename = url.substr(pos+1);

            // If that failed, go to plan b
            if(basename == "" ||
               basename.find('?') != std::string::npos)
              basename = "tmp.out";
          }
      }
  }
};

#endif
