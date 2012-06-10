#include "listbase.hpp"
#include "myids.hpp"

using namespace wxTiggit;

ListBase::ListBase(wxWindow *parent, int id)
  : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
               wxBORDER_SUNKEN | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
{
  Connect(wxEVT_KEY_DOWN,
          wxKeyEventHandler(ListBase::onKeyDown));
}

void ListBase::onKeyDown(wxKeyEvent &evt)
{
  // Capture special keys
  if(evt.GetKeyCode() == WXK_LEFT ||
     evt.GetKeyCode() == WXK_RIGHT ||
     evt.GetKeyCode() == WXK_DELETE)
    {
      // Send the key as a special button press
      wxCommandEvent cmd(wxEVT_COMMAND_BUTTON_CLICKED, myID_SPECIAL_KEY);
      cmd.SetInt(evt.GetKeyCode());
      wxPostEvent(this, cmd);
    }
  else
    evt.Skip();
}
