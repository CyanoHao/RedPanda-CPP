#ifndef ADDON_RUNTIME_H
#define ADDON_RUNTIME_H

#include <memory>

#include <quickjs/quickjs.hpp>

#include "utils.h"

namespace AddOn {

class EsModuleError : public BaseError {
public:
    explicit EsModuleError(const QString &reason);
};

class EsModuleRuntime {
protected:
    EsModuleRuntime();
    EsModuleRuntime(const EsModuleRuntime &) = delete;
    EsModuleRuntime(EsModuleRuntime &&) = default;
    EsModuleRuntime &operator=(const EsModuleRuntime &) = delete;
    EsModuleRuntime &operator=(EsModuleRuntime &&) = default;
    virtual ~EsModuleRuntime() = default;

    void loadEsModule(const QByteArray &text, const QString &moduleName) {
        RaiiJs::Value val = mContext.Eval(text, moduleName, JS_EVAL_TYPE_MODULE);
    }
    void loadMainModule(const QByteArray &text) {
        loadEsModule(text, "main");
    }

    RaiiJs::Value executeScript(const QString &script);

    bool checkApiVersion(int major, int minor);

    void callExportDefault() {}

    void callExportFunction() {}

private:
    RaiiJs::Runtime mRuntime;
    RaiiJs::Context mContext;
};

class ThemeRuntime : protected EsModuleRuntime {
public:
    ThemeRuntime();
    ThemeRuntime(const ThemeRuntime &) = delete;
    ThemeRuntime(ThemeRuntime &&) = default;
    ThemeRuntime &operator=(const ThemeRuntime &) = delete;
    ThemeRuntime &operator=(ThemeRuntime &&) = default;
    ~ThemeRuntime() = default;

    void loadTheme(const QByteArray &source, const QString &filename) {
        loadEsModule(source, filename);
        checkApiVersion(0, 1);
    }
    QJsonObject getTheme() { return {}; }
};

}

#endif // ADDON_RUNTIME_H
