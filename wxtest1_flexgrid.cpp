#include <wx/wx.h>

#include <string>
#include <iostream>
using namespace std;

class Simple : public wxFrame
{
public:
  Simple(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 150))
  {
    Centre();

    /* Old code
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    wxButton *button = new wxButton(panel, wxID_EXIT, wxT("Quit"),
                                    wxPoint(20, 20));

    Connect(wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(Simple::onQuit));
    button->SetFocus();
    */

    // The main sizer
    wxFlexGridSizer *pDialogSizer = new wxFlexGridSizer(1);

    /*
    pDialogSizer->AddGrowableCol(0);
    pDialogSizer->AddGrowableRow(0);
    */

    /* ROW 1 */

    // The text entry box (minimum size = 400x300)
    wxTextCtrl *m_pMyTextEntry = new wxTextCtrl(this, wxID_ANY, wxT("some text"),
                                                wxDefaultPosition, wxSize(400,300));
    pDialogSizer->Add(m_pMyTextEntry, 1,
                           wxBOTTOM | wxGROW | wxALIGN_CENTER | wxFIXED_MINSIZE, 10);

    /* ROW 2 */

    // Add buttons (optional)
    wxBoxSizer *pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton *pButtonA = 
      new wxButton(this, wxID_ANY, wxT("button-a"));
    pButtonsSizer->Add(pButtonA, 0, wxRIGHT, 10);

    pButtonsSizer->AddStretchSpacer(1);

    wxButton *pButtonB = 
        new wxButton(this, wxID_ANY, wxT("button-b"));
    pButtonsSizer->Add(pButtonB, 0, wxRIGHT, 10);

    wxButton *pButtonC = 
        new wxButton(this, wxID_EXIT, wxT("Exit"));
    pButtonsSizer->Add(pButtonC);

    pDialogSizer->Add(pButtonsSizer, 1, wxGROW);

    // Add a 10px border (also optional)
    wxBoxSizer *pDialogSizerOuter = new wxBoxSizer(wxVERTICAL);
      pDialogSizerOuter->Add(pDialogSizer, 1, wxGROW | wxALL, 10);
    SetSizerAndFit(pDialogSizerOuter);

    // Hook up the button
    Connect(wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(Simple::onQuit));
  }

  void onQuit(wxCommandEvent &event)
  {
    Close(true);
  }
};

class MyApp : public wxApp
{
public:
  virtual bool OnInit()
  {
    Simple *simple = new Simple(wxT("Hello world"));
    simple->Show(true);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
