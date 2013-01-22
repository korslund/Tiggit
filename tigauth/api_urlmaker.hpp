#ifndef __TIGAUTH_API_URLMAKER_HPP_
#define __TIGAUTH_API_URLMAKER_HPP_

#include <string>

namespace TigAuth
{
  namespace API
  {
    struct URLMaker
    {
      // Input variables. Their meanings depend somewhat on what type
      // of URL is being produced.
      std::string key, wantlist, baseURL;

      URLMaker(const std::string &_baseURL);

      /* Create a link that is meant to be opened in a web browser.
         The user has to perform some action manually on a web site.

         Parameters (either or both may be present):

         - if 'key' is set, the website should ask the user to
           validate the given key for use with the API backend. The
           server may ignore this step if the given key is already
           valid.

         - if 'wantlist' is set, the website should redirect to a
           'buy' page for all the items in the list. The list is a set
           of IDs separated by plus-signs.
       */
      std::string createBrowserLink();

      /* Create a request for more information from the API backend.
         This link is designed to be executed and decoded by
         software. The produced link will return a response coded in
         JSON containing the required information. Use
         api_response.hpp to decode it.

         Parameters:

         - key: must be set. Used to identify ourselves to the server

         - wantlist: may contain the ID of one item (not a list). If
           set, the API will return a one-time download link for the
           given item, or an error code if the item is not accessible.
           If NOT set, the API will instead return user information
           for the current user.
       */
      std::string createJsonLink();

      /* Create a one-way signout message to the server.

         Parameters:

         - key: the auth key to sign out. The key will be deleted from
           the server and will no longer work in future authentication
           requests.
       */
      std::string createSignOutLink();

    private:
      std::string make(const std::string &base, bool q=true);
      std::string signal(const std::string &sign);
    };
  }
}

#endif
