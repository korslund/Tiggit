#include "tigauth.hpp"
#include <spread/misc/jconfig.hpp>
#include "misc/fetch.hpp"
#include "api_response.hpp"
#include "api_urlmaker.hpp"
#include <boost/thread/mutex.hpp>
#include <spread/misc/random.hpp>

using namespace TigAuth;

/* Generate a random alpha-numeric string of any length. If no charset
   is provided, use the default of upper+lower case letters and
   numbers.
 */
static Misc::Random rnd;
static std::string randomString(int len, const std::string &charset = "")
{
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  std::string chars = charset;
  if(chars == "") chars = alphanum;

  std::string result;

  result.resize(len);

  for (int i = 0; i < len; ++i)
    result[i] = chars[rnd.genBelow(chars.size())];

  return result;
}

static std::string listToString(const ItemList &list)
{
  if(list.size() == 0)
    return "";

  std::string res = list[0];
  for(int i=1; i<list.size(); i++)
    {
      res += '+';
      res += list[i];
    }
  return res;
}

static void stringToList(const std::string &input, ItemList &output)
{
  output.clear();

  if(input == "") return;

  int start = 0, end;
  do
    {
      end = input.find('+', start);
      output.push_back(input.substr(start, end-start));
      start = end+1;
    }
  while(end != std::string::npos);
}

#define LOCK boost::lock_guard<boost::mutex> lock(ptr->mutex)

struct Auth::_Internal
{
  std::string baseURL;
  Misc::JConfig conf;
  boost::mutex mutex;

  std::string userid, key, usernick, userauth;
  ItemList items;

  _Internal(const std::string &_baseURL, const std::string &confFile)
    : baseURL(_baseURL), conf(confFile)
  {
    key = conf.get("key");
    userid = conf.get("userid");
    usernick = conf.get("usernick");
    userauth = conf.get("userauth");
    stringToList(conf.get("items"), items);
  }

  /* Generate new key, store it, and return it. Will place the object
     in state 2. Discards any existing state 2 key. Asserts if called
     in state 3.
   */
  std::string makeKey()
  {
    setKey(randomString(12));
    return key;
  }

  void setKey(const std::string &newKey)
  {
    assert(userid == "");
    clear();
    key = newKey;
    writeState();
  }

  // Write current state to config file
  void writeState()
  {
    std::map<std::string,std::string> entries;

    entries["key"] = key;
    entries["userid"] = userid;
    entries["usernick"] = usernick;
    entries["userauth"] = userauth;
    entries["items"] = listToString(items);
    conf.setMany(entries);
  }

  // Return to non-signed in state (state 1).
  void clear()
  {
    key = "";
    userid = "";
    usernick = "";
    userauth = "";
    items.clear();
    writeState();
  }

  /* Process a result string from the server. Does various error
     handling authentication upkeep, as well as internalizing any
     information contained in the response.
   */
  std::string processResponse(const std::string &response)
  {
    // Decode the response. This throws on decode error, which is
    // expected.
    API::Response res;
    try { res.decode(response); }
    catch(...) { res.isValid = false; }
    if(!res.isValid)
      throw std::runtime_error("Invalid server response: '" + response + "'");
    assert(res.isValid);

    /* Handle key updates from the server. The old key will no longer
       work after this.
     */
    if(res.newKey != "")
      {
        key = res.newKey;
        writeState();
      }

    // Respond to errors by throwing a tantrum
    if(res.isError)
      {
        /* Authentication failures basically means we are no longer
           logged in, since the server has rejected our sign-in key.
        */
        if(res.isAuthFail)
          clear();

        throw std::runtime_error("Message from server: " + res.message);
      }

    // Update user info, if any is present in the response
    if(res.hasUserInfo)
      {
        userid = res.userid;
        usernick = res.usernick;
        userauth = res.userauth;
        stringToList(res.useritems, items);
        writeState();
      }

    // Any non-error message from the server is returned back to the
    // caller
    assert(!res.isError);
    return res.message;
  }

  /* Send the given URL signal to the server, and process the returned
     result string from the server.

     May return a response from the server in some cases. May also
     return error messages by throwing exceptions.
   */
  std::string processURL(const std::string &url)
  {
    // Fetch result string from the net. Throws on connection and
    // server errors, which is expected.
    return processResponse(Fetch::fetchString(url));
  }
};

Auth::Auth(const std::string &baseURL, const std::string &confFile)
{ ptr.reset(new _Internal(baseURL, confFile)); }

std::string Auth::getKey() { return ptr->key; }
std::string Auth::getUserID() { return ptr->userid; }
std::string Auth::getUserNick() { return ptr->usernick; }
std::string Auth::getAuthName() { return ptr->userauth; }
const ItemList &Auth::getItemList() { return ptr->items; }

bool Auth::isSignedIn() { return ptr->userid != ""; }
bool Auth::hasKey() { return ptr->key != ""; }

void Auth::setKey(const std::string &newKey)
{
  LOCK;
  ptr->setKey(newKey);
}

std::string Auth::getSignInURL()
{
  LOCK;

  API::URLMaker url(ptr->baseURL);
  url.key = ptr->makeKey();
  return url.createBrowserLink();
}

std::string Auth::getBuyURL(const std::string &item, bool signIn)
{
  LOCK;

  API::URLMaker url(ptr->baseURL);
  if(signIn)
    {
      // If the user needs to sign in, create a new key
      if(!isSignedIn())
        url.key = ptr->makeKey();

      assert(url.key != "");
    }

  // Send our userid so the server can check that we're actually
  // logged in as the same user.
  url.userid = ptr->userid;

  // Set the list of items we want to acquire
  assert(item != "");
  url.wantlist = item;
  return url.createBrowserLink();
}

std::string Auth::getBuyURL(const ItemList &itemList, bool signIn)
{
  // Create a concatenated string and pass it to the string version
  assert(itemList.size() > 0);
  return getBuyURL(listToString(itemList), signIn);
}

void Auth::updateData()
{
  LOCK;

  assert(hasKey());

  API::URLMaker url(ptr->baseURL);
  url.key = ptr->key;
  assert(url.key != "");

  ptr->processURL(url.createJsonLink());
}

std::string Auth::getDownloadLink(const std::string &item)
{
  LOCK;

  assert(item != "");
  assert(isSignedIn());

  API::URLMaker url(ptr->baseURL);
  url.key = ptr->key;
  url.wantlist = item;
  assert(url.key != "");

  return ptr->processURL(url.createJsonLink());
}

void Auth::signOut()
{
  LOCK;

  if(hasKey())
    {
      // Send a fire-and-forget signal to the server that our old key is
      // no longer needed.
      API::URLMaker url(ptr->baseURL);
      url.key = ptr->key;
      Fetch::fetchString(url.createSignOutLink(), true);
    }

  // Clear the state
  ptr->clear();

}
