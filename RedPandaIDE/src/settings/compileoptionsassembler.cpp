#include "compileoptionsassembler.h"
#include "../compiler/compilerinfo.h"
#include <QVersionNumber>
#include <QDebug>

QMap<QString,QString> CompileOptionsAssembler::assemble(
    const Toolchain& toolchain,
    const BuildConfiguration& buildConfig,
    const QMap<QString,QString>& projectOverrides)
{
    // Start with toolchain-level options
    QMap<QString,QString> result = toolchain.compilerOptions;

    // Override with build-config options (with fallback resolution)
    for (auto it = buildConfig.compilerOptions.begin();
         it != buildConfig.compilerOptions.end(); ++it) {
        const QString& key = it.key();
        PCompilerOption opt = CompilerInfoManager::getCompilerOption(
            toolchain.compilerType, key);
        if (!opt) {
            // Option not valid for this compiler type, skip
            continue;
        }
        QString resolved = resolveOptionValue(toolchain, key, it.value());
        if (resolved.isEmpty()) {
            resolved = opt->defaultValue
                ? QString::number(opt->defaultValue) : QString();
        }
        if (!resolved.isEmpty())
            result[key] = resolved;
    }

    // Override with project options (with fallback resolution)
    for (auto it = projectOverrides.begin();
         it != projectOverrides.end(); ++it) {
        const QString& key = it.key();
        PCompilerOption opt = CompilerInfoManager::getCompilerOption(
            toolchain.compilerType, key);
        if (!opt)
            continue;
        QString resolved = resolveOptionValue(toolchain, key, it.value());
        if (!resolved.isEmpty())
            result[key] = resolved;
    }

    return result;
}

QString CompileOptionsAssembler::resolveOptionValue(
    const Toolchain& toolchain,
    const QString& optionKey,
    const QString& desiredValue)
{
    if (desiredValue.isEmpty())
        return {};

    PCompilerOption option = CompilerInfoManager::getCompilerOption(
        toolchain.compilerType, optionKey);
    if (!option)
        return {};

    // Find the choice matching desiredValue
    const CompilerOptionChoice* foundChoice = nullptr;
    for (const auto& choice : option->choices) {
        if (choice.value == desiredValue) {
            foundChoice = &choice;
            break;
        }
    }

    // If choice found and available, use it
    if (foundChoice && isChoiceAvailable(toolchain, *foundChoice))
        return desiredValue;

    // Try simple fallbackValue chain
    if (foundChoice && !foundChoice->fallbackValue.isEmpty()) {
        QString fallback = resolveOptionValue(
            toolchain, optionKey, foundChoice->fallbackValue);
        if (!fallback.isEmpty())
            return fallback;
    }

    // Try custom fallback function
    if (option->fallbackFunc) {
        QString fallback = option->fallbackFunc(toolchain, desiredValue);
        if (!fallback.isEmpty() && fallback != desiredValue) {
            QString resolved = resolveOptionValue(
                toolchain, optionKey, fallback);
            if (!resolved.isEmpty())
                return resolved;
        }
    }

    // Fall back to default value
    if (option->defaultValue != 0) {
        return QString::number(option->defaultValue);
    }

    // For Choice type, use first available choice
    if (option->type == CompilerOptionType::Choice && !option->choices.isEmpty()) {
        for (const auto& c : option->choices) {
            if (isChoiceAvailable(toolchain, c))
                return c.value;
        }
    }

    return {};
}

bool CompileOptionsAssembler::isChoiceAvailable(
    const Toolchain& toolchain,
    const CompilerOptionChoice& choice)
{
    // Check CompilerType constraint
    if (!choice.availableFor.isEmpty()
        && !choice.availableFor.contains(toolchain.compilerType)) {
        // GCC and Clang are interchangeable
        if (toolchain.compilerType == CompilerType::GCC
            && choice.availableFor.contains(CompilerType::Clang))
            ; // OK
        else if (toolchain.compilerType == CompilerType::Clang
                 && choice.availableFor.contains(CompilerType::GCC))
            ; // OK
        else
            return false;
    }

    // Check minimum version constraint
    if (!choice.minVersion.isEmpty()) {
        QVersionNumber toolVer = QVersionNumber::fromString(toolchain.version);
        QVersionNumber minVer = QVersionNumber::fromString(choice.minVersion);
        if (toolVer < minVer)
            return false;
    }

    return true;
}
