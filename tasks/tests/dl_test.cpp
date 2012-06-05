#include "../download.hpp"

#include <iostream>
using namespace std;

int main()
{
  string url = "http://tiggit.net/client/latest.tig";
  cout << "Downloading " << url << "...\n";

  Jobify::JobInfoPtr info(new Jobify::JobInfo);
  Tasks::DownloadTask dlj(url, "_output.txt", info);

  dlj.run();

  if(info->isSuccess())
    cout << "Success!\n";
  else
    cout << "Failure: " << info->message << endl;

  return 0;
}
