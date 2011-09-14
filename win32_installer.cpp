#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>
#include <fstream>
#include "unzip.hpp"

namespace fs = boost::filesystem;

TCHAR pathbuf[MAX_PATH];

fs::path getBinPath(const std::string &appName)
{
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathbuf);
  fs::path base = pathbuf;
  return base / appName / "bin";
}

std::string getExe()
{
  GetModuleFileName(NULL, pathbuf, MAX_PATH);
  return std::string(pathbuf);
}

void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

// Extract payload from an exe file to the given destination folder
void extractPayload(const std::string &exe,
                    const std::string &dest)
{
  std::ifstream inf(exe.c_str(), std::ios::binary);
  if(!inf)
    fail("Failed to open " + exe);

  char buff[4096];

  inf.read(buff, sizeof(buff));
  if(!inf)
    fail("Failed to read " + exe);

  IMAGE_DOS_HEADER* dosheader = (IMAGE_DOS_HEADER*)buff;
  if(dosheader->e_magic != IMAGE_DOS_SIGNATURE ||
     dosheader->e_lfanew >= (sizeof(buff) - sizeof(IMAGE_NT_HEADERS32)))
    fail("Invalid EXE header in " + exe);

  IMAGE_NT_HEADERS32* header = (IMAGE_NT_HEADERS32*)(buff + dosheader->e_lfanew);
  if(header->Signature != IMAGE_NT_SIGNATURE)
    fail("Invalid EXE header in " + exe);

  IMAGE_SECTION_HEADER* sectiontable =
    (IMAGE_SECTION_HEADER*)((char*)header + sizeof(IMAGE_NT_HEADERS32));

  if((char*)sectiontable >= buff + sizeof(buff))
    fail("Invalid EXE header in " + exe);

  size_t maxpointer = 0, exesize = 0;

  // Loop through the sections and find the one with the highest
  // offset.
  for(int i = 0; i < header->FileHeader.NumberOfSections; ++i)
    {
      if(sectiontable->PointerToRawData > maxpointer)
        {
          maxpointer = sectiontable->PointerToRawData;
          exesize = sectiontable->PointerToRawData + sectiontable->SizeOfRawData;
        }
      sectiontable++;
    }

  // Seek to the data
  inf.clear();
  inf.seekg(exesize);

  // Set up the destination file
  std::ofstream of(dest.c_str(), std::ios::binary);

  // Read and dump until we run out of data
  size_t total = 0;
  while(!inf.eof())
    {
      inf.read(buff, sizeof(buff));
      size_t count = inf.gcount();
      of.write(buff, count);
      total += count;
    }

  // Fail if no data was read
  if(total == 0)
    fail("No embedded data found in " + exe);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
  fs::path bin = getBinPath("tiggit");
  std::string exe = getExe();

  // Make sure bin/ exists
  fs::create_directories(bin);

  // Destination zip file
  std::string zip = (bin / "tmp.zip");

  try
    {
      // Extract embedded ZIP file
      extractPayload(exe, zip);

      // Unpack the contents into the bin/ folder
      UnZip z;
      z.unpack(zip, bin.string());
    }
  catch(std::exception &e)
    {
      MessageBox(NULL, e.what(), "Error", MB_ICONERROR);
      return 1;
    }

  // Kill the zip file
  fs::remove(zip);

  // Run the main program
  ShellExecute(NULL, "open", (bin/"tiggit.exe").string().c_str(),
               NULL, NULL, SW_SHOW);

  return 0;
}
