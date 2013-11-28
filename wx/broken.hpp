/* This is a drop-in include for gametab.cpp, not meant to be used
   separately.
 */
struct ProblemDialog : wxDialog
{
  bool ok;

  std::string comment;

  ProblemDialog(wxWindow *parent, const wxString &gameTitle)
    : wxDialog(parent, -1, wxT("Report Broken Download"), wxDefaultPosition,
               wxSize(300,300))
  {
    wxPanel *panel;
    wxBoxSizer *sizer;

    panel = new wxPanel(this);

    // Set up the sizers
    wxBoxSizer *outer = new wxBoxSizer(wxVERTICAL);
    sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(outer);
    outer->Add(sizer, 1, wxGROW | wxALL, 20);

    wxTextCtrl *t_comment;

    sizer->Add(new wxStaticText(panel, -1, wxT("Game: ") + gameTitle));
    sizer->Add(new wxStaticText(panel, -1, wxT("Describe the issue (required):")), 0, wxTOP, 6);
    t_comment = new wxTextCtrl(panel, -1, wxT(""), wxDefaultPosition, wxSize(260,80),
                               wxTE_MULTILINE);
    sizer->Add(t_comment);

    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttons, 0, wxTOP, 12);
    buttons->Add(new wxButton(panel, wxID_OK, wxT("OK")));
    buttons->Add(new wxButton(panel, wxID_CANCEL, wxT("Cancel")));

    panel->Fit();
    Fit();
    Center();

    ok = ShowModal() == wxID_OK;

    if(ok)
      {
        comment = wxToStr(t_comment->GetValue());

        // Not filling out the description is considered the same as
        // clicking cancel
        if(comment == "") ok = false;
      }

    Destroy();
  }
};
