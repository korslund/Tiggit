#ifndef __TIGDATA_BINARY_HPP_
#define __TIGDATA_BINARY_HPP_

#include "tiglist.hpp"

namespace TigData
{
  void fromBinary(TigData::TigList &out, const std::string &file);
  void toBinary(const TigData::TigList &list, const std::string &file);
}
#endif
