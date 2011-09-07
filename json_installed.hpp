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

/*
  This class is responsible for reading and writing the
  "installed.json" config file, which stores the list of installed
  games.
 */

struct JsonInstalled
{
  // Write the config file based on the given data list
  void write(const DataList &data)
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
        const DataList::Entry &e = data.arr[i];

        if(e.status == 2)
          root[e.idname.mb_str()] =
            // TODO: This isn't correct (this is the latest version,
            // not necessarily the installed version) but will fix all
            // this later.
            e.tigInfo.version;
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

            if(game != std::string(e.idname.mb_str()))
              continue;

            // Match. Set status to installed.
            e.status = 2;

            // TODO: We ignore the version for now.
            break;
          }
      }
  }
};
#endif
