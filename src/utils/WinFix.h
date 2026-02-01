#pragma once

// This header should be included AFTER wx/wx.h but BEFORE our core headers
// It undefines Windows macros that conflict with our code

// Windows defines many macros that conflict with common names
// These must be undefined after including Windows.h (via wxWidgets)

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

#ifdef NEAR
#undef NEAR
#endif

#ifdef FAR
#undef FAR
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef SERVICES
#undef SERVICES
#endif

#ifdef Industry
#undef Industry
#endif

#ifdef Tech
#undef Tech
#endif

#ifdef Health
#undef Health
#endif

// From winsvc.h - service types
#ifdef SERVICE_AUTO_START
// These may conflict
#endif
