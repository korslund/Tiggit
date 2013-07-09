#ifndef __WXAPP_LIBRARIES_HPP_
#define __WXAPP_LIBRARIES_HPP_

#include <spread/misc/jconfig.hpp>

namespace wxTigApp
{
  struct LibraryHandler
  {
    std::vector<std::string> titles, names, urls, rtags;
    std::map<std::string,int> tags;

    LibraryHandler(const std::string &file)
      : conf(file)
    {
      // Displayed in the menu
      titles.resize(7);
      titles[0] = "&DirectX 9.0c";
      titles[1] = "&XNA Framework 4.0";
      titles[2] = ".&NET Redist 4.5";
      titles[3] = ".N&ET Redist 3.5";
      titles[4] = "&Java Runtime";
      titles[5] = "Visual C++ Redist 201&2";
      titles[6] = "Visual C++ Redist 201&0";

      // Displayed in dialog boxes
      names.resize(7);
      names[0] = "Microsoft DirectX 9.0c";
      names[1] = "Microsoft XNA Framework 4.0";
      names[2] = "Microsoft .NET Redist 4.5";
      names[3] = "Microsoft .NET Redist 3.5";
      names[4] = "Java Runtime Environment";
      names[5] = "Visual C++ Redist 2012";
      names[6] = "Visual C++ Redist 2010";

      urls.resize(7);
      urls[0] = "http://www.microsoft.com/en-us/download/details.aspx?id=35";
      urls[1] = "http://www.microsoft.com/en-us/download/details.aspx?id=20914";
      urls[2] = "http://www.microsoft.com/en-in/download/details.aspx?id=30653";
      urls[3] = "http://www.microsoft.com/en-in/download/details.aspx?id=21";
      urls[4] = "http://java.com/en/download/index.jsp";
      urls[5] = "http://www.microsoft.com/en-in/download/details.aspx?id=30679";
      urls[6] = "http://www.microsoft.com/en-in/download/details.aspx?id=5555";

      rtags.resize(7);
      rtags[0] = "directx";
      rtags[1] = "xna";
      rtags[2] = "net4";
      rtags[3] = "net3";
      rtags[4] = "java";
      rtags[5] = "vc2012";
      rtags[6] = "vc2010";

      for(int i=0; i<rtags.size(); i++)
        tags[rtags[i]] = i;
    }

    std::string getUrl(int num)
    {
      if(num < 0 || num >= urls.size()) return "";
      return urls[num];
    }

    int getNum(const std::string &tag)
    {
      if(tags.count(tag) != 1) return -1;
      return tags[tag];
    }

    void markInstalled(const std::string &tag)
    {
      if(tags.count(tag) == 1)
        conf.setBool(tag, true);
    }

    void markInstalled(int num)
    {
      if(num >= 0 && num<rtags.size())
        markInstalled(rtags[num]);
    }

    bool isInstalled(const std::string &tag)
    {
      return conf.getBool(tag);
    }

  private:
    Misc::JConfig conf;
  };
}

#endif
