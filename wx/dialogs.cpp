#include "dialogs.hpp"

using namespace wxTiggit;

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

OutputDirDialog::OutputDirDialog(wxWindow *parent, const std::string &old_dir,
                                 const std::string &legacy_dir,
                                 bool writeFailed, bool freshInstall)
  : BrowseDialog(parent, wxT("Select Data Directory"), 430, 256)
{
  wxString old(old_dir.c_str(), wxConvUTF8);

  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

  wxString text = wxT("Select a data directory. This is where all games, configuration and data\nwill be stored.");

  vbox->Add(new wxStaticText(this, -1, text), 0, wxBOTTOM, 15);

  vbox->Add(new wxStaticText(this, -1, wxT("Data directory:")), 0, wxBOTTOM, 3);
  addTo(vbox,old);
  if(writeFailed)
    {
      wxStaticText *t = new wxStaticText(this, -1, wxT("The path you specified is not writable!"));
      t->SetForegroundColour(wxColour(255,0,0));
      vbox->Add(t);
    }

  if(freshInstall)
    text = wxT("The directory will be created if it does not already exist.\nPress Cancel to exit.");
  else
    text = wxT("WARNING: This will MOVE all installed games, savegames and\nTiggit configuration to this location.");
  vbox->Add(new wxStaticText(this, -1, text),0, wxTOP, 12);

  wxCheckBox *import = NULL;
  if(legacy_dir != "")
    {
      assert(freshInstall);

      import = new wxCheckBox(this, -1, strToWx("Import games from " + legacy_dir + " \n(files are moved to the new directory)"));
      import->SetValue(true);
      vbox->Add(import, 0, wxTOP, 10);
    }

  vbox->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_CENTER | wxTOP, 20);

  wxBoxSizer *main = new wxBoxSizer(wxVERTICAL);
  main->Add(vbox, 0, wxALL, 15);
  SetSizer(main);

  // This dialog is kinda dynamic in content, so better to ask it to
  // resize itself depending on contained elements.
  Fit();

  Centre();

  // Run and fetch info
  ok = ShowModal() == wxID_OK;
  //move = moverb->GetValue();

  doImport = false;
  if(legacy_dir != "" && import)
    doImport = import->GetValue();

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
