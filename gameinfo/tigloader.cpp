#include "tigloader.hpp"

#include "binary_tigfile.hpp"
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace GameInfo;
using namespace TigData;
using namespace boost::filesystem;

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

std::string TigLoader::addChannel(const std::string &binfile)
{
  assert(binfile != "");

  ListPtr list(new TigList);

  BinLoader::readBinary(binfile, *list);
  addList(list, binfile);
}

void TigLoader::saveChannel(const std::string &channel, const std::string &outfile) const
{
  const TigList *list = getChannel(channel);
  if(list == NULL)
    fail("Error writing " + outfile + ": Channel not found: " + channel);

  create_directories(path(outfile).parent_path());
  BinLoader::writeBinary(outfile, *list);
}

void TigLoader::addList(ListPtr list, const std::string &binFile)
{
  // Get and check the channel name
  const std::string &chan = list->channel;
  if(chan == "")
    fail("Invalid (empty) channel name in " + binFile);

  // Does the channel already exist?
  if(getChannel(chan) != NULL)
    fail("Channel '" + chan + "' already exists! (reading " + binFile + ")");

  // Add channel to our lists
  chanInfo.push_back(binFile);
  channels[chan] = list;

  // Add all the entries to the lookup
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

void TigLoader::reload()
{
  dumpData();

  // Add channels back
  for(int i=0; i<chanInfo.size(); i++)
    addChannel(chanInfo[i]);
}

void TigLoader::clear()
{
  dumpData();
  chanInfo.clear();
}

/* Dumps all data except the chanInfo list. That list is used by
   reload() to reload data from source files.
 */
void TigLoader::dumpData()
{
  channels.clear();
  lookup.clear();
}
