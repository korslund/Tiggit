#include "api_urlmaker.hpp"
#include <iostream>

using namespace std;
using namespace TigAuth::API;

int main()
{
  cout << "Browser links:\n";
  {
    URLMaker url("test");
    url.key="KEY";
    cout << "  " << url.createBrowserLink() << endl;
    url.wantlist="hello+world";
    cout << "  " << url.createBrowserLink() << endl;
    url.key="";
    cout << "  " << url.createBrowserLink() << endl;
  }

  cout << "JSON links:\n";
  {
    URLMaker url("http://example.com/api/");
    url.key="KEY";
    cout << "  " << url.createJsonLink() << endl;
    url.wantlist="I+hope+you+are+fine";
    cout << "  " << url.createJsonLink() << endl;
  }

  cout << "Sign-out link:\n";
  {
    URLMaker url("http://someurl/with/long/path");
    url.key="KEY";
    cout << "  " << url.createSignOutLink() << endl;
  }

  return 0;
}
