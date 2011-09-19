#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <string>
#include <fstream>
#include <stdexcept>
#include <boost/filesystem.hpp>

struct Config
{
  std::string filename;

  bool updateList, updateTigs;

  Config() : updateList(false), updateTigs(false) {}

  void fail(const std::string &msg)
  {
    throw std::runtime_error(msg);
  }

  void load(const boost::filesystem::path &where)
  {
    using namespace std;

    filename = (where/"config").string();

    if(!boost::filesystem::exists(filename))
      {
        updateList = true;
        updateTigs = true;

        write();
      }
    else
      {
        ifstream f(filename.c_str());
        string repo;
        if(f) f >> repo;

        if(repo != "" && repo != "1")
          fail("Unknown repo version. Please update.");
      }
  }

  void write()
  {
    std::ofstream of(filename.c_str());
    of << "1";
  }
};

Config conf;
#endif
