#ifndef __TIGDATA_JSON_HPP_
#define __TIGDATA_JSON_HPP_

#include "tiglist.hpp"
#include <json/json.h>

namespace TigData
{
  void fromJson(TigData::TigInfo& out, const Json::Value &v);
  void fromJson(TigData::TigEntry& out, const Json::Value &v);

  struct FetchTig
  {
    /* Callback used by the tiglist reader. Is given an idname and the
       url given in the main tiglist, and is expected to return the
       tigfile as a Json value.

       May return a Null value - in that case the entry is skipped.
     */
    virtual Json::Value fetchTig(const std::string &idname,
                                 const std::string &url) = 0;
  };

  /* Load a tiglist from a Json value. Throws an exception on error.
   */
  void fromJson(TigData::TigList &out, const Json::Value &list,
                FetchTig &fetch);
}
#endif
