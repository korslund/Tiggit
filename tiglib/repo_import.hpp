#ifndef __TIGLIB_REPOIMPORT_HPP_
#define __TIGLIB_REPOIMPORT_HPP_

#include <spread/job/jobinfo.hpp>

namespace TigLibInt
{
  /* Import a repository from 'input' and store the result in
     'output'. Will merge with any existing repository data, but will
     only add, not overwrite, existing information and files.
   */
  Spread::JobInfoPtr importRepo(const std::string &input, const std::string &output,
                                bool async = true);
}

#endif
