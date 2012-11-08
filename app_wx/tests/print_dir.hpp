#include <spread/dir/from_fs.hpp>
#include <iostream>

void printDir(Spread::Directory &dir)
{
  using namespace Spread;
  using namespace std;

  Hash::DirMap::const_iterator it;
  for(it = dir.dir.begin(); it != dir.dir.end(); it++)
    {
      string hstring = it->second.toString();

      // Pad hashes up to 50 chars
      for(int i=hstring.size(); i<50; i++)
        hstring += " ";

      cout << hstring << " " << it->first << endl;
    }
  cout << "Total " << dir.dir.size() << " elements\n";;
  cout << "Hash: " << dir.hash() << endl;
}

void printDir(const std::string &where)
{
  using namespace Spread;
  using namespace std;

  Cache::CacheIndex cache;
  DirFromFS dfs(cache);
  Directory dir;
  dfs.includeDirs = true;
  dfs.load(where, dir);
  cout << "\nDirectory: " << where << endl;
  printDir(dir);
}
