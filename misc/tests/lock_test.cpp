#include "lockfile.hpp"

#include <iostream>
using namespace std;

void w(bool b)
{
  if(b) cout << "YES\n";
  else cout << "NO\n";
}

int main()
{
  Misc::LockFile lockA("_lock1"), lockB;
  lockB.setFile("_lock1");

  cout << "Locking B:\n";
  w(lockB.lock());
  w(lockA.lock());

  cout << "Checking:\n";
  w(lockA.isLocked());
  w(lockB.isLocked());

  cout << "Aquiring A normally:\n";
  w(lockA.lock());
  lockB.unlock();
  w(lockA.lock());

  cout << "Checking:\n";
  w(lockA.isLocked());
  w(lockB.isLocked());

  cout << "Aquiring B by force:\n";
  w(lockB.lock());
  w(lockB.lock(true));

  cout << "Checking:\n";
  w(lockA.isLocked());
  w(lockB.isLocked());
  return 0;
}
