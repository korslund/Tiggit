#ifndef __GI_TIGLOADER_HPP_
#define __GI_TIGLOADER_HPP_

#include "tiglist.hpp"
#include <map>
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

    Lookup lookup;
    ChanMap channels;

  public:
    /* Load a channel data file. An exception is thrown if the name
       matches an already existing channel name, or if any other error
       occurs.
     */
    void addChannel(const std::string &channel,
                    const std::string &dataFile);

    // Clear away all loaded data
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
  };
}

#endif
