#ifndef __TIGAUTH_AUTH_HPP_
#define __TIGAUTH_AUTH_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include <vector>

/*
-----------------------------------------
  STATE #1: Not Signed In
  Can move to: STATE#2
-----------------------------------------

   Identification:
       isSignedIn()  =  false
       hasKey()      =  false

   Allowed operations:
       getSignInURL()
       getBuyURL()
       - on success, either function will change the state to STATE#2.
       signOut()
       - has no effect
       setKey()

-----------------------------------------
  STATE #2: Key generated
  Can move to: STATE#1, STATE#3
-----------------------------------------

   Identification:
       isSignedIn()  =  false
       hasKey()      =  true

   Allowed operations:
       getKey()
       signOut()
       - returns to STATE#1
       updateData()
       - on success, will change state to STATE#3

       getSignInURL()
       getBuyURL()
       setKey()
       - all of these will discard current key, and will otherwise act
         exactly like in STATE#1

-----------------------------------------
  STATE #3: Authenticated
  Can move to: STATE#1
-----------------------------------------

   Identification:
       isSignedIn()  =  true
       hasKey()      =  true

   Allowed operations:
       updateData()
       signOut()
       getBuyURL()
       getDownloadLink()
       getKey()
       getUserID() etc
 */

namespace TigAuth
{
  typedef std::vector<std::string> ItemList;

  struct Auth
  {
    /* Constructor. This loads our current state from a config file,
       if any. All future state changes are written to the file.

       The constructor does not perform any internet connections.

       Parameters:

       baseURL - base API url used for all future server connections
       confFile - config file used to store and restore state
     */
    Auth(const std::string &baseURL, const std::string &confFile);

    /* Get the current authorization key. The key is a randomly
       generated string and may change at any time. The function is
       valid in STATE#2 and STATE#3.
     */
    std::string getKey();

    /* Set a new key. Mostly intended for testing purposes.
     */
    void setKey(const std::string &newKey);

    /* User data functions can only be called in STATE#3. They should
       only be called after isSignedIn() has returned true.
     */
    std::string getUserID();
    std::string getUserNick();
    std::string getAuthName();
    const ItemList &getItemList();

    /* State detection functions. See the documentation at the top of
       the file.
     */
    bool isSignedIn();
    bool hasKey();

    /* Update user data from server.

       Will throw an exception if:
       - called in STATE#1 (no key present)
       - authentication failed (key is invalid). This will also sign
         you out (set our state to STATE#1.)
       - connection or server failure (does not affect state)

       After calling (even if it throws), always remember to recheck
       the state with isSignedIn(), since it may or may not have
       changed.
     */
    void updateData();

    /* Sign out. This will reset the current state to STATE#1.

       This will also instruct the server to invalidate the current
       key, but does not require any response from the server and will
       not throw on connection errors.

       May only be called in STATE#3.
     */
    void signOut();

    /* Create a sign-in URL for the user. This URL is meant to be
       opened in a web browser, and contains a generated API key. The
       web URL signs the user in (if necessary), then asks them if
       they would like to approve the API key.

       Once the user has approved the key, updateData() should be
       called to complete the sign-in.

       This moves the Auth object from STATE#1 to STATE#2. The final
       move to STATE#3 (signed in) is done by updateData().
     */
    std::string getSignInURL();

    /* Produce an URL that redirects the user to a buy page. The URL
       is meant to be opened in a web browser. After signing in (if
       necessary), the website will present the user with what they
       need to purchase or otherwise obtain the given item(s).

       If signIn=true, the generated URL will also act as a sign-in
       URL for the user (if necessary), signing them into the
       client. This works just as if the URL was generated with
       getSignInURL(), and the result will be exactly the same as when
       calling getSignInURL, except that the user is also redirected
       the buy page after signing in.

       You may specify several items to purchase, either through the
       list version, or through the string version by concatenating
       IDs into a list delimited by plus signs, ie.:
       item1+item2+...

       After the user signals they have finished the purchase, a call
       to updateData() can be used to update the client's list of
       owned items.
     */
    std::string getBuyURL(const std::string &item, bool signIn=true);
    std::string getBuyURL(const ItemList &itemList, bool signIn=true);

    /* Get a temporary download link for a given item. This requires
       the user to be signed in (STATE#3).

       Throws an exception if the requested item is not available to
       this user.

       This function also throws exceptions under the same conditions
       as updateData(), and like updateData() may also sign you out if
       the authentication failed.
     */
    std::string getDownloadLink(const std::string &item);

  private:
    struct _Internal;
    boost::shared_ptr<_Internal> ptr;
  };
}

#endif
