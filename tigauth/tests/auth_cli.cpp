#include <iostream>
#include "tigauth.hpp"
#include <spread/misc/readjson.hpp>

using namespace std;
string baseURL;
string url;

TigAuth::Auth *auth;

void check()
{
  cout << "CHECKING STATE: ";
  if(auth->isSignedIn())
    {
      cout << "Signed in\n";
      cout << "  Key:    " << auth->getKey() << endl;
      cout << "  UserID: " << auth->getUserID() << endl;
      cout << "  Nick:   " << auth->getUserNick() << endl;
      cout << "  Auth:   " << auth->getAuthName() << endl;
      cout << "  Items:\n";
      const TigAuth::ItemList &items = auth->getItemList();
      for(int i=0; i<items.size(); i++)
        cout << "    " << items[i] << endl;
    }
  else if(auth->hasKey())
    cout << "Key only, key=" << auth->getKey() << endl;
  else
    cout << "Signed out\n";
  cout << endl;
}

void update()
{
  cout << "UPDATING\n";
  try { auth->updateData(); }
  catch(exception &e) { cout << "ERROR: " << e.what() << endl; }
  check();
}

void fetch(const std::string &item)
{
  cout << "FETCH " << item << endl;
  try { cout << "URL=" << auth->getDownloadLink(item) << endl; }
  catch(exception &e) { cout << "ERROR " << e.what() << endl; }
  check();
}

void login(const std::string &key)
{
  cout << "LOGIN as " << key << endl;
  auth->setKey(key);
  check();
  update();
}

int main()
{
  baseURL = ReadJson::readJson("data.json").asString();
  cout << "Base URL: " << baseURL << endl;

  // Initialize the object
  auth = new TigAuth::Auth(baseURL, "_auth.conf");
  cout << "\nInitial state\n";
  check();

  login("unknown");
  login("dude");

  cout << "Fetching a download link\n";
  fetch("three");

  update();

  cout << "Fail on purpose:\n";
  fetch("abc");
  fetch("def");

  // Sign out
  if(auth->isSignedIn())
    {
      cout << "Signing out.\n";
      auth->signOut();
      check();
    }

  /*
  // Get sign-in URL
  try
    {
      //url = auth->getSignInURL();
      url = auth->getBuyURL("item");
      cout << "URL for signing in: " << url << endl;

      cout << "Press ENTER when you have enabled this key online\n";
      cin.get();
      auth->updateData();

      // This try-catch-check structure is the standard way of calling
      // state-changing functions like updateData() and
      // getDownloadLink().
    }
  catch(exception &e) { cout << "ERROR " << e.what() << endl; }
  check();
  */

  return 0;
}
