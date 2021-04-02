#pragma once
#include "util.h"
#include "hook_util.h"
#include "platforms/ea/ea_util.h"

constexpr auto mangled_toStdString = "?toStdString@QByteArray@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";

#ifdef _WIN64

// The docuemented signature is this:
// std::string QByteArray::toStdString() const
// But once again we see the required mystery argument...
// Source: https://doc.qt.io/qt-5/qbytearray.html#toStdString
string* toStdString(PARAMS(void* mystery));

#endif
