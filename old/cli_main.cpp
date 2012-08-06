#include "tiglib/repo.hpp"

#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

/* Command Line Interface to TigLib.

   This is not usable for anything yet. It is just meant as a future
   testbed for the library API, and may later possibly be used to
   showcase how you can make your own frontends.
 */

void help()
{
  cout << "\ntiggit-cli <command> [parameters] [options]\n\n"
       << "Commands:\n\n"

       << "    help                        - show this help\n"
       << "    help <command>              - show help for <command>\n"
       << "    repo                        - manage repository\n"

       << "\n\nTiggit is Free Software, licensed under the GNU GPL v3."
       << "\nThis is a pre-alpha prototype of tiggit-cli. Suggestions can\n"
       << "be submitted at http://tiggit.net/forum/.\n\n";
  exit(0);
}

void help(const string &cmd)
{
  cout << endl;
  if(cmd == "repo")
    cout << "repo                   - show default repository location\n";
  else
    help();
  cout << endl;
  exit(0);
}

void fail(const std::string &msg)
{
  cout << msg << endl;
  exit(1);
}

int last;

void parseOpt(char **argv, int &index)
{
  if(index >= last || argv[index][0] != '-')
    return;
}

enum
  {
    NONE = 0,
    HELP, REPO
  };

int cmd = NONE;

void parseCmd(char **argv, int &index)
{
  if(index >= last) return;

  string str = argv[index++];
  if(str == "help")
    cmd = HELP;
  else if(str == "repo")
    cmd = REPO;
  else
    fail("Unknown command: " + str);
}

vector<string> args;

void parseArg(char **argv, int &index)
{
  if(index >= last || argv[index][0] == '-')
    return;

  args.push_back(argv[index++]);
}

TigLib::Repo repo;

void doRepo()
{
  cout << endl;
  if(!repo.findRepo())
    cout << "No default repository found.\n";
  else
    cout << "Repository found at: \n    " << repo.getPath("") << endl;
  cout << endl;
}

int main(int argc, char **argv)
{
  if(argc < 2)
    help();

  int index = 1;
  last = argc;
  parseOpt(argv, index);
  parseCmd(argv, index);
  parseOpt(argv, index);
  while(index < argc)
    {
      int start = index;
      parseArg(argv, index);
      parseOpt(argv, index);
      if(start == index)
        fail("Unknown argument: " + string(argv[index]));
    }

  if(cmd == HELP)
    {
      if(args.size() != 1)
        help();
      else
        help(args[0]);
      return 1;
    }

  if(cmd == REPO) doRepo();

  return 0;
}
