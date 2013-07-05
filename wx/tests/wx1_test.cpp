#include "frame.hpp"

#include <iostream>

using namespace std;
using namespace wxTiggit;

struct TestInfo : wxGameInfo
{
  wxString name;
  wxImage image;

  TestInfo(const string &n)
    : name(strToWx(n)) {}

  bool isNew() const { return false; }
  bool isInstalled() const { return false; }
  bool isUninstalled() const { return true; }
  bool isWorking() const { return false; }
  bool isDemo() const { return false; }

  wxString getVersion() const { return wxT("1.0!"); }
  wxString getSize() const { return wxT("10 gigaflops"); }

  wxString getTitle(bool includeStatus=false) const
  {
    return name;
  }
  wxString timeString() const
  {
    return wxT("Parallel universe");
  }
  wxString rateString() const
  {
    return wxT("OVER 9000!");
  }
  wxString dlString() const
  {
    return wxT("1234.5");
  }
  wxString statusString() const
  {
    return wxT("Ah, not bad.");
  }

  std::string getHomepage() const { return "http://tiggit.net/"; }
  std::string getIdName() const { return "tiggit.net/mayor-poo"; }
  wxString getDesc() const { return wxT("Description!"); }
  std::string getDir() const { return "/"; }
  int myRating() const { return 4; }

  const wxImage &getShot() { return image; }

  void rateGame(int i)
  {
    cout << "Rating to " << i << endl;
  }

  void installGame() { cout << "Install\n"; }
  void uninstallGame() { cout << "Uninstall\n"; }
  void launchGame() { cout << "Launch\n"; }
  void abortJob() { cout << "Abort\n"; }
};

TestInfo
  game1("Game 1"), game2("Game 2!!"), game3("Another game");

struct TestList : wxGameList
{
  void addListener(wxGameListener*) {}
  void removeListener(wxGameListener*) {}

  void flipReverse() {}
  void setReverse(bool) {}

  bool sortTitle() { cout << "Title!\n"; return false; }
  bool sortDate() { cout << "Date!\n"; return false; }
  bool sortRating() { cout << "Rating!\n"; return false; }
  bool sortDownloads() { cout << "Downloads!\n"; return false; }

  void clearTags() {}
  void setTags(const std::string &) {}
  void setSearch(const std::string &str)
  { cout << "Setting search: " << str << endl; }
  int countTags(const std::string &str) { return str.size(); }

  int size() const { return 4; }
  const wxGameInfo& get(int i) { return edit(i); }
  wxGameInfo& edit(int i)
  {
    if(i == 0) return game1;
    if(i == 1) return game2;
    return game3;
  }
};

struct TestConf : wxGameConf
{
  virtual bool getShowVotes() { return false; }
  virtual void setShowVotes(bool b)
  {
    cout << "Setting option: " << (b?"TRUE":"FALSE") << endl;
  }
};

struct TestNews : wxGameNews
{
  wxGameNewsItem it1, it2;

  TestNews()
  {
    it1.dateNum = 1;
    it1.date = wxT("long ago");
    it1.subject = wxT("Some subject");
    it1.body = wxT("Some body");
    it1.read = true;

    it2.dateNum = 2;
    it2.date = wxT("in a galaxy far away");
    it2.subject = wxT("Help!");
    it2.body = wxT("I'm trapped in this little text box!");
    it2.read = false;
  }

  const wxGameNewsItem &get(int i) const
  {
    if(i == 0) return it1;
    return it2;
  }

  int size() const { return 2; }
  void reload() {}

  void markAsRead(int i)
  {
    if(i==0) it1.read = true;
    it2.read = true;
  }

  void markAllAsRead()
  {
    markAsRead(0);
    markAsRead(1);
  }
};

TestList listLatest, listFreeware, listDemos, listInstalled;
TestConf testConf;
TestNews testNews;

struct TestData : wxGameData
{
  wxGameList &getLatest() { return listLatest; }
  wxGameList &getFreeware() { return listFreeware; }
  wxGameList &getDemos() { return listDemos; }
  wxGameList &getInstalled() { return listInstalled; }
  bool isActive() { return false; }

  bool moveRepo(const std::string &newRepo)
  {
    cout << "Moved repo to " << newRepo << endl;
    return true;
  }

  std::string getRepoDir() { return "over the rainbow"; }

  wxGameNews &getNews() { return testNews; }
  wxGameConf &conf() { return testConf; }

  void notifyButton(int id) {}
};

TestData testData;

struct TigApp : wxApp
{
  bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    SetAppName(wxT("Test App"));
    wxInitAllImageHandlers();

    game1.image.LoadFile(wxT("game1.png"));
    game2.image.LoadFile(wxT("game2.png"));
    game3.image.LoadFile(wxT("game3.png"));

    TigFrame *frame = new TigFrame(wxT("Test App"), "1", testData);
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(TigApp)
