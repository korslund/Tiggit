#include "jconfig.hpp"
#include "readjson/readjson.hpp"
#include "comp85.hpp"
#include <stdexcept>

using namespace Misc;

static Json::Value *Vp(void*p) { return (Json::Value*)p; }
static Json::Value &V(void*p) { return *Vp(p); }

JConfig::JConfig(const std::string &_file)
  : file(_file)
{
  p = new Json::Value;
  load();
}

JConfig::~JConfig() { delete Vp(p); }

void JConfig::load()
{
  // Never fail. A missing or invalid file is OK.
  if(file == "") return;
  try { V(p) = ReadJson::readJson(file); }
  catch(...) {}
}

void JConfig::save()
{
  if(file == "")
    return;

  ReadJson::writeJson(file, V(p));
}

void JConfig::setBool(const std::string &name, bool b)
{ V(p)[name] = b; save(); }

void JConfig::setInt(const std::string &name, int i)
{ V(p)[name] = i; save(); }

void JConfig::set(const std::string &name, const std::string &value)
{ V(p)[name] = value; save(); }

bool JConfig::getBool(const std::string &name, bool def)
{ return V(p).get(name, def).asBool(); }

int JConfig::getInt(const std::string &name, int def)
{ return V(p).get(name, def).asInt(); }

std::string JConfig::get(const std::string &name, const std::string &def)
{ return V(p).get(name, def).asString(); }

bool JConfig::has(const std::string &name)
{ return V(p).isMember(name); }

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

