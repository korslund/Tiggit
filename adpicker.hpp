#ifndef __ADPICKER_HPP_
#define __ADPICKER_HPP_

#include <string>
#include "filegetter.hpp"
#include "readjson.hpp"
#include <stdlib.h>

struct AdPicker
{
  std::string code, imgFile, text, link, image;
  int height, width;

  void setup(bool isTest)
  {
    using namespace Json;
    using namespace std;
    using namespace boost::filesystem;

    try
      {
        path pf = get.getPath("promo.json");

        if(!exists(pf) || difftime(time(0),last_write_time(pf)) > 60*10)
          pf = get.getTo("http://tiggit.net/client/promo.json",
                         "promo.json");

        Value root = readJson(pf.string());

        vector<int> choices;

        for(int i=0; i<root.size(); i++)
          {
            if(root[i]["test"].asBool())
              if(isTest)
                {
                  // In a test run, only pick the test candidate, and
                  // ignore everything else.
                  choices.resize(1);
                  choices[0] = i;
                  break;
                }
              else
                // In a non-test run, ignore test subjects
                continue;

            if(root[i]["disable"].asBool())
              continue;

            choices.push_back(i);
          }

        if(choices.size() == 0) return;

        srand(time(NULL));
        int ch = rand() % choices.size();
        Value v = root[choices[ch]];
        code = v["code"].asString();
        height = v["height"].asInt();
        width = v["width"].asInt();
        text = v["text"].asString();
        link = v["link"].asString();
        image = v["image"].asString();
        if(image == "")
          image = code;

        imgFile = get.getPath("cache/ads/" + image);

        // Dl the image if we don't already have it.
        if(!exists(imgFile))
          imgFile = get.getTo("http://tiggit.net/client/promo/" + image + ".png",
                              "cache/ads/" + image);
      }
    catch(...)
      {
        // Use this to signal that no ad was selected
        code = "";
      }
  }

  std::string getImage()
  {
    if(code != "" && imgFile != "")
      return imgFile;
    return "";
  }

  std::string getUrl() { return "http://tiggit.net/link/" + code; }
};

// More soon-to-be-gone globals
AdPicker adPicker;

#endif
