struct MyTimer : wxTimer
{
  MyFrame *frame;

  MyTimer(MyFrame *f)
    : frame(f)
  {
    Start(500);
  }

  void Notify()
  {
    frame->tick();
  }
};

class MyApp : public wxApp
{
  MyTimer *time;

public:
  virtual bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    SetAppName(wxT("tiggit"));

    try
      {
        string exe(wxStandardPaths::Get().GetExecutablePath().mb_str());
        string appData(wxStandardPaths::Get().GetUserLocalDataDir().mb_str());

        // Update the application first
        string version;
        {
          // This requires us to immediately exit in some cases.
          Updater upd(this);
          if(upd.doAutoUpdate(exe))
            return false;

          version = upd.version;
          conf.offline = upd.offline;
        }

        // Then find and load the repository
        if(!Repository::setupPaths(exe, appData))
          return false;

        auth.load();
        ratings.read();

        // Download cached data if this is the first time we run.
        if(conf.updateCache && !conf.offline)
          {
            CacheFetcher cf(this);
            cf.goDoItAlready();
          }

        updateData(conf.updateList || conf.updateCache);
        wxInitAllImageHandlers();

        if(!conf.hideAds && !conf.offline)
          adPicker.setup(auth.isAdmin());

        MyFrame *frame = new MyFrame(wxT("Tiggit - The Indie Game Installer"),
                                     version);
        frame->Show(true);
        time = new MyTimer(frame);

        return true;
      }
    catch(std::exception &e)
      {
        wxString msg(e.what(), wxConvUTF8);
        errorBox(msg);
      }

    return false;
  }
};

IMPLEMENT_APP(MyApp)
