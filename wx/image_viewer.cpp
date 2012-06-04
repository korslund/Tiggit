#include "image_viewer.hpp"

using namespace wxTiggit;

void ImageViewer::loadImage(const wxImage &image)
{
  bitmap = wxBitmap(image);
  SetVirtualSize(bitmap.GetWidth(), bitmap.GetHeight());
  Refresh();
}

void ImageViewer::loadImage(const std::string &file)
{
  loadImage(wxImage(wxString(file.c_str(), wxConvUTF8)));
}

void ImageViewer::clear()
{
  loadImage(wxImage());
}

void ImageViewer::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  PrepareDC(dc);
  dc.DrawBitmap(bitmap, 0,0, true);
}

BEGIN_EVENT_TABLE(ImageViewer,wxWindow)
EVT_PAINT(ImageViewer::OnPaint)
END_EVENT_TABLE()
