#include "gametab.hpp"

#include "image_viewer.hpp"

using namespace wxTiggit;

GameTab::GameTab(wxNotebook *parent, const wxString &name, int listType)
  : TabBase(parent), tabName(name)
{
  wxBoxSizer *searchBox = new wxBoxSizer(wxHORIZONTAL);
  searchBox->Add(new wxStaticText(this, wxID_ANY, wxT("Search:")), 0);
  searchBox->Add(new wxTextCtrl(this, myID_SEARCH_BOX, wxT(""), wxDefaultPosition,
                                wxSize(260,22)),1, wxGROW);

  wxBoxSizer *bcLeft = new wxBoxSizer(wxVERTICAL);
  bcLeft->Add(searchBox, 0, wxBOTTOM | wxLEFT, 2);
  bcLeft->Add(new wxStaticText(this, wxID_ANY, wxString(wxT("Mouse: double-click to ")) +
                               (/*(listType == ListKeeper::SL_INSTALL)*/false?
                                wxT("play"):wxT("install")) +
                               wxT("\nKeyboard: arrow keys + enter, delete")),
              0, wxLEFT | wxBOTTOM, 4);

  wxBoxSizer *bottomCenter = new wxBoxSizer(wxHORIZONTAL);
  bottomCenter->Add(bcLeft, 1, wxGROW);

  textView = new wxTextCtrl
    (this, myID_TEXTVIEW, wxT(""), wxDefaultPosition, wxDefaultSize,
     wxBORDER_NONE | wxTE_MULTILINE | wxTE_READONLY | wxTE_AUTO_URL | wxTE_RICH);

  screenshot = new ImageViewer(this, myID_SCREENSHOT, wxDefaultPosition,
                               wxSize(300,260));

  b1 = new wxButton(this, myID_BUTTON1, wxT("No action"));
  b2 = new wxButton(this, myID_BUTTON2, wxT("No action"));

  wxBoxSizer *buttonBar = new wxBoxSizer(wxHORIZONTAL);
  buttonBar->Add(b1, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);
  buttonBar->Add(b2, 0, wxTOP | wxBOTTOM | wxRIGHT, 3);

  wxBoxSizer *buttonHolder = new wxBoxSizer(wxVERTICAL);
  buttonHolder->Add(buttonBar, 0);
  buttonHolder->Add(new wxButton(this, myID_GAMEPAGE, wxT("Game Website")),
                    0, wxTOP | wxBOTTOM, 3);

  wxString choices[7];
  choices[0] = wxT("Rate this game");
  choices[1] = wxT("5: Awesome!");
  choices[2] = wxT("4: Very Good");
  choices[3] = wxT("3: It's OK");
  choices[4] = wxT("2: Meh");
  choices[5] = wxT("1: Very Bad");
  choices[6] = wxT("0: Unplayable");

  rateString[0] = wxT("Your rating: not rated");
  rateString[1] = wxT("Your rating: 0 (unplayable)");
  rateString[2] = wxT("Your rating: 1 (very bad)");
  rateString[3] = wxT("Your rating: 2 (meh)");
  rateString[4] = wxT("Your rating: 3 (ok)");
  rateString[5] = wxT("Your rating: 4 (very good)");
  rateString[6] = wxT("Your rating: 5 (awesome)");

  rateBox = new wxChoice(this, myID_RATE, wxDefaultPosition, wxDefaultSize,
                         7, choices);
  rateText = new wxStaticText(this, wxID_ANY, wxT(""));

  wxBoxSizer *rateBar = new wxBoxSizer(wxHORIZONTAL);
  rateBar->Add(rateBox, 0, wxRIGHT, 5);
  rateBar->Add(rateText, 0, wxTOP | wxLEFT, 6);

  tags = new wxListBox(this, myID_TAGS);

  wxBoxSizer *bottomLeft = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *rightPane = new wxBoxSizer(wxVERTICAL);
  rightPane->Add(screenshot, 0, wxTOP, 5);
  rightPane->Add(rateBar);
  rightPane->Add(textView, 1, wxGROW | wxTOP | wxRIGHT, 7);

  wxBoxSizer *topPart = new wxBoxSizer(wxHORIZONTAL);
  topPart->Add(tags, 30, wxGROW);
  //topPart->Add(list, 100, wxGROW | wxRIGHT, 10);
  topPart->Add(new wxListBox(this, -1), 100, wxGROW | wxRIGHT, 10);
  topPart->Add(rightPane, 60, wxGROW | wxBOTTOM, 2);

  wxBoxSizer *bottomPart = new wxBoxSizer(wxHORIZONTAL);
  bottomPart->Add(bottomLeft, 30, wxGROW);
  bottomPart->Add(bottomCenter, 100, wxGROW | wxTOP, 5);
  bottomPart->Add(buttonHolder, 60, wxGROW | wxTOP, 5);

  wxBoxSizer *parts = new wxBoxSizer(wxVERTICAL);
  parts->Add(topPart, 1, wxGROW);
  parts->Add(bottomPart, 0, wxGROW | wxTOP, 1);

  SetSizer(parts);
}

void GameTab::gotFocus()
{
}

wxString GameTab::getTitle()
{
  return tabName + wxT(" (10)");
}
