#ifndef __IMAGE_VIEWER_HPP
#define __IMAGE_VIEWER_HPP

#include <wx/wx.h>

struct ImageViewer : wxWindow
{
  ImageViewer(wxWindow *parent, wxWindowID id = -1,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize)
    : wxWindow(parent, id, pos, size)
    , bitmap(0,0)
  {}

  void loadImage(const wxImage &image)
  {
    bitmap = wxBitmap(image);
    SetVirtualSize(bitmap.GetWidth(), bitmap.GetHeight());
    Refresh();
  }

  void loadImage(const std::string &file)
  {
    loadImage(wxImage(wxString(file.c_str(), wxConvUTF8)));
  }

  void clear()
  {
    loadImage(wxImage());
  }

protected:
  wxBitmap bitmap;

  void OnPaint(wxPaintEvent &event)
  {
    wxPaintDC dc(this);
    PrepareDC(dc);
    dc.DrawBitmap(bitmap, 0,0, true);
  }

private:
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ImageViewer,wxWindow)
EVT_PAINT(ImageViewer::OnPaint)
END_EVENT_TABLE()

#endif
