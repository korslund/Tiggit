#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <string>
#include <fstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "readjson.hpp"

struct Config
{
  std::string filename;

  // Set true when a forced update is necessary.
  bool updateList, updateTigs;

  // Set to true the first time we run only
  bool first_time;

  int64_t lastTime;

  Config() : updateList(false), updateTigs(false), first_time(false), lastTime(0) {}

  void fail(const std::string &msg)
  {
    throw std::runtime_error(msg);
  }

  // Called when we discover games newer than the current registered
  // time.
  void setLastTime(int64_t newTime)
  {
    if(newTime > lastTime)
      {
        lastTime = newTime;
        write();
      }
  }

  void load(const boost::filesystem::path &where)
  {
    using namespace std;

    filename = (where/"config").string();

    bool error = false;

    if(!boost::filesystem::exists(filename))
      {
        error = true;
        first_time = true;
      }
    else
      {
        string repo;
        {
          ifstream f(filename.c_str());
          if(f) f >> repo;
        }

        // Old pre-json config file
        if(repo == "" || repo == "1" || repo == "2")
          // Treat as error
          error = true;

        else
          {
            // Extract json config
            try
              {
                Json::Value root = readJson(filename);

                if(root["repo_version"] != "3")
                  error = true;

                // TODO: We are compiling without 64 bit ints, which
                // is stupid. See if we can fix this before 2038 :)
                // See also in write().

                //lastTime = root["last_time"].asInt64();
                lastTime = root["last_time"].asInt();
                if(lastTime < 0) lastTime = 0;
              }
            catch(...)
              {
                // Fail
                error = true;
              }
          }
      }

    if(error)
      {
        updateList = true;
        //updateTigs = true;
        write();
      }
  }

  void write()
  {
    Json::Value root;
    root["repo_version"] = "3";
    // TODO: Subject to 2038-bug
    root["last_time"] = (int)lastTime;

    writeJson(filename, root);
  }
};

Config conf;
#endif
