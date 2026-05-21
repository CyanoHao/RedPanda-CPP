#ifndef TOOLCHAIN_DETECT_H
#define TOOLCHAIN_DETECT_H

#include "toolchain.h"
#include "../utils/toolchain.h"
#include <QString>
#include <QSet>
#include <QList>

namespace ToolchainDetect {

CompilerIdentity getCompilerIdentity(const QString& folder,
                                     const GnuToolchainName& c_prog);
bool detectProperties(Toolchain& tc, const QString& binDir,
                      const GnuToolchainName& c_prog);
bool detectProperties(Toolchain& tc, const QString& binDir,
                      const GnuToolchainName& c_prog,
                      const CompilerIdentity& preIdentity);
void detectExecutables(Toolchain& tc, const GnuToolchainName& c_prog);
void detectDirectories(Toolchain& tc, const QString& binDir,
                       const GnuToolchainName& c_prog);

QList<Toolchain> discoverClangTriplets(
    const QString& folder,
    const GnuToolchainName& c_prog,
    const QSet<QString>& existingKeys,
    const CompilerIdentity& preIdentity);

} // namespace ToolchainDetect

#endif // TOOLCHAIN_DETECT_H
