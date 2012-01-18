#ifndef _AUTH_HPP_
#define _AUTH_HPP_

#include "readjson.hpp"

struct Auth
{
  enum UserType
    {
      UT_User,
      UT_Moderator,
      UT_Admin
    };

  // User privilege level. This has no security implications, it just
  // affects which client interfaces are exposed. The server handles
  // all actual user authentication.
  int type;

  Auth() : type(UT_User) {}

  void load()
  {
    Json::Value v;
    try
      {
        v = readJson(get.getPath("auth.json"));
      }
    // A missing auth file is acceptable
    catch(...) { return; }

    std::string ts = v["usertype"].asString();

    if(ts == "admin") type = UT_Admin;
    else if(ts == "moderator") type = UT_Moderator;
    else type = UT_User;
  }

  bool isAdmin() { return type == UT_Admin; }
};

Auth auth;
#endif
