#ifndef _INSTALL_HPP
#define _INSTALL_HPP

#include <queue>
#include <boost/filesystem.hpp>

#include "unzip.hpp"
#include "jobify.hpp"

struct ZipJob : ThreadJob
{
  int status;
  std::string zip, dir;

  ZipJob(const std::string &_zip, const std::string &_dir)
    : zip(_zip), dir(_dir)
  {}

  void executeJob()
  {
    using namespace boost::filesystem;
    using namespace std;
    setBusy();

    // Make sure the directory exists
    create_directories(dir);

    // Unzip the file
    UnZip unzip;

    bool success = true;
    string z = zip;

    try { unzip.unpack(zip,dir); }
    catch(std::exception &e)
      {
        // Remove install directory
        remove_all(dir);

        setError(e.what());

        success = false;
      }

    // Delete the zip file
    remove(z);

    // Mark as done, unless there was an error
    if(success)
      setDone();
  }
};

#endif
