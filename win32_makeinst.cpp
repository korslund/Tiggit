#include <fstream>
#include <iostream>

using namespace std;

struct SizeInfo
{
  int sign, start, size;
};

int main()
{
  ifstream exe("setup_stub.exe", ios::binary);
  ifstream zip("payload.zip", ios::binary);
  ofstream out("ogl_setup.exe", ios::binary);

  if(!exe || !zip || !out)
    {
      cout << "Failed.\n";
      cout << "This program uses:\n";
      cout << "  setup_stub.exe   (input)\n";
      cout << "  payload.zip      (input)\n";
      cout << "  ogl_setup.exe    (output)\n";
      return 1;
    }

  SizeInfo si;

  si.sign = 0x13737FAB;

  char buf[1024];
  while(!exe.eof())
    {
      exe.read(buf, 1024);
      size_t cnt = exe.gcount();
      out.write(buf, cnt);
      si.start += cnt;
    }

  while(!zip.eof())
    {
      zip.read(buf, 1024);
      size_t cnt = zip.gcount();
      out.write(buf, cnt);
      si.size += cnt;
    }

  out.write((char*)&si, sizeof(SizeInfo));
  cout << "Wrote: exe=" << si.start << " zip=" << si.size << endl;

  return 0;
}
