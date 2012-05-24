#include "dialogs.hpp"

#define myID_BROWSE 12345

BrowseDialog::BrowseDialog(wxWindow *parent, const wxString &title,
                           int width, int height, bool file)
  : wxDialog(parent, -1, title, wxDefaultPosition, wxSize(width, height)),
    browseFile(file)
{
  Connect(myID_BROWSE, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(BrowseDialog::onBrowse));
}

void BrowseDialog::addTo(wxSizer *sizer, const wxString &value)
{
  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
  edit = new wxTextCtrl(this, -1, value, wxDefaultPosition, wxSize(300,24));
  hbox->Add(edit, 0, wxTOP, 2);
  hbox->Add(new wxButton(this, myID_BROWSE, wxT("Browse")), 0, wxLEFT, 5);
  sizer->Add(hbox);
}

void BrowseDialog::onBrowse(wxCommandEvent &event)
{
  wxString dir = edit->GetValue();
  if(browseFile)
    dir = wxFileSelector(wxT("Chose file"), dir);
  else
    dir = wxDirSelector(wxT("Choose directory"), dir);
  if(!dir.empty())
    edit->ChangeValue(dir);
}

OutputDirDialog::OutputDirDialog(wxWindow *parent, const std::string &old_dir)
  : BrowseDialog(parent, wxT("Set Data Directory"), 450, 280)
{
  wxString old(old_dir.c_str(), wxConvUTF8);

  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

  vbox->Add(new wxStaticText(this, -1, wxT("Instructions over several lines. Blah blah blah blah.\nAnd more blah blah blah blah. Did I say\nblah?")),
            0, wxBOTTOM, 30);

  vbox->Add(new wxStaticText(this, -1, wxT("New data directory:")));
  addTo(vbox,old);
  vbox->Add(new wxStaticText(this, -1, wxT("Installed games and data will be MOVED to this directory")),0, wxTOP, 12);

  /*
  wxRadioButton *moverb = new wxRadioButton(this, -1, wxT("Move game files"),
                                            wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  vbox->Add(moverb, 0, wxTOP, 15);
  vbox->Add(new wxRadioButton(this, -1, wxT("Keep games in ") + old));
  */
  vbox->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_CENTER | wxTOP, 40);

  wxBoxSizer *main = new wxBoxSizer(wxVERTICAL);
  main->Add(vbox, 0, wxALL, 15);
  SetSizer(main);

  Centre();

  // Run and fetch info
  ok = ShowModal() == wxID_OK;
  //move = moverb->GetValue();
  move = true;
  path = std::string(edit->GetValue().mb_str());
  changed = old_dir != path;

  Destroy(); 
}
/*
ImportDialog::ImportDialog(wxWindow *parent, const std::string &maindir)
  : BrowseDialog(parent, wxT("Add/Import Data"), 450, 280)
{
  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

  wxString old(maindir.c_str(), wxConvUTF8);

  vbox->Add(new wxStaticText(this, -1, wxT("Select a directory that was previously created through\nthe Export option.")),
            0, wxBOTTOM, 30);

  vbox->Add(new wxStaticText(this, -1, wxT("Directory to import:")));
  addTo(vbox);

  // Not really needed, and not sure how to best implement it.
  //wxStaticText *status = new wxStaticText(this, -1, wxT("Found: xyz/installed.json - OK"));
  //status->SetForegroundColour(wxColour(0,128,0));
  //vbox->Add(status, 0, wxLEFT, 3);

  wxRadioButton *rb = new wxRadioButton(this, -1, wxT("Play from this location"),
                                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  vbox->Add(rb, 0, wxTOP, 25);
  vbox->Add(new wxRadioButton(this, -1, wxT("Copy files to ") + old));
  vbox->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_CENTER | wxTOP, 30);

  wxBoxSizer *main = new wxBoxSizer(wxVERTICAL);
  main->Add(vbox, 0, wxALL, 8);
  SetSizer(main);

  Centre();

  // Run and fetch info
  ok = ShowModal() == wxID_OK;
  copy = !rb->GetValue();

  source = std::string(edit->GetValue().mb_str());

  Destroy();
}

ExportDialog::ExportDialog(wxWindow *parent, const std::vector<std::string> &games)
  : BrowseDialog(parent, wxT("Export Data"), 450, 280)
{
  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

  vbox->Add(new wxStaticText(this, -1, wxT("Select a directory where data should be exported. This is useful for\ntransmitting games through USB sticks and similar.\nExported data can be loaded with the Import menu option.")),
            0, wxBOTTOM, 30);

  vbox->Add(new wxStaticText(this, -1, wxT("Destination directory:")));
  addTo(vbox);

  wxRadioButton *rb = new wxRadioButton(this, -1, wxT("Include Tiggit launcher"),
                                            wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  vbox->Add(rb, 0, wxTOP, 25);
  vbox->Add(new wxRadioButton(this, -1, wxT("Copy games only")));
  vbox->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_CENTER | wxTOP, 30);

  wxBoxSizer *main = new wxBoxSizer(wxVERTICAL);
  main->Add(vbox, 0, wxALL, 8);
  SetSizer(main);

  Centre();

  // Run and fetch info
  ok = ShowModal() == wxID_OK;
  launcher = rb->GetValue();
  output = std::string(edit->GetValue().mb_str());

  Destroy();
}
*/
/* Checkbox list for the export dialog:
   http://wiki.wxwidgets.org/WxListCtrl#Implement_wxListCtrl_with_Checkboxes
   WebUpdate has a CheckedListCtrl which we'll steal.
 */
