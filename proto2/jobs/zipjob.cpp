#include "zipjob.hpp"

#include <boost/filesystem.hpp>
#include "unzip.hpp"

void ZipJob::executeJob()
{
  using namespace boost::filesystem;

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
