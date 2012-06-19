#ifndef __JCONFIG_HPP_
#define __JCONFIG_HPP_

#include <string>
#include <vector>

namespace Misc
{
  /* This struct reads and writes config to a json file.
   */
  struct JConfig
  {
    JConfig(const std::string &_file = "");
    ~JConfig();

    void load();
    void save();

    void load(const std::string &_file)
    { file = _file; load(); }
    void save(const std::string &_file)
    { file = _file; save(); }

    void setBool(const std::string &name, bool b);
    bool getBool(const std::string &name, bool def=false);

    void setInt(const std::string &name, int b);
    int getInt(const std::string &name, int def=0);

    // Store binary data encoded in a string
    void setData(const std::string &name, const void *p, size_t num);
    void getData(const std::string &name, void *p, size_t num);

    void set(const std::string &name, const std::string &value);
    std::string get(const std::string &name, const std::string &def="");

    bool has(const std::string &name);

    std::vector<std::string> getNames();

  private:
    std::string file;
    void *p;
  };
}

#endif
