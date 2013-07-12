/* This is a drop-in include for frame.cpp, not meant to be used
   separately.
 */
#include <wx/hyperlink.h>

struct SuggestDialog : wxDialog
{
  bool ok;

  std::string title, homepage, shot, download, version, devname, tags, type, desc;

  wxPanel *panel;
  wxBoxSizer *sizer;

  wxBoxSizer *getSpaced(int spacing=6)
  {
    wxBoxSizer *holder = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(holder, 0, wxTOP, spacing);
    return holder;
  }

  wxTextCtrl *addText(const std::string &name, int spacing=6, bool isMultiLine=false)
  {
    wxBoxSizer *holder = getSpaced(spacing);

    holder->Add(new wxStaticText(panel, wxID_ANY, strToWx(name), wxDefaultPosition,
                                 wxSize(110,isMultiLine?50:22)));
    wxTextCtrl *res;
    if(isMultiLine)
      res = new wxTextCtrl(panel, -1, wxT(""), wxDefaultPosition, wxSize(320,160),
                           wxTE_MULTILINE);
    else
      res = new wxTextCtrl(panel, -1, wxT(""), wxDefaultPosition, wxSize(320,22));

    holder->Add(res);

    return res;
  }

  wxTextCtrl *addMultiText(const std::string &name, int spacing=6)
  {
    return addText(name, spacing, true);
  }

  wxRadioButton *addRadio(const std::string &name, bool isFirst=false, int spacing=0)
  {
    wxBoxSizer *holder = getSpaced(spacing);

    wxRadioButton *res;
    if(isFirst) res = new wxRadioButton(panel, -1, strToWx(name), wxDefaultPosition,
                                        wxDefaultSize, wxRB_GROUP);
    else res = new wxRadioButton(panel, -1, strToWx(name));
    holder->Add(res, 0, wxLEFT, 110);
    return res;
  }

  SuggestDialog(wxWindow *parent)
    : wxDialog(parent, -1, wxT("Suggest A Game"), wxDefaultPosition,
               wxSize(300,500))
  {
    panel = new wxPanel(this);

    // Set up the sizers
    wxBoxSizer *outer = new wxBoxSizer(wxVERTICAL);
    sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(outer);
    outer->Add(sizer, 1, wxGROW | wxALL, 20);

    wxTextCtrl *t_title, *t_homepage, *t_shot, *t_download, *t_version,
      *t_devname, *t_tags, *t_desc;

    wxRadioButton *r_free, *r_open, *r_demo;

    sizer->Add(new wxStaticText(panel, -1, wxT("Suggest a game that you want to add to Tiggit:")));
    t_title = addText("Title:", 20);
    sizer->Add(new wxStaticText(panel, -1, wxT("OPTIONAL: provide more info to add the game faster:")), 0, wxTOP, 17);
    t_homepage = addText("Homepage URL:", 18);
    t_shot = addText("Screenshot URL:");
    t_download = addText("Download URL:");
    t_desc = addMultiText("Description: (shown in Tiggit)", 8);

    r_free = addRadio("Freeware", true, 6);
    r_open = addRadio("Open source");
    r_demo = addRadio("Demo / Commercial");

    t_version = addText("Latest version:");
    t_devname = addText("Developer:");
    t_tags = addText("Tags:");
    sizer->Add(new wxStaticText(panel, wxID_ANY, wxT("Suggested tags: arcade action cards casual fps fighter puzzle platform roguelike strategy music simulation racing rpg shooter single-player multi-player"), wxDefaultPosition, wxSize(400,55)), 0, wxTOP, 10);

    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttons, 0, wxTOP, 12);
    buttons->Add(new wxButton(panel, wxID_OK, wxT("Save")));
    buttons->Add(new wxButton(panel, wxID_CANCEL, wxT("Cancel")));
    buttons->Add(new wxHyperlinkCtrl(panel, -1, wxT("(see list of suggested games)"), wxT("http://tiggit.net/suggest.php#list")), 0, wxALIGN_CENTER | wxLEFT, 30);

    panel->Fit();
    Fit();
    Center();

    ok = ShowModal() == wxID_OK;

    if(ok)
      {
        title = wxToStr(t_title->GetValue());
        homepage = wxToStr(t_homepage->GetValue());
        shot = wxToStr(t_shot->GetValue());
        download = wxToStr(t_download->GetValue());
        version = wxToStr(t_version->GetValue());
        devname = wxToStr(t_devname->GetValue());
        tags = wxToStr(t_tags->GetValue());
        desc = wxToStr(t_desc->GetValue());

        if(r_free->GetValue()) type="free";
        else if(r_open->GetValue()) type="open";
        else if(r_demo->GetValue()) type="demo";
        else type="unset";

        fixURL(homepage);
        fixURL(shot);
        fixURL(download);

        fixTags(tags);
      }

    Destroy();
  }

  void fixTags(std::string &tags)
  {
    for(int i=0; i<tags.size(); i++)
      {
        char &c = tags[i];
        if(c >= 'A' && c <= 'Z')
          c += 'a'-'A';
        else if(c==',' || c==':' || c==';') c=' ';
      }
  }

  void fixURL(std::string &url)
  {
    if(url != "" && url.find("://") == std::string::npos)
      url = "http://" + url;
  }
};
