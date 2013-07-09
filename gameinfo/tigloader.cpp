#include "tigloader.hpp"

#include <misc/readjson.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <cstdlib>

using namespace GameInfo;
using namespace TigData;
using namespace boost::filesystem;

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void TigLoader::clear()
{
  lookup.clear();
  channels.clear();
}

static std::string getNext(const char* &ptr)
{
  while(*ptr == ' ') ptr++;
  const char *start = ptr;
  while(*ptr != ' ' && *ptr != 0) ptr++;
  return std::string(start, ptr-start);
}

void TigLoader::addChannel(const std::string &channel,
                           const std::string &jsonfile)
{
  // Check the channel name
  if(channel.find('\\') != std::string::npos ||
     channel.find('/') != std::string::npos)
    fail("Invalid channel name '" + channel + "' (empty or contains slashes)");

  // Does the channel already exist?
  if(getChannel(channel) != NULL)
    fail("Channel '" + channel + "' already exists!");

  /* TODO: In future versions, we may add some structure around the
     list, maybe include the channel name and other info in the file
     itself. It's important that we are able to catch and throw on
     version updates, because the client relies on exceptions to
     signal that the data is in an outdated format.
   */

  // Load the JSON data
  Json::Value root = ReadJson::readJson(jsonfile);
  if(root.isNull()) return;
  if(!root.isArray())
    fail("Invalid game data in " + jsonfile);

  // Create and set up the list
  ListPtr list(new TigList);
  list->channel = channel;

  list->list.reserve(root.size());
  for(int i=0; i<root.size(); i++)
    {
      const Json::Value &v = root[i];
      TigEntry e;

      e.launch = v["launch"].asString();
      e.title = v["title"].asString();
      e.desc = v["desc"].asString();
      e.devname = v["devname"].asString();
      e.homepage = v["homepage"].asString();
      e.tags = v["tags"].asString();
      e.urlname = v["name"].asString();

      {
        std::string libs = v["libs"].asString();
        const char *ptr = libs.c_str();
        for(;;)
          {
            std::string next = getNext(ptr);
            if(next == "") break;
            e.libs.push_back(next);
          }
      }

      e.channel = channel;
      e.idname = channel + "/" + e.urlname;
      e.flags = 0;

      if(v["is_demo"].asBool())
        e.flags |= TF_DEMO;

      e.addTime = std::atoll(v["addtime"].asString().c_str());

      /* TODO: Perform more sanitation here. The tigdata is in
         principle untrusted data.
       */
      if(e.launch == "" || e.title == "" || e.urlname == "")
        continue;

      // Add the item if it checked out
      list->list.push_back(e);
    }

  // Store the list, and add all the entries to the lookup
  channels[channel] = list;
  for(int i=0; i<list->list.size(); i++)
    {
      TigEntry *e = &list->list[i];
      lookup[e->idname] = e;
    }
}

const TigEntry* TigLoader::getGame(const std::string &idname) const
{
  Lookup::const_iterator it = lookup.find(idname);
  if(it == lookup.end())
    return NULL;
  return it->second;
}

TigEntry* TigLoader::editGame(const std::string &idname)
{
  Lookup::iterator it = lookup.find(idname);
  if(it == lookup.end())
    return NULL;
  return it->second;
}

const TigList* TigLoader::getChannel(const std::string &name) const
{
  ChanMap::const_iterator it = channels.find(name);
  if(it == channels.end())
    return NULL;
  return it->second.get();
}
