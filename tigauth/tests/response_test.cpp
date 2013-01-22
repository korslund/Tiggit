#include "api_response.hpp"
#include <iostream>

using namespace std;

void test(const std::string &str)
{
  cout << "\nTESTING: '" << str << "'\n";
  TigAuth::API::Response res;
  try { res.decode(str); }
  catch(exception &e)
    {
      cout << "CAUGHT: " << e.what() << endl;
      return;
    }
  if(!res.isValid)
    {
      cout << "Invalid response\n";
      return;
    }

  cout << " isError=" << res.isError
       << " isAuthFail=" << res.isAuthFail
       << " hasUserInfo=" << res.hasUserInfo << endl
       << " newKey=" << res.newKey
       << " message=" << res.message << endl
       << " userid=" << res.userid
       << " usernick=" << res.usernick
       << " userauth=" << res.userauth
       << " useritems=" << res.useritems << endl
       << " generated=" << res.generated << endl;
}

int main()
{
  test("");
  test("{}");
  test("{\"type\":\"fishcat\"}");
  test("{\"type\":\"message\"}");
  test("{\"type\":\"error\"}");
  test("{\"type\":\"authfail\",\"generated\":\"now\"}");

  test("{\"type\":\"message\",\"newkey\":\"NEW KEY!\"}");
  test("{\"type\":\"message\",\"message\":\"This is a message\"}");
  test("{\"type\":\"error\",\"message\":\"This is an error message\"}");
  test("{\"type\":\"message\",\"userinfo\":{\"userid\":\"1234\",\"nickname\":\"My Name is Nick\"}}");

  test("{\"type\":\"message\",\"message\":\"MESSAGE\",\"userinfo\":{\"userid\":\"1234\",\"nickname\":\"My Name is Nick\",\"authname\":\"Home\",\"items\":\"item1+item2+another_item+anitemIreallylike\"},\"newkey\":\"NEWKEY\",\"generated\":\"2013-01-22 15:47:12 UTC\"}");
  return 0;
}
