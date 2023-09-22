#include "runtime.h"

#include <quickjs/quickjs.h>


namespace AddOn {

EsModuleError::EsModuleError(const QString &reason): BaseError(reason) {}

EsModuleRuntime::EsModuleRuntime() :
    mRuntime{},
    mContext{mRuntime.NewContext()}
{
    mRuntime.SetMemoryLimit(16 * 1024 * 1024);
    mRuntime.SetMaxStackSize(1 * 1024 * 1024);
    mRuntime.SetGCThreshold(64 * 1024);
}

RaiiJs::Value EsModuleRuntime::executeScript(const QString &script) {
    return mContext.Eval(script, "<eval>", JS_EVAL_TYPE_GLOBAL);
}

bool EsModuleRuntime::checkApiVersion(int major, int minor) {
    RaiiJs::Value result = executeScript(QStringLiteral("import { apiVersion } from 'main'; return apiVersion;"));
    if (result.IsException()) {
        qDebug() << result.exceptionDetail();
        throw EsModuleError(QStringLiteral("apiVersion is not defined"));
    }

    QJsonObject expectedVersion(result);
    qDebug() << expectedVersion;
    int eMajor = expectedVersion.value("major").toInt();
    int eMinor = expectedVersion.value("minor").toInt();
    if (major == 0 || eMajor == 0)
        // we do not promise backward compatibility for 0.x versions
        return major == eMajor && minor == eMinor;
    return major == eMajor && minor >= eMinor;
}

ThemeRuntime::ThemeRuntime()
{
}

}
