#ifndef API_H
#define API_H

#include <quickjs/quickjs.hpp>
#include "utils.h"

namespace AddOn {

// (string) -> undefined
extern "C" JSValue jsApi_Debug_log(JSContext *, JSValueConst, int, JSValueConst *) noexcept;

// (any) -> undefined
extern "C" JSValue jsApi_Debug_dump(JSContext *, JSValueConst, int, JSValueConst *) noexcept;

// () -> string
extern "C" JSValue jsApi_Desktop_desktopEnvironment(JSContext *, JSValueConst, int, JSValueConst *) noexcept;

// () -> string
extern "C" JSValue jsApi_Desktop_language(JSContext *, JSValueConst, int, JSValueConst *) noexcept;

// () -> [string]
extern "C" JSValue jsApi_Desktop_qtStyleList(JSContext *, JSValueConst, int, JSValueConst *) noexcept;

// () -> string
extern "C" JSValue jsApi_Desktop_systemAppMode(JSContext *, JSValueConst, int, JSValueConst *) noexcept;

}

#endif // API_H
