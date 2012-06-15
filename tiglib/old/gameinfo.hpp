  wxString urlname, idname, name, tigurl, timeString, rating, rating2, dlCount, price;

  // Rate the game and send the rating off to the server
  void rateGame(int rating)
  {
    assert(rating >= 0 && rating <= 5);

    // No point in voting more than once, the server will filter it
    // out.
    if(myRating() != -1) return;

    // Send vote to server
    char str[2];
    str[0] = '0' + rating;
    str[1] = 0;

    DownloadJob *notify = new DownloadJob("http://tiggit.net/api/count/" + entry.urlname + "&rate=" + str, get.tmp->get("rate-" + entry.urlname));
    notify->runAndDelete();

    // Update and save local voting database
    ratings.setRating(entry.idname, rating);
  }

  // Return the rating we given this game, or -1 if we haven't yet
  // rated it.
  int myRating()
  {
    return ratings.getRating(entry.idname);
  }

  // Start downloading this game
  void startDownload()
  {
    assert(!job);
    assert(isNone());

    // Construct the destination file name
    boost::filesystem::path dir = "incoming";
    dir /= entry.idname + ".zip";
    std::string out = get.getPath(dir.string());

    // Start the download job
    job = new DownloadJob(entry.tigInfo.url, out);
    job->run();

    // Also give the server an anonymous update to statistics
    DownloadJob *notify = new DownloadJob("http://tiggit.net/api/count/" + entry.urlname + "&download", get.tmp->get("count-" + entry.urlname));
    notify->runAndDelete();

    // Set status
    setDownloading();
  }

  /* Called regularly to update status. Returns:

     0 - status has not changed
     1 - status has changed and the install list should be updated
     2 - status has changed, display lists and install list should be
         updated

     May also throw exceptions. These should also be counted as status
     changes (same as when returning 2.)
  */
  int updateStatus()
  {
    // Are we doing anything at the moment?
    if(!isWorking())
      return false;

    assert(job);

    int exitStatus = 0;
    bool error = false;
    StatusJob *newJob = NULL;
    std::string errMsg = "";

    if(job->isFinished())
      {
        if(job->isSuccess())
          {
            if(isDownloading())
              {
                // Download succeeded. Start unpacking.
                DownloadJob *g = dynamic_cast<DownloadJob*>(job);
                assert(g);

                // Get install path
                boost::filesystem::path dir = conf.gamedir;
                dir /= entry.idname;

                // Set up installer
                newJob = new ZipJob(g->file, get.getPath(dir.string()));
                jobQueue.queue(newJob);
                setUnpacking();

                setStatus(wxT("Unpacking..."));
              }
            else
              {
                // The game has finished installing, and is now ready
                // for use.
                exitStatus = 1;
                assert(isUnpacking());
                setInstalled();
              }
          }
        else
          {
            // The job did not finish
            exitStatus = 2;
            setUninstalled();

            // Did it fail?
            if(job->isError())
              {
                // Set errMsg depending on job type
                error = true;
                if(isDownloading())
                  errMsg = "Downloading failed for " + entry.name + ":\n" +
                    job->getError();
                else
                  errMsg = "Install failed for " + entry.name + ":\n" +
                    job->getError();
              }
          }

        // Since the job is done, delete the object.
        jobQueue.unlink(job);
        delete job;
        job = newJob;
      }
    else
      {
        // Our job is still running. Update download stats.
        if(isDownloading())
          {
            DownloadJob *g = dynamic_cast<DownloadJob*>(job);
            assert(g);
          }
      }

    if(error)
      throw std::runtime_error(errMsg);

    return exitStatus;
  }

  /* Request the screenshot of this game. The parameter callback will
     be invoked once the screenshot is ready.

     The callback might be invoked immediately (during this call), or
     it might be invoked later (eg. if it needs to be downloaded.) And
     if the screenshot is missing, the callback will not be invoked at
     all.

     However, if it IS invoked, it is always invoked in the main
     thread, possibly through jobQueue.update().

     It's OK for cb to be NULL. In that case this function simply
     preloads the image for later use.
   */
  void requestShot(ScreenshotCallback *cb = NULL)
  {
    // Use exact sized image if it exists
    std::string url = entry.tigInfo.shot300x260;

    // If not, try the general screenshot
    if(url == "") url = entry.tigInfo.shot;

    // Exit if there's no valid screenshot to use
    if(url == "")
      return;

    // Figure out the cache filename
    std::string filename = get.getPath("cache/shot300x260/" + entry.idname);

    // Let the screenshot loader handle all the details internally
    screen.handleRequest(entry.idname, filename, url, cb);
  }
