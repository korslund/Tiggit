#ifndef __TIGDATA_BINARY_HPP_
#define __TIGDATA_BINARY_HPP_

#include "tiglist.hpp"
#include <mangle/stream/stream.hpp>

/* THIS FILE IS NOT CURRENTLY USED.

   Binary reading/writing is an optimization, JSON is good enough for
   the time being.
 */

namespace BinLoader
{
  void readBinary(const std::string &file, TigData::TigList &out);
  void writeBinary(const std::string &file, const TigData::TigList &list);

  // Read from stream. The 'name' is only used for error messages.
  void readBinary(Mangle::Stream::StreamPtr strm, TigData::TigList &out,
                  const std::string &name = "");
  void writeBinary(Mangle::Stream::StreamPtr strm, const TigData::TigList &list,
                   const std::string &name = "");
}
#endif
