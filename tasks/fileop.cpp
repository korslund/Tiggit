#include "fileop.hpp"

#include <boost/filesystem.hpp>

using namespace Tasks;
using namespace Jobify;
namespace bs = boost::filesystem;

void FileOpTask::addOp(const FileOp &op)
{
  ops.push_back(op);
}

void FileOpTask::addOp(int type, const std::string &source, const std::string &dest)
{
  FileOp op;
  op.type = type;
  op.source = source;
  op.dest = dest;
  addOp(op);
}

void FileOpTask::doJob()
{
  info->total = ops.size();
  info->current = 0;

  OpList::iterator it;
  for(it = ops.begin(); it != ops.end(); it++)
    {
      const FileOp &op = *it;

      // Deletes are easy to handle, so get them out of the way first.
      if(op.type == FILEOP_DELETE)
        {
          setBusy("Removing " + op.dest);
          bs::remove_all(op.dest);
          continue;
        }

      /* Fix this up later. I want more robust wrt directory copying
         and moving. In the copy case, it is pretty straight forward
         to index a directory (weeding out symbolic link loops), then
         copy each file individually, making sure to write directories
         along the way.
       */
      if(op.type == FILEOP_MOVE)
        {
          setBusy("Moving " + op.source + " => " + op.dest);
          if(bs::exists(op.dest) && !bs::is_directory(op.dest))
            bs::remove(op.dest);
          bs::rename(op.source, op.dest);
        }
      else if(op.type == FILEOP_COPY)
        {
          // Determine what to do first
          if(!bs::exists(op.source))
            setError("File not found - " + op.source);
          else if(bs::is_directory(op.source))
            setError("Directory copy not implemented yet");
          else
            {
              // Source exists and is a file
              if(bs::exists(op.dest))
                {
                  // Destination exists
                  if(bs::is_directory(op.dest))
                    setError("Cannot copy to directories yet");
                  else
                    // Overwrite existing file
                    bs::remove(op.dest);
                }
            }

          // Exit on errors
          if(!info->isBusy())
            return;

          // Single file copy only
          setBusy("Copying " + op.source + " => " + op.dest);
          bs::copy_file(op.source, op.dest);
        }
      else setError("Invalid operation");

      // Check for aborts or errors
      if(checkStatus())
        return;

      info->current++;
    }

  setDone();
}
