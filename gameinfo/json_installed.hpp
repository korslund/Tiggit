#ifndef __JSON_INSTALLED_HPP_
#define __JSON_INSTALLED_HPP_

#include <json/json.h>
#include <fstream>
#include <string>
#include <stdexcept>
#include <assert.h>
#include <iostream>

#include "datalist.hpp"
#include "filegetter.hpp"
#include "gameinfo.hpp"

/*
  This class is responsible for reading and writing the
  "installed.json" config file, which stores the list of installed
  games.
 */

struct JsonInstalled
{
  // Write the config file based on the given data list
  void write(DataList &data)
  {
    std::string filename = get.getPath("installed.json");

    std::ofstream of(filename.c_str());
    if(!of)
      {
        std::cout << "Unable to open " << filename << " for writing\n";
        return;
      }

    Json::Value root;

    // Search the data list for installed games
    for(int i=0; i<data.arr.size(); i++)
      {
        GameInfo &g = GameInfo::conv(data.arr[i]);

        if(g.isInstalled())
          root[g.entry.idname] =
            // TODO: This isn't correct - this is the latest version,
            // not necessarily the installed version. But we'll fix
            // all this later.
            g.entry.tigInfo.version;
      }

    of << root;
  }

  void read(DataList &data)
  {
    using namespace Json;

    Value root;
    {
      std::string filename = get.getPath("installed.json");

      std::ifstream inf(filename.c_str());
      // A missing file is normal, just ignore it.
      if(!inf) return;

      Reader reader;
      if(!reader.parse(inf, root))
        {
          std::cout << "ERROR parsing " << filename << ": "
                    << reader.getFormatedErrorMessages() << "\n";
          return;
        }
    }

    // Parse entries
    Value::Members keys = root.getMemberNames();
    Value::Members::iterator it;
    for(it = keys.begin(); it != keys.end(); it++)
      {
        const std::string &game = *it;
        std::string version = root[game].asString();

        // Find the game in the data list
        for(int i=0; i<data.arr.size(); i++)
          {
            DataList::Entry &e = data.arr[i];

            if(game != e.idname)
              continue;

            // Match. Set status to installed.
            GameInfo::conv(e).setInstalled();

            // TODO: We ignore the version for now.
            break;
          }
      }
  }
};
#endif
