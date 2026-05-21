/*
 * Copyright (C) 2020-2026 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "parser.h"
#include "../parser/cppparser.h"
#include "../settings/toolchain.h"
#include "../settings/buildconfig.h"
#include "../mainwindow.h"

void resetCppParser(std::shared_ptr<CppParser> parser, const Toolchain& toolchain, const BuildConfiguration& buildConfig)
{
    if (!parser)
        return;
    // Configure parser
    parser->resetParser();
    parser->setEnabled(true);

#ifdef ENABLE_SDCC
    if (toolchain.compilerType == CompilerType::SDCC)
        parser->setLanguage(ParserLanguage::SDCC);
#endif
    parser->clearIncludePaths();
    bool isCpp = parser->language()==ParserLanguage::CPlusPlus;
    if (isCpp) {
        foreach  (const QString& file, toolchain.cppIncludeDirs) {
            parser->addIncludePath(file);
        }
    }
    foreach  (const QString& file, toolchain.cIncludeDirs) {
        parser->addIncludePath(file);
    }
    if (isCpp) {
        foreach  (const QString& file, toolchain.defaultCppIncludeDirs()) {
            parser->addIncludePath(file);
        }
    }
    foreach  (const QString& file, toolchain.defaultCIncludeDirs()) {
        parser->addIncludePath(file);
    }
    // Set defines - merge toolchain and build config options
    QMap<QString,QString> mergedOptions = toolchain.compilerOptions;
    for (auto it = buildConfig.compilerOptions.cbegin(); it != buildConfig.compilerOptions.cend(); ++it)
        mergedOptions[it.key()] = it.value();
    foreach (const QString &define, toolchain.defines(parser->language()==ParserLanguage::CPlusPlus, mergedOptions)) {
        parser->addHardDefineByLine(define);
    }
    // add a Red Pand C++ 's own macro
//        parser->addHardDefineByLine("#define EGE_FOR_AUTO_CODE_COMPLETETION_ONLY");
    // add C/C++ default macro
    parser->addHardDefineByLine("#define __FILE__  1");
    parser->addHardDefineByLine("#define __LINE__  1");
    parser->addHardDefineByLine("#define __DATE__  1");
    parser->addHardDefineByLine("#define __TIME__  1");

    parser->parseHardDefines();
    pMainWindow->disconnect(parser.get(),
                            &CppParser::parseStarted,
                            pMainWindow,
                            &MainWindow::onParseStarted);
    pMainWindow->disconnect(parser.get(),
                            &CppParser::progress,
                            pMainWindow,
                            &MainWindow::onParserProgress);
    pMainWindow->disconnect(parser.get(),
                            &CppParser::parseFinished,
                            pMainWindow,
                            &MainWindow::onParseFinished);
    pMainWindow->connect(parser.get(),
                            &CppParser::parseStarted,
                            pMainWindow,
                            &MainWindow::onParseStarted);
    pMainWindow->connect(parser.get(),
                            &CppParser::progress,
                            pMainWindow,
                            &MainWindow::onParserProgress);
    pMainWindow->connect(parser.get(),
                            &CppParser::parseFinished,
                            pMainWindow,
                            &MainWindow::onParseFinished);
}
