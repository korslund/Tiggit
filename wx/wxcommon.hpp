#ifndef __WX_COMMON_HPP_
#define __WX_COMMON_HPP_

#ifndef wxUSE_UNICODE
#define wxUSE_UNICODE 1
#endif

#include <wx/wx.h>
#include <string>

#define strToWx(x) wxString((x).c_str(), wxConvUTF8)
#define wxToStr(x) std::string((x).mb_str())

#define myID_BOOK 20001
#define myID_BUTTON1 20002
#define myID_BUTTON2 20003
#define myID_GAMEPAGE 20004
#define myID_TEXTVIEW 20005
#define myID_SCREENSHOT 20006
#define myID_LIST 20007
#define myID_RATE 20008
#define myID_TAGS 20009
#define myID_SEARCH_BOX 20010
#define myID_TIGGIT_PAGE 20011
#define myID_OPEN_LOCATION 20012

#endif
