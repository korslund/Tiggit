#include "repo.hpp"
#include "fetch.hpp"
#include "findrepo.hpp"
#include "misc/jconfig.hpp"
#include <boost/filesystem.hpp>

using namespace TigLib;

struct MyFetch : GameInfo::URLManager
{
  void getUrl(const std::string &url, const std::string &outfile)
  { fetchFile(url, outfile); }
};

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

Repo::Repo(const std::string &where)
  : dir(where)
{
  if(dir == "")
    dir = findRepo();

  if(dir == "")
    fail("Could not find a standard repository path");

  if(!lock.lock(getPath("lock")))
    fail("Failed to lock " + dir);

  initRepo();

  listFile = getPath("all_games.json");
  tigDir = getPath("tigfiles/");
}

/* Initialize repository. Is specifically used to convert old
   repository content into something we can use.
 */
void Repo::initRepo()
{
  using namespace boost::filesystem;
  using namespace Misc;

  // Is there an old config file?
  std::string oldcfg = getPath("config");
  if(exists(oldcfg))
    try
      {
        // Convert it to something usable
        JConfig in(oldcfg);

        std::string tmp;
        tmp = getPath("wxtiggit.conf");
        if(in.has("vote_count") && !exists(tmp))
          {
            JConfig out(tmp);
            out.setBool("show_votes", in.getBool("vote_count"));
          }

        tmp = getPath("tiglib.conf");
        if(in.has("last_time") && !exists(tmp))
          {
            JConfig out(tmp);
            int64_t oldTime = in.getInt("last_time");

            // Store as binary data, since our jsoncpp doesn't support
            // 64 bit ints.
            out.setData("last_time", &oldTime, 8);
          }

        // Kill the old file
        remove(oldcfg);
      }
    catch(...) {}

  /* TODO:
     - convert installed.json
     - rename all cache/shot300x260/tiggit.net/* to add .png.
     - don't delete old config files, rename them to .old instead.

     - only do these things if we know we are converting an old
       repository. Ie, they should all be done if the old config
       exists. It makes sense to move all this to a separate function,
       or set of functions. They can be static here, that's fine. The
       file repo.cpp is exactly the right place for this code.
   */
}

std::string Repo::getPath(const std::string &fname)
{
  using namespace boost::filesystem;
  return (path(dir)/fname).string();
}

void Repo::fetchFiles()
{
  fetchIfOlder("http://tiggit.net/api/all_games.json",
               listFile, 60);
}

void Repo::loadData()
{
  MyFetch fetch;
  data.data.addChannel(listFile, tigDir, &fetch);
  data.copyList();
}
