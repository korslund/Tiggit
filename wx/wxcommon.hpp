#ifndef __WX_COMMON_HPP_
#define __WX_COMMON_HPP_

#ifndef wxUSE_UNICODE
#define wxUSE_UNICODE 1
#endif

#include <wx/wx.h>
#include <string>

#define strToWx(x) wxString((x).c_str(), wxConvUTF8)
#define wxToStr(x) std::string((x).mb_str())

#endif
