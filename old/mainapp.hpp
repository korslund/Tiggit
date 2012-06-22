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

// ...
return true;
