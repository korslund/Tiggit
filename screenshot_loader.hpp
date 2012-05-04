#ifndef __SCREENSHOT_LOADER_HPP_
#define __SCREENSHOT_LOADER_HPP_

#include "downloadjob.hpp"
#include "jobqueue.hpp"
#include <wx/image.h>
#include <string>

// Interface used to notify a caller that a screenshot is ready.
struct ScreenshotCallback
{
  // Called with finished screenshot data. The id is passed from
  // handleRequest and describes what game this image belongs to.
  virtual void shotIsReady(const std::string &id, const wxImage &shot) = 0;
};

class ScreenshotLoader : public ThreadJob
{
  struct Invoker : StatusJob
  {
    ScreenshotLoader *loader;
    ScreenshotCallback *cb;

    Invoker(ScreenshotLoader *l, ScreenshotCallback *c)
      : loader(l), cb(c)
    {
      assert(loader);
    }

    void executeJob()
    {
      if(loader->isLoaded && cb)
        cb->shotIsReady(loader->id, loader->memImage);
      setDone();
    }
  };

  // True when the image is loaded into memory
  bool isLoaded;

  // Memory image cache
  wxImage memImage;

  // Shared strings
  std::string id, filename, url;
  ScreenshotCallback *cb;

  Invoker *invoker;

  // Loads the memory image from cache. If successful, isLoaded will
  // be true.
  void loadImage()
  {
    // If the image is already loaded, there's nothing more to do
    if(isLoaded) return;

    // Does the cache file exist?
    if(!boost::filesystem::exists(filename))
      return;

    // It does. Try to load it.
    wxLogNull dontLog;
    if(!memImage.LoadFile(wxString(filename.c_str(), wxConvUTF8)))
      return;

    // Success. Set 'loaded' status.
    isLoaded = true;
  }

  void executeJob()
  {
    // Download the image
    DownloadJob dl(url, filename);

    // Run the job directly in this thread
    dl.runNoThread();

    // Anything other than success is a failure
    if(!dl.isSuccess())
      {
        if(dl.isError())
          setError(dl.getError());
        else
          setError("Unknown download error");

        return;
      }

    // Image should now be downloaded into the cache file. Try to load
    // it.
    loadImage();

    if(!isLoaded)
      {
        // Loading failed
        setError("Loading image failed");
        return;
      }

    // The image loaded, but it might be too big.
    int W = memImage.GetWidth();
    int H = memImage.GetHeight();

    if(W > 300 || H > 260)
      {
        float aspect = W*1.0/H;

        if(aspect >= 300.0/260.0)
          {
            // Use full width
            W = 300;
            H = (int)(300/aspect);
          }
        else
          {
            // Use full height
            H = 260;
            W = (int)(260*aspect);
          }

        // Resize the image, and then save the correctly sized image
        // to the cache.
        memImage.Rescale(W, H
#if wxCHECK_VERSION(2, 8, 0)
                         , wxIMAGE_QUALITY_HIGH
#endif
                         );
        memImage.SaveFile(wxString(filename.c_str(), wxConvUTF8), wxBITMAP_TYPE_PNG);
      }

    // Complete success! Schedule a callback invocation.
    assert(invoker == NULL);
    invoker = new Invoker(this, cb);
    jobQueue.queue(invoker);
    setDone();
  }

public:
  ScreenshotLoader() : isLoaded(false), invoker(NULL), cb(NULL) {}

  void handleRequest(const std::string &_id, const std::string &_filename,
                     const std::string &_url, ScreenshotCallback *_cb)
  {
    // Are we already trying to fetch the image?
    if(isBusy()) return;

    // Has a previous download job failed?
    if(isError()) return;

    // Copy parameters
    id = _id;
    filename = _filename;
    url = _url;
    cb = _cb;

    // Try loading the cache first
    if(!isLoaded)
      loadImage();

    // If we have the image in memory, just serve it directly
    if(isLoaded)
      {
        if(cb)
          cb->shotIsReady(id, memImage);
        return;
      }

    // Ok, so let's try to fetch the image in the background. This
    // runs executeJob() in another thread.
    run();
  }
};

#endif
