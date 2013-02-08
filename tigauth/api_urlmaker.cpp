#include "api_urlmaker.hpp"
#include <assert.h>

using namespace TigAuth::API;

URLMaker::URLMaker(const std::string &_baseURL)
  : baseURL(_baseURL)
{
  // Make sure the base URL is slash terminated
  assert(baseURL.size() > 0);
  if(baseURL[baseURL.size() - 1] != '/')
    baseURL += "/";
}

std::string URLMaker::make(const std::string &base, bool q)
{
  std::string res = baseURL + base;
  if(key != "")
    {
      if(q) res += '?';
      else res += '&';
      q = false;
      res += "key=" + key;
    }
  if(wantlist != "")
    {
      if(q) res += '?';
      else res += '&';
      q = false;
      res += "want=" + wantlist;
    }
  if(userid != "")
    {
      if(q) res += '?';
      else res += '&';
      q = false;
      res += "asuser=" + userid;
    }

  return res;
}

std::string URLMaker::signal(const std::string &sign)
{ return make("signal_v1.php?act="+sign, false); }

std::string URLMaker::createBrowserLink()
{ return make("browser_v1.php"); }

std::string URLMaker::createJsonLink()
{ return make("json_v1.php"); }

std::string URLMaker::createSignOutLink()
{ return signal("signout"); }
