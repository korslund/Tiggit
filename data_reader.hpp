#ifndef _DATA_READER_HPP_
#define _DATA_READER_HPP_

#include "datalist.hpp"
#include <stdexcept>
#include <json/json.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include "filegetter.hpp"

/*
// True if this platform matches
bool isPlatform(const std::string &plat)
{
  bool isWin = false, isLin = false, isMac = false;

#ifdef _WIN32
  isWin = true;
#endif

#ifdef __linux__
  isLin = true;
#endif

#ifdef TARGET_OS_MAC
  isMac = true;
#endif

  // Use os.hpp (not done) for this later.

  if(isWin && (plat == "windows" || plat == "win32" || plat == "win64"))
    return true;

  if(isLin && (plat == "linuxs" || plat == "linux32" || plat == "linux64"))
    return true;

  if(isMac && (plat == "mac" || plat == "osx"))
    return true;

  return false;
}
*/

struct TigListReader
{
  std::string filename, channel, desc, location, homepage;

  void fail(const std::string &msg)
  {
    throw std::runtime_error("ERROR parsing '" + filename + "':\n\n" + msg);
  }

  // Get the tigfile, from local cache if possible
  std::string getTigFile(const std::string &name, const std::string &url)
  {
    boost::filesystem::path dir;

    dir = "tigfiles";
    dir /= channel;
    dir /= name + ".tigfile";

    return get.getCache(dir.string(), url);
  }

  bool isSupported(const std::string &tigfile)
  {
    // Support all files for now. We only support the windows version
    // at the moment, and all the games we put in currently have
    // windows versions.
    return true;
  }

  void loadData(const std::string &file, DataList &data)
  {
    using namespace Json;
    filename = file;

    Value root;

    {
      std::ifstream inf(file.c_str());
      if(!inf)
        fail("Cannot read file");

      Reader reader;
      if(!reader.parse(inf, root))
        fail(reader.getFormatedErrorMessages());
    }

    // Check file type
    if(root["type"] != "tiglist 1.0")
      fail("Not a valid tiglist");

    channel = root["channel"].asString();
    desc = root["desc"].asString();
    location = root["location"].asString();
    homepage = root["homepage"].asString();

    // This must be present, the rest are optional
    if(channel == "") fail("Missing or invalid channel name");

    // Traverse the list
    root = root["list"];

    // We have to do it this way because the jsoncpp iterators are
    // b0rked.
    Value::Members keys = root.getMemberNames();
    Value::Members::iterator it;
    for(it = keys.begin(); it != keys.end(); it++)
      {
        const std::string &key = *it;
        Value game = root[key];

        // Get the tigfile
        std::string tigf = getTigFile(key, game["tigurl"].asString());

        // Check the tigfile if it supports our platform
        if(!isSupported(tigf))
          continue;

        // Push the game into the list
        data.add(0, key,
                 game["title"].asString(), game["desc"].asString(),
                 game["fpshot"].asString(), game["tigurl"].asString(),
                 tigf);
      }
  }
};

#endif
