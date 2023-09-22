#include <QStyleFactory>

#include "api.h"

#include "settings.h"
#include "thememanager.h"

namespace AddOn {

extern "C" JSValue jsApi_Debug_log(JSContext *ctx, JSValueConst, int argc, JSValueConst *argv) noexcept {
    if (argc < 1) {
        return JS_UNDEFINED;
    }
    JSValueConst msg = argv[0];
    try {
        QString text = RawJs::ToQString(ctx, msg);
        qDebug() << text;
    } catch (const RawJs::TypeError &e) {
        qDebug() << e.what();
    }
    return JS_UNDEFINED;
}

extern "C" JSValue jsApi_Debug_dump(JSContext *ctx, JSValueConst, int argc, JSValueConst *argv) noexcept {
    if (argc < 1) {
        return JS_UNDEFINED;
    }
    JSValueConst jsObj = argv[0];
    QJsonValue obj;
    try {
        obj = RawJs::ToQJsonValue(ctx, jsObj);
        qDebug() << obj;
    } catch (const RawJs::TypeError &e) {
        qDebug() << e.what();
    }
    return JS_UNDEFINED;
}

extern "C" JSValue jsApi_Desktop_desktopEnvironment(JSContext *ctx, JSValueConst, int, JSValueConst *) noexcept {
#if defined(Q_OS_WIN32)
    // exclude WinRT intentionally
    return RawJs::NewString(ctx, "windows");
#elif defined(Q_OS_MACOS)
    return RawJs::NewString(ctx, "macos");
#elif (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_HURD) || defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD) || defined(Q_OS_SOLARIS)
    // desktops that follows to freedesktop.org specs, i.e. GNU/Linux, GNU/Hurd, BSD, Solaris (illumos)
    return RawJs::NewString(ctx, "xdg");
#else
    return RawJs::NewString(ctx, "unknown");
#endif
}

extern "C" JSValue jsApi_Desktop_language(JSContext *ctx, JSValueConst, int, JSValueConst *) noexcept {
    return RawJs::NewString(ctx, pSettings->environment().language());
}

extern "C" JSValue jsApi_Desktop_qtStyleList(JSContext *ctx, JSValueConst, int, JSValueConst *) noexcept {
    return RawJs::NewArray(ctx, QStyleFactory::keys());
}

extern "C" JSValue jsApi_Desktop_systemAppMode(JSContext *ctx, JSValueConst, int, JSValueConst *) noexcept {
    return RawJs::NewString(ctx, AppTheme::isSystemInDarkMode() ? "dark" : "light");
}

}
