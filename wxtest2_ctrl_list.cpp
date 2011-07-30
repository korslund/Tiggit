#include <wx/wx.h>
#include <wx/hyperlink.h>
//#include <wx/headerctrl.h>

#include <string>
#include <iostream>
using namespace std;

class CtrlList : public wxScrolledWindow
{
public:
  CtrlList(wxWindow *parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxHSCROLL | wxVSCROLL | wxBORDER_SUNKEN)
  {
    SetScrollRate(10,10);

    /* This should give us a listview-like header, but as far as I can
       see, this is new in wx2.9, we have 2.8.

    wxHeaderCtrlSimple * header = new wxHeaderCtrlSimple(this);
    wxHeaderColumnSimple col("Title");
    col.SetWidth(100);
    col.SetSortable(100);
    header->AppendColumn(col);
    */

    // The main sizer
    wxFlexGridSizer *flex = new wxFlexGridSizer(3, 2, 10);

    flex->SetFlexibleDirection(wxHORIZONTAL);
    /*
    flex->AddGrowableCol(0, 0);
    flex->AddGrowableCol(1, 0);
    flex->AddGrowableCol(2, 0);
    */

    wxColour white( 255, 255, 255 );

    for(int i=0; i<8; i++)
      {
        // Just add elements linearly, and the grid size will sort
        // them into columns for us

        const int FLAGS =  wxBOTTOM | wxGROW | wxALIGN_CENTER | wxFIXED_MINSIZE;

        flex->Add(new wxStaticText(this, wxID_ANY, wxT("This is a string")), 1, FLAGS);

        flex->Add(new wxStaticText(this, wxID_ANY, wxT("Another string")), 1, FLAGS);

        /*
        flex->Add(new wxTextCtrl(this, wxID_ANY, wxT("some text"),
                                         wxDefaultPosition, wxSize(200,25)),
                          1, FLAGS);
        */

        //flex->Add(new wxButton(this, wxID_ANY, wxT("button-a")), 1, FLAGS);

        wxWindow *link = new wxHyperlinkCtrl(this, wxID_ANY, wxT("Home page"),
                                             wxT("http://tiggit.net/"));
        link->SetBackgroundColour( white );
        flex->Add(link, 1, FLAGS);
      }

    // Tell the scroll window how large we are
    SetSizer(flex);
    SetBackgroundColour( white );
  }
};

class MyFrame : public wxFrame
{
public:
  MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(600, 300))
  {
    Centre();

    CtrlList *list = new CtrlList(this);

    // Add a 10px border
    wxBoxSizer *border = new wxBoxSizer(wxVERTICAL);
    border->Add(list, 1, wxGROW | wxALL, 10);

    SetSizer(border);
  }
};

class MyApp : public wxApp
{
public:
  virtual bool OnInit()
  {
    MyFrame *frame = new MyFrame(wxT("Custom list"));
    frame->Show(true);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
