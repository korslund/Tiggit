#ifndef __TIGAUTH_API_RESPONSE_HPP_
#define __TIGAUTH_API_RESPONSE_HPP_

#include <string>

namespace TigAuth
{
  namespace API
  {
    struct Response
    {
      bool isValid, isError, isAuthFail, hasUserInfo;
      std::string newKey, message, userid, usernick, userauth, useritems, generated;

      Response(const std::string &str="");
      void decode(const std::string &input);
      void clear();
    };
  }
}

#endif
