#ifndef __IMAGE_VIEWER_CLICK_HPP
#define __IMAGE_VIEWER_CLICK_HPP

#include "image_viewer.hpp"

namespace wxTiggit
{
  struct ClickableImage : ImageViewer
  {
    wxString url;

    ClickableImage(wxWindow *parent, wxWindowID id = -1,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize)
      : ImageViewer(parent, id, pos, size)
    {
      Connect(id, wxEVT_LEFT_DOWN,
              wxMouseEventHandler(ClickableImage::onClick));
      /*
      Connect(id, wxEVT_MOTION,
              wxMouseEventHandler(ClickableImage::onMotion));
      */
    }

    void setData(const std::string &file, const std::string &_url)
    {
      url = strToWx(_url);

      if(file.size()) loadImage(file);
      else clear();

      if(url.size()) SetCursor(wxCURSOR_HAND);
      else SetCursor(wxNullCursor);
    }

  private:
    void onClick(wxMouseEvent &event)
    {
      if(url.size())
        {
          wxLaunchDefaultBrowser(url);
          return;
        }
    }
    /*
    void onMotion(wxMouseEvent &event)
    {
      long x, y;
      event.GetPosition(&x, &y);
      printf("X:%ld Y:%ld\n", x, y);
      event.Skip();
    }
    */
  };
}
#endif
