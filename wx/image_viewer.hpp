#ifndef __IMAGE_VIEWER_HPP
#define __IMAGE_VIEWER_HPP

#include "wxcommon.hpp"

namespace wxTiggit
{
  struct ImageViewer : wxWindow
  {
    ImageViewer(wxWindow *parent, wxWindowID id = -1,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize)
      : wxWindow(parent, id, pos, size)
      , bitmap(0,0) {}

    void loadImage(const wxImage &image);
    void loadImage(const std::string &file);
    void clear();

  protected:
    wxBitmap bitmap;

    void OnPaint(wxPaintEvent &event);

  private:
    DECLARE_EVENT_TABLE()
  };
}
#endif
