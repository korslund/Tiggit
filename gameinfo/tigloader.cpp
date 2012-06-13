#include "tigloader.hpp"

#include "json_tigdata.hpp"
#include "binary_tigdata.hpp"
#include "readjson/readjson.hpp"
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace GameInfo;
using namespace TigData;
using namespace boost::filesystem;

struct MyFetcher : FetchTig
{
  URLManager *urlm;
  path dir;

  Json::Value fetchTig(const std::string &idname,
                       const std::string &url)
  {
    std::string file = (dir / (idname + ".tig")).string();

    try
      {
        // Try fetching the file if it doesn't exist
        if(urlm && !exists(file))
          {
            create_directories(dir);
            urlm->getUrl(url, file);
          }

        if(exists(file))
          return ReadJson::readJson(file);
      }
    // On error, return an empty value. The tigloader will skip this
    // entry.
    catch(...) {}
    return Json::Value();
  }
};

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

// A simple scope guard class
template <class X>
struct Killer
{
  X *ptr;
  Killer(X *x) : ptr(x) {}
  ~Killer() { if(ptr) delete ptr; }
  void done() { ptr = NULL; }
};

void TigLoader::addChannel(const std::string &listFile,
                           const std::string &tigDir,
                           URLManager *urlm)
{
  if(listFile == "") fail("Missing tiglist file");
  if(tigDir == "") fail("Missing tigfile directory");

  MyFetcher myf;
  myf.dir = tigDir;
  myf.urlm = urlm;

  TigList *list = new TigList;
  Killer<TigList> k(list);

  // Load everything from data
  fromJson(*list, ReadJson::readJson(listFile), myf);
  addList(list, listFile, tigDir);

  // Everything is ok, don't delete the object
  k.done();
}

void TigLoader::loadBinary(const std::string &binfile)
{
  if(binfile == "") fail("Missing binary list filename");

  TigList *list = new TigList;
  Killer<TigList> k(list);

  fromBinary(*list, binfile);
  addList(list, binfile);

  // Everything is ok, don't delete the object
  k.done();
}

void TigLoader::saveBinary(const std::string &channel, const std::string &outfile) const
{
  const TigList *list = getChannel(channel);
  if(list == NULL)
    fail("Channel not found: " + channel);

  create_directories(path(outfile).parent_path());
  toBinary(*list, outfile);
}

void TigLoader::addList(TigList *list, const std::string &param1, const std::string &param2)
{
  // Get and check the channel name
  std::string chan = list->channel;
  if(chan == "")
    fail("Invalid (empty) channel name");

  // Does the channel already exist?
  if(getChannel(chan) != NULL)
    fail("Channel '" + chan + "' already exists!");

  // Channel checks out. Add it to our lists.
  chanInfo.push_back(ChanInfo(param1, param2));
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

const TigList* TigLoader::getChannel(const std::string &name) const
{
  ChanMap::const_iterator it = channels.find(name);
  if(it == channels.end())
    return NULL;
  return it->second;
}

void TigLoader::reload()
{
  dumpData();

  // Add channels back
  for(int i=0; i<chanInfo.size(); i++)
    {
      const ChanInfo &ci = chanInfo[i];
      if(ci.second != "")
        addChannel(ci.first, ci.second);
      else
        // Single-string entries are used to represent binary files
        loadBinary(ci.first);
    }
}

void TigLoader::clear()
{
  dumpData();
  chanInfo.clear();
}

/* Dumps all data except the chanInfo list. The list may be used to
   reload data from source.
 */
void TigLoader::dumpData()
{
  // Delete all allocated TigLists.
  ChanMap::iterator it;
  for(it = channels.begin(); it != channels.end(); it++)
    delete it->second;

  // Kill the lists referencing the deleted data
  channels.clear();
  lookup.clear();
}
