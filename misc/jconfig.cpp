#include "jconfig.hpp"
#include "readjson/readjson.hpp"
#include "comp85.hpp"
#include <stdexcept>
#include <boost/thread/recursive_mutex.hpp>

using namespace Misc;

struct Opaque
{
  Json::Value val; 
  boost::recursive_mutex mutex;

  static Opaque* get(void*p) { return (Opaque*)p; }
};

static Json::Value &V(void*p) { return Opaque::get(p)->val; }
static boost::recursive_mutex &M(void*p) { return Opaque::get(p)->mutex; }
static void L(void*p) { M(p).lock(); }
static void U(void*p) { M(p).unlock(); }

JConfig::JConfig(const std::string &_file)
  : file(_file)
{
  p = new Opaque;
  load();
}

JConfig::~JConfig() { delete Opaque::get(p); }

/* Mutex notes:

   The mutex is intended primarily to protect the setter and getter
   functions to avoid multiple simultaneous access to the Json::Value
   or the config file.
 */

void JConfig::load()
{
  // Never fail. A missing or invalid file is OK.
  if(file == "") return;

  L(p);
  try { V(p) = ReadJson::readJson(file); }
  catch(...) {}
  U(p);
}

void JConfig::save()
{
  if(file == "")
    return;

  L(p);
  ReadJson::writeJson(file, V(p));
  U(p);
}

void JConfig::setBool(const std::string &name, bool b)
{ L(p); V(p)[name] = b; save(); U(p); }

void JConfig::setInt(const std::string &name, int i)
{ L(p); V(p)[name] = i; save(); U(p); }

void JConfig::set(const std::string &name, const std::string &value)
{ L(p); V(p)[name] = value; save(); U(p); }

static Json::Value LG(void*p, const std::string &name, const Json::Value &v)
{
  L(p);
  Json::Value res = V(p).get(name, v);
  U(p);
  return res;
}

bool JConfig::getBool(const std::string &name, bool def)
{ return LG(p,name,def).asBool(); }

int JConfig::getInt(const std::string &name, int def)
{ return LG(p,name,def).asInt(); }

std::string JConfig::get(const std::string &name, const std::string &def)
{ return LG(p,name,def).asString(); }

bool JConfig::has(const std::string &name)
{ return V(p).isMember(name); }

std::vector<std::string> JConfig::getNames()
{ return V(p).getMemberNames(); }

void JConfig::setData(const std::string &name, const void *p, size_t num)
{
  set(name, Comp85::encode(p,num));
}

void JConfig::getData(const std::string &name, void *p, size_t num)
{
  std::string val = get(name);
  if(Comp85::calc_binary(val.size()) != num)
    throw std::runtime_error("Space mismatch in field '" + name + "'");
  Comp85::decode(get(name), p);
}

