#ifndef __GI_TIGLOADER_HPP_
#define __GI_TIGLOADER_HPP_

#include "tigdata/tiglist.hpp"
#include <map>
#include <vector>

namespace GameInfo
{
  // Callback
  struct URLManager
  {
    /* Download 'url' to the given file. Expected to throw exceptions
       on failure.
     */
    virtual void getUrl(const std::string &url, const std::string &outfile) = 0;
  };

  class TigLoader
  {
  public:
    typedef std::map<std::string, TigData::TigEntry*> Lookup;
    typedef std::map<std::string, TigData::TigList*> ChanMap;

  private:
    typedef std::pair<std::string, std::string> ChanInfo;
    typedef std::vector<ChanInfo> ChanInfoList;

    Lookup lookup;
    ChanMap channels;
    ChanInfoList chanInfo;

  public:
    ~TigLoader() { dumpData(); }

    /* Add a new channel.

       The listFile is the path to a JSON tiglist (ie. the
       all_games.json file)

       The tigDir is where to find the JSON tigfiles (ie.
       tigDir/channelname/gamename.tig), and also where newly
       downloaded tigfiles will be stored.

       The URL manager can be used to fetch missing tigfiles. If not
       provided, missing games will be skipped.
     */
    void addChannel(const std::string &listFile,
                    const std::string &tigDir,
                    URLManager *urlm = NULL);

    /* Load and save binary channel data. This replaces both the
       tiglist and the individual tigfile directory. The binary format
       is optimized for faster loading, a smaller download, and for
       small bsdiff patching.
     */
    void loadBinary(const std::string &binfile);
    void saveBinary(const std::string &channel, const std::string &outfile) const;

    /* Reload all data. Will reload all the channels already added
       with addChannel() and loadBinary(). Assumes files and
       directories are in the same place as when last added.

       NOTE: will invalidate ALL pointers returned through getGame,
       getChannel and getList!
     */
    void reload();

    /* Dump all data, and remove all added lists.

       NOTE: will invalidate ALL pointers returned through the get*()
       functions!
     */
    void clear();

    /* Find a game based on idname ("channel/name"). Returns NULL if
       not found.
     */
    const TigData::TigEntry* getGame(const std::string &idname) const;

    /* Find a channel's info. Returns NULL if not found.
     */
    const TigData::TigList* getChannel(const std::string &name) const;

    /* Return the full lookup of games
     */
    const Lookup &getList() const { return lookup; }

    /* Return full channel list
     */
    const ChanMap &getChanList() const { return channels; }

  private:
    void dumpData();
    void addList(TigData::TigList *list,
                 const std::string &param1,
                 const std::string &param2 = "");
  };
}

#endif
