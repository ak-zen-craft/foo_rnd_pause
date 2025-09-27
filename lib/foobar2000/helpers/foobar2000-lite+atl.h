#pragma once

#include "../SDK/foobar2000-winver.h"

#ifdef _WIN32
#define _SECURE_ATL 1
#endif

#include "../SDK/foobar2000-lite.h"

#ifdef _WIN32
#include <atlbase.h>
#include <atltypes.h>
#include <atlstr.h>
#include <WTL10_10320_Release/Include/atlapp.h>
#include <WTL10_10320_Release/Include/atlctrls.h>
#include <atlwin.h>
#include <atlcom.h>
#include <WTL10_10320_Release/Include/atlcrack.h>
#endif

#pragma once
