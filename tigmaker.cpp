#define wxUSE_UNICODE 1
#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/textfile.h>

#include <iostream>
#include <assert.h>
#include "readjson.hpp"
#include "curl_get.hpp"

using namespace std;

string char2hex( char dec )
{
  char dig1 = (dec&0xF0)>>4;
  char dig2 = (dec&0x0F);
  if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
  if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
  if ( 0<= dig2 && dig2<= 9) dig2+=48;
  if (10<= dig2 && dig2<=15) dig2+=97-10;

  string r;
  r.append( &dig1, 1);
  r.append( &dig2, 1);
  return r;
}

// From: http://www.zedwood.com/article/111/cpp-urlencode-function
string urlencode(const string &c)
{
  string escaped="";
  int max = c.length();
  for(int i=0; i<max; i++)
    {
      if ( (48 <= c[i] && c[i] <= 57) ||//0-9
           (65 <= c[i] && c[i] <= 90) ||//abc...xyz
           (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
           (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
           )
        {
          escaped.append( &c[i], 1);
        }
      else
        {
          escaped.append("%");
          escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
  return escaped;
}

// Used to pre-load data from file before we start
struct PreLoad
{
  bool hasLoaded;

  PreLoad() : hasLoaded(false) {}

  wxString version, desc, homepage, url;

  void process(const wxString &line)
  {
    wxString key = line.BeforeFirst(':');
    wxString data = line.AfterFirst(':');

    if(data.IsEmpty()) return;

    if(key == wxT("Version"))
      version = data;
    else if(key == wxT("About"))
      desc = data;
    else if(key == wxT("Website"))
      homepage = data;
    /*
    else if(key == wxT("Download"))
      url = data;
    */
  }

  void load(const wxString &file)
  {
    hasLoaded = true;

    wxTextFile inf;
    inf.Open(file);

    process(inf.GetFirstLine());
    while(!inf.Eof())
      process(inf.GetNextLine());
  }
};

struct Config
{
  // Base url location for tigfile
  std::string baseUrl,

  // Where to store the tigfile on this computer
    localPath,

  // Default values for some fields
    defDevname,
    defPaypal,

  // Authentication values for server interaction
    user,
    api_key,

  // Command to be run on the tigfile after saving. The command is run
  // as "command-string" + " " + full_tigfile_path.
    postHook;

  /* Whether to post games automatically on the website when
     saving. Requires authentication details.

     You should make sure to upload your tigfile into the given url as
     soon as possible after the game is posted!
  */
  bool autoPost;

  bool read(const std::string &filename)
  {
    Json::Value root;
    try
      {
        root = readJson(filename);

        // Required values
        localPath = root["local_path"].asString();
        baseUrl = root["base_url"].asString();

        if(localPath == "" || baseUrl == "")
          throw std::exception();
      }
    catch(...)
      {
        cout << "Missing or incomplete config file. Edit tigmaker.json before proceeding.\n";
        return false;
      }

    // Read the rest
    Json::Value defs = root["defaults"];
    if(!defs.isNull())
      {
        defDevname = defs["devname"].asString();
        defPaypal = defs["paypal"].asString();
      }

    user = root["user"].asString();
    api_key = root["api_key"].asString();
    postHook = root["post_hook"].asString();

    autoPost = root["auto_post"].asBool();

    return true;
  }

  Config() : autoPost(false) {}
};

Config conf;
PreLoad preload;

// When true, exit immediately after clicking Save
bool exitAfterSave = false;

enum MyIDs
  {
    myID_TITLE = 10,
    myID_URLNAME,

    myID_URL,
    myID_LAUNCH,
    myID_VERSION,

    myID_DESC,
    myID_HOMEPAGE,
    myID_DEVNAME,
    myID_SHOT,
    myID_PAYPAL,

    myID_SAVE,
    myID_CLEAR,
    myID_WEBSITE
  };

struct TheFrame : public wxFrame
{
  wxTextCtrl *title, *urlname, *url, *launch, *version,
    *desc, *homepage, *devname, *shot, *paypal;
  wxPanel *panel;
  wxBoxSizer *sizer;

  // When true, the urlname has been changed manually, so stop
  // updating it.
  bool urlChanged;

  wxTextCtrl *addText(const std::string &name, int id, int spacing=6)
  {
    wxString caption((name + ":").c_str(), wxConvUTF8);

    wxBoxSizer *holder = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(holder, 0, wxTOP, spacing);

    holder->Add(new wxStaticText(panel, wxID_ANY, caption, wxDefaultPosition,
                                 wxSize(80,22)));
    wxTextCtrl *res = new wxTextCtrl(panel, id, wxT(""),
                                     wxDefaultPosition, wxSize(320,22));
    holder->Add(res);

    return res;
  }

  wxTextCtrl *addMultiText(const std::string &name, int id, int spacing=6)
  {
    wxString caption((name + ":").c_str(), wxConvUTF8);

    wxBoxSizer *holder = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(holder, 0, wxTOP, spacing);

    holder->Add(new wxStaticText(panel, wxID_ANY, caption, wxDefaultPosition,
                                 wxSize(80,22)));
    wxTextCtrl *res = new wxTextCtrl(panel, id, wxT(""),
                                     wxDefaultPosition, wxSize(320,160),
                                     wxTE_MULTILINE);
    holder->Add(res);

    return res;
  }

  TheFrame()
    : wxFrame(NULL, wxID_ANY, wxT("Register games"), wxDefaultPosition, wxSize(480, 600)),
      urlChanged(false)
  {
    panel = new wxPanel(this);

    Center();

    // Set up the sizers
    wxBoxSizer *outer = new wxBoxSizer(wxVERTICAL);
    sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(outer);
    outer->Add(sizer, 1, wxGROW | wxALL, 20);

    // Set up elements
    title = addText("Title", myID_TITLE);
    urlname = addText("URL name", myID_URLNAME);
    urlname->SetMaxLength(32);

    url = addText("ZIP URL", myID_URL, 40);
    launch = addText("EXE path", myID_LAUNCH);
    version = addText("Version", myID_VERSION);

    desc = addMultiText("Description", myID_DESC, 40);
    homepage = addText("Homepage",  myID_HOMEPAGE, 40);
    shot = addText("Screenshot", myID_SHOT);
    devname = addText("Developer", myID_DEVNAME);
    paypal = addText("Paypal", myID_PAYPAL);

    // Buttons at the bottom (of the sea)
    //sizer->AddStretchSpacer();
    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttons, 0, wxTOP, 10);
    buttons->Add(new wxButton(panel, myID_SAVE, wxT("Save!")));
    buttons->Add(new wxButton(panel, myID_CLEAR, wxT("Clear")));
    buttons->Add(new wxButton(panel, myID_WEBSITE, wxT("Goto Website")));

    clear();

    // Load preload values, if any
    if(preload.hasLoaded)
      {
        desc->ChangeValue(preload.desc);
        url->ChangeValue(preload.url);
        version->ChangeValue(preload.version);
        homepage->ChangeValue(preload.homepage);
      }

    Connect(myID_TITLE, wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler(TheFrame::onTitleChange));
    Connect(myID_URLNAME, wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler(TheFrame::onURLChange));


    Connect(myID_SAVE, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(TheFrame::onSave));
    Connect(myID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(TheFrame::onClear));
    Connect(myID_WEBSITE, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(TheFrame::onWebsite));
  }

  void onTitleChange(wxCommandEvent &event)
  {
    if(!urlChanged)
      {
        // Fetch the new value
        wxString val = title->GetValue();

        // Make sure we never go larger than 32 characters
        val = val(0,32);

        // Making lower-case
        val.MakeLower();

        // Replace invalid characters
        for(int i=0; i<val.Length(); i++)
          {
            char c = val[i];

            if(c != '-' &&
               (c < 'a' || c > 'z') &&
               (c < '0' || c > '9'))
              c = '-';

            val[i] = c;
          }

        // Remove starting or trailing dashes
        int start = 0;
        while(start < val.Length() && val[start] == '-') start++;
        int end = val.Length();
        while(end > start && val[end-1] == '-') end--;
        val = val(start, end-start);

        // Set it
        urlname->ChangeValue(val);
      }
  }

  void onURLChange(wxCommandEvent &event)
  {
    if(event.GetString() != wxT(""))
      urlChanged = true;
    else
      urlChanged = false;
  }

  void set(Json::Value &v, const std::string &key, const wxTextCtrl *txt)
  {
    set(v, key, std::string(txt->GetValue().mb_str()));
  }

  void set(Json::Value &v, const std::string &key, const std::string &value)
  {
    if(value != "")
      v[key] = value;
  }

  void onSave(wxCommandEvent &event)
  {
    Json::Value tig;

    set(tig, "url", url);
    set(tig, "launch", launch);
    set(tig, "version", version);
    set(tig, "title", title);
    set(tig, "desc", desc);
    set(tig, "homepage", homepage);
    set(tig, "devname", devname);
    set(tig, "paypal", paypal);
    set(tig, "shot", shot);

    string s_urlname = string(urlname->GetValue().mb_str());
    string file = s_urlname + ".tig";
    string s_tigurl = conf.baseUrl + file;
    set(tig, "location", s_tigurl);

    file = conf.localPath + file;
    writeJson(file, tig);
    cout << "Wrote " << file << endl;

    // Execute post-hook, if any
    if(conf.postHook != "")
      wxShell(wxString((conf.postHook + " " + file).c_str(), wxConvUTF8));

    // Execute posting
    if(conf.autoPost)
      {
        // Construct the command url
        string cmd = "http://tiggit.net/api/post_game.php";

        // Game UNIX name
        cmd += "?game=" + urlencode(s_urlname);

        // Title
        cmd += "&title=" + urlencode(string(title->GetValue().mb_str()));

        // Tigfile url
        cmd += "&tigurl=" + urlencode(s_tigurl);

        // Username
        cmd += "&user=" + urlencode(conf.user);

        // Secret key
        cmd += "&key=" + urlencode(conf.api_key);

        // Send it off
        CurlGet::get(cmd, "response.txt");

        // Visit finished site
        wxLaunchDefaultBrowser(wxT("http://tiggit.net/game/") + urlname->GetValue());
      }

    clear();

    if(exitAfterSave)
      Close();
  }

  void onClear(wxCommandEvent &event) { clear(); }

  void onWebsite(wxCommandEvent &event)
  {
    wxLaunchDefaultBrowser(homepage->GetValue());
  }

  void clear()
  {
    title->Clear();
    urlname->Clear();
    url->Clear();
    launch->Clear();
    version->Clear();
    desc->Clear();
    homepage->Clear();
    devname->Clear();
    shot->Clear();
    paypal->Clear();

    // Happens automatically but to make sure:
    urlChanged = false;

    // Set default values
    devname->ChangeValue(wxString(conf.defDevname.c_str(), wxConvUTF8));
    paypal->ChangeValue(wxString(conf.defPaypal.c_str(), wxConvUTF8));

    title->SetFocus();
  }
};

static const wxCmdLineEntryDesc cmdLineDesc[] =
  {
    { wxCMD_LINE_SWITCH, wxT("e"), wxT("exit"), wxT("Exit after Save") },
    { wxCMD_LINE_OPTION, wxT("g"), wxT("gdfile"), wxT("Game.cfg file from Game Downloader"), wxCMD_LINE_VAL_STRING },
    { wxCMD_LINE_NONE }
  };

class MyApp : public wxApp
{
public:
  bool OnInit()
  {
    if (!wxApp::OnInit())
      return false;

    if(!conf.read("tigmaker.json"))
      return false;

    TheFrame *frame = new TheFrame();
    frame->Show(true);

    return true;
  }

  void OnInitCmdLine(wxCmdLineParser& parser)
  {
    parser.SetDesc (cmdLineDesc);
    parser.SetSwitchChars(wxT("-"));
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser)
  {
    wxString file;
    if(parser.Found(wxT("g"), &file))
      preload.load(file);

    if(parser.Found(wxT("e")))
      exitAfterSave = true;

    return true;
  }
};

IMPLEMENT_APP(MyApp)
