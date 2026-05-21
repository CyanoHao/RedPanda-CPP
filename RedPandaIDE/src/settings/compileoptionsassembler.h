#ifndef COMPILEOPTIONSASSEMBLER_H
#define COMPILEOPTIONSASSEMBLER_H

#include <QMap>
#include <QString>
#include "toolchain.h"
#include "buildconfig.h"

class CompileOptionsAssembler {
public:
    // Merge options with priority:
    //   1. CompilerInfo defaults (handled by resolveOptionValue fallback)
    //   2. Toolchain.compilerOptions
    //   3. BuildConfig.compilerOptions
    //   4. projectOverrides
    // With value fallback resolution per §3.4
    static QMap<QString,QString> assemble(
        const Toolchain& toolchain,
        const BuildConfiguration& buildConfig,
        const QMap<QString,QString>& projectOverrides
    );

    // Resolve a single option value with recursive fallback
    static QString resolveOptionValue(
        const Toolchain& toolchain,
        const QString& optionKey,
        const QString& desiredValue
    );

    // Check if a choice is available for the given toolchain
    static bool isChoiceAvailable(
        const Toolchain& toolchain,
        const CompilerOptionChoice& choice
    );
};

#endif // COMPILEOPTIONSASSEMBLER_H
