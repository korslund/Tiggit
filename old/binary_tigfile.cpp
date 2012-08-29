#include "binary_tigfile.hpp"
#include <stdexcept>
#include <stdint.h>
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>

using namespace TigData;
using namespace Mangle::Stream;

static const int MAGIC = 0x387adba8;
static const int VERSION = 2;

void BinLoader::readBinary(const std::string &file, TigData::TigList &list)
{
  readBinary(FileStream::Open(file), list, file);
}

void BinLoader::writeBinary(const std::string &file, const TigData::TigList &list)
{
  writeBinary(OutFileStream::Open(file), list, file);
}

struct FileRead
{
  std::vector<char> buf;
  std::string name;
  StreamPtr st;

  void read(StreamPtr strm, TigData::TigList &list, const std::string &_name)
  {
    name = _name;
    st = strm;

    {
      uint32_t i;
      u32(i);
      if(i != MAGIC) fail("Not a valid tigdata file");
    }
    {
      uint16_t i;
      u16(i);
      if(i != VERSION) fail("Unsupported file version");
    }

    // Read basic channel strings
    s(list.channel);
    s(list.desc);
    s(list.homepage);

    // Read main list
    uint32_t len;
    u32(len);
    if(len > 200000)
      fail("Corrupt file");
    list.list.resize(len);

    for(int i=0; i<len; i++)
      {
        TigEntry &e = list.list[i];

        u64(e.addTime);
        s(e.urlname);
        s(e.launch);
        s(e.title);
        s(e.desc);
        s(e.devname);
        s(e.homepage);
        s(e.tags);
        u32(e.flags);

        // Set up generated data
        e.channel = list.channel;
        e.idname = e.channel + "/" + e.urlname;
      }
  }

  template <typename T>
  void rT(T &t) { st->read(&t, sizeof(T)); }

  void s(std::string &str)
  {
    uint16_t len;
    u16(len);
    if(buf.size() < len)
      buf.resize(len+20);
    st->read(&buf[0], len);
    str.assign(&buf[0],len);
  }

  void f(float &f) { rT(f); }
  void u16(uint16_t &i) { rT(i); }
  void u32(uint32_t &i) { rT(i); }
  void u64(uint64_t &i) { rT(i); }

  void fail(const std::string &msg)
  { throw std::runtime_error("Error parsing " + name + ": " + msg); }
};

struct FileWrite
{
  std::string name;
  StreamPtr st;

  void write(StreamPtr strm, const TigData::TigList &list, const std::string &_name)
  {
    name = _name;
    st = strm;

    u32(MAGIC);
    u16(VERSION);

    // Read basic channel strings
    s(list.channel);
    s(list.desc);
    s(list.homepage);

    // Write data
    int len = list.list.size();
    u32(len);
    for(int i=0; i<len; i++)
      {
        const TigEntry &e = list.list[i];

        u64(e.addTime);
        s(e.urlname);
        s(e.launch);
        s(e.title);
        s(e.desc);
        s(e.devname);
        s(e.homepage);
        s(e.tags);
        u32(e.flags);
      }
  }

  template <typename T>
  void wT(const T &t) { st->write(&t, sizeof(T)); }

  void s(const std::string &str)
  {
    u16(str.size());
    st->write(str.c_str(), str.size());
  }

  void f(float f) { wT(f); }
  void u16(uint16_t i) { wT(i); }
  void u32(uint32_t i) { wT(i); }
  void u64(uint64_t i) { wT(i); }

  void fail(const std::string &msg)
  { throw std::runtime_error("Error writing " + name + ": " + msg); }
};

void BinLoader::readBinary(StreamPtr strm, TigData::TigList &list,
                           const std::string &name)
{
  FileRead read;
  read.read(strm,list,name);
}

void BinLoader::writeBinary(StreamPtr strm, const TigData::TigList &list,
                            const std::string &name)
{
  FileWrite write;
  write.write(strm,list,name);
}
