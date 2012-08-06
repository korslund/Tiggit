#ifndef __GI_TIGLOADER_HPP_
#define __GI_TIGLOADER_HPP_

#include "tiglist.hpp"
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace GameInfo
{
  class TigLoader
  {
  public:
    typedef boost::shared_ptr<TigData::TigList> ListPtr;
    typedef std::map<std::string, TigData::TigEntry*> Lookup;
    typedef std::map<std::string, ListPtr> ChanMap;

  private:
    typedef std::vector<std::string> ChanInfoList;

    Lookup lookup;
    ChanMap channels;
    ChanInfoList chanInfo;

  public:
    /* Load a channel data file. The channel name is read from the
       data itself. An exception is thrown if the name matches an
       already existing channel name, or if any other error occurs.

       Returns the channel name.
     */
    std::string addChannel(const std::string &dataFile);

    /* Save a binary file from a channel.
     */
    void saveChannel(const std::string &channel, const std::string &outfile) const;

    /* Reload all files previously added with addChannel. Assumes they
       are still in the same place.

       NOTE: will invalidate ALL pointers returned through getGame,
       getChannel and getList!
     */
    void reload();

    /* Drop all data, and remove all added lists.

       NOTE: will invalidate ALL pointers returned through the get*()
       functions!
     */
    void clear();

    /* Find a game based on idname ("channel/name"). Returns NULL if
       not found.
     */
    const TigData::TigEntry* getGame(const std::string &idname) const;

    /* Edit a game entry. Used by the stats loader. */
    TigData::TigEntry *editGame(const std::string &idname);

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
    void addList(ListPtr list, const std::string &file = "");
  };
}

#endif
