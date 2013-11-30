#ifndef __TIGLIB_SERVERAPI_HPP_
#define __TIGLIB_SERVERAPI_HPP_

#include <string>
#include <assert.h>

/* Tiggit.net server API class.

   Currently just a bunch of hard-coded URLs. May evolve as needed
   later.
 */

namespace TigLib
{
  struct ServerAPI
  {
    typedef std::string S;
    typedef const S& CS;

    static S statsURL()
    { return "http://tiggit.net/api/stats_json.php"; }

    static S spreadURL_SR0()
    { return "http://tiggit.net/client/sr0/"; }

    static S newsURL()
    { return "http://tiggit.net/api/news_json.php"; }

    static S countURL(CS urlname)
    { return "http://tiggit.net/api/count/" + urlname; }

    static S dlCountURL(CS urlname)
    { return countURL(urlname) + "&download"; }

    static S rateURL(CS urlname, int val)
    {
      assert(val >= 0 && val <= 5);
      S url = countURL(urlname) + "&rate=";
      char rateCh = '0' + val;
      url += rateCh;
      return url;
    }
  };
}

#endif
