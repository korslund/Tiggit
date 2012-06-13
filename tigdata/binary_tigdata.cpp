#include "binary_tigdata.hpp"
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

using namespace TigData;
using namespace std;

static const int MAGIC = 0x387adba8;
static const int VERSION = 1;

template <typename T>
void wT(ostream &s, const T &t)
{ s.write((char*)&t, sizeof(T)); }

template <typename T>
void rT(istream &s, T &t)
{ s.read((char*)&t, sizeof(T)); }

void wS(ostream &s, const std::string &str)
{
  uint16_t len = str.size();
  wT(s, len);
  s.write(str.c_str(), str.size());
}

vector<char> buf;

void rS(istream &s, std::string &str)
{
  uint16_t len;
  rT(s, len);
  if(buf.size() < len)
    buf.resize(len);
  s.read(&buf[0], len);
  str.assign(&buf[0],len);
}

#include <iostream>

struct FileRead
{
  ifstream ifs;
  std::string name;

  FileRead(const std::string &fname)
    : ifs(fname.c_str()), name(fname)
  {
    if(!ifs) fail("Cannot read file");

    {
      int32_t i;
      i32(i);
      if(i != MAGIC) fail("Not a valid tigdata file");
    }
    uint16_t i;
    i16(i);
    if(i != VERSION) fail("Unsupported file version");
  }

  void fail(const std::string &msg)
  { throw std::runtime_error("Error parsing " + name + ": " + msg); }

  void s(std::string &str) { rS(ifs, str); }
  void f(float &f) { rT(ifs, f); }
  void i8(int8_t &i) { rT(ifs, i); }
  void i16(uint16_t &i) { rT(ifs, i); }
  void i32(int32_t &i) { rT(ifs, i); }
  void i64(int64_t &i) { rT(ifs, i); }
};

struct FileWrite
{
  ofstream ofs;
  std::string name;

  FileWrite(const std::string &fname)
    : ofs(fname.c_str()), name(fname)
  {
    if(!ofs) fail("Cannot write to file");

    i32(MAGIC);
    i16(VERSION);
  }

  void fail(const std::string &msg)
  { throw std::runtime_error("Error writing " + name + ": " + msg); }

  void s(const std::string &str) { wS(ofs, str); }
  void f(float f) { wT(ofs, f); }
  void i8(uint8_t i) { wT(ofs, i); }
  void i16(uint16_t i) { wT(ofs, i); }
  void i32(uint32_t i) { wT(ofs, i); }
  void i64(uint64_t i) { wT(ofs, i); }
};

void TigData::toBinary(const TigList &list, const string &file)
{
  FileWrite stat(file);

  // Write basic channel strings
  stat.s(list.channel);
  stat.s(list.desc);
  stat.s(list.homepage);

  // Write static data
  int len = list.list.size();
  stat.i32(len);
  for(int i=0; i<len; i++)
    {
      const TigEntry &e = list.list[i];
      const TigInfo &inf = e.tigInfo;

      // Static data
      stat.i64(e.addTime);
      stat.s(e.urlname);
      stat.s(inf.url);
      stat.s(inf.launch);
      stat.s(inf.version);
      stat.s(inf.title);
      stat.s(inf.desc);
      stat.s(inf.devname);
      stat.s(inf.homepage);
      stat.i8(inf.isDemo);
    }

  // Write dynamic (often changing) data at the end
  for(int i=0; i<len; i++)
    {
      const TigEntry &e = list.list[i];
      const TigInfo &inf = e.tigInfo;

      stat.f(e.rating);
      stat.i32(e.rateCount);
      stat.i32(e.dlCount);
      stat.s(inf.tags);
    }
}

void TigData::fromBinary(TigList &list, const string &file)
{
  FileRead stat(file);

  // Read basic channel strings
  stat.s(list.channel);
  stat.s(list.desc);
  stat.s(list.homepage);

  // Read main list
  int32_t len;
  stat.i32(len);
  if(len > 200000)
    stat.fail("Corrupt file");
  list.list.resize(len);
  for(int i=0; i<len; i++)
    {
      TigEntry &e = list.list[i];
      TigInfo &inf = e.tigInfo;

      // Static data
      stat.i64(e.addTime);
      stat.s(e.urlname);
      stat.s(inf.url);
      stat.s(inf.launch);
      stat.s(inf.version);
      stat.s(inf.title);
      stat.s(inf.desc);
      stat.s(inf.devname);
      stat.s(inf.homepage);
      int8_t b;
      stat.i8(b);
      inf.isDemo = b;

      // Set up generated data
      e.channel = list.channel;
      e.idname = e.channel + "/" + e.urlname;
    }

  // Read dynamic data
  for(int i=0; i<len; i++)
    {
      TigEntry &e = list.list[i];
      TigInfo &inf = e.tigInfo;

      stat.f(e.rating);
      stat.i32(e.rateCount);
      stat.i32(e.dlCount);
      stat.s(inf.tags);
    }
}
