#ifndef __COMP85_HPP_
#define __COMP85_HPP_

#include <string>

namespace Comp85
{
  // Buffer version
  int encode(const void *in, int in_size, char *out);
  int decode(const char *in, int in_size, void *out);

  // String version
  std::string encode(const void *binary, int count);
  int decode(const std::string &ascii, void *out);

  // Precalculate allocation length
  int calc_binary(int ascii_length); // Exact
  int calc_ascii(int binary_length); // May overshoot by 1 byte
}
#endif
