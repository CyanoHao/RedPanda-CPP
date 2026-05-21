#include "compilerinfo.h"
#include <QObject>
#include <QDebug>

CompilerInfo::CompilerInfo(const QString &name):
    mName(name)
{
}

const QList<PCompilerOption> &CompilerInfo::compilerOptions() const
{
    return mCompilerOptionList;
}

const QString &CompilerInfo::name() const
{
    return mName;
}

PCompilerOption CompilerInfo::getCompilerOption(const QString &key) const
{
    return mCompilerOptions.value(key,PCompilerOption());
}

bool CompilerInfo::hasCompilerOption(const QString &key) const
{
    return mCompilerOptions.contains(key);
}

bool CompilerInfo::supportSyntaxCheck()
{
    return true;
}

PCompilerOption CompilerInfo::addOption(const QString &key, const QString &name,
                             const QString section, bool isC, bool isCpp, bool isLinker, const QString &setting,
                             CompilerOptionType type, const CompileOptionChoiceList &choices)
{
    Q_ASSERT(choices.isEmpty() || type == CompilerOptionType::Choice);
    PCompilerOption pOption = std::make_shared<CompilerOption>();
    pOption->key = key;
    pOption->name = name;
    pOption->section = section;
    pOption->isC = isC;
    pOption->isCpp = isCpp;
    pOption->isLinker = isLinker;
    pOption->setting= setting;
    pOption->type = type;
    pOption->choices = choices;
    pOption->scale = 1;
    mCompilerOptions.insert(key,pOption);
    mCompilerOptionList.append(pOption);
    return pOption;
}

PCompilerOption CompilerInfo::addNumberOption(const QString &key, const QString &name, const QString section, bool isC, bool isCpp, bool isLinker, const QString &setting, const QString &suffix, int scale, int defaultValue, int minValue, int maxValue)
{
    PCompilerOption pOption = std::make_shared<CompilerOption>();
    pOption->key = key;
    pOption->name = name;
    pOption->section = section;
    pOption->isC = isC;
    pOption->isCpp = isCpp;
    pOption->isLinker = isLinker;
    pOption->setting= setting;
    pOption->type = CompilerOptionType::Number;
    pOption->suffix = suffix;
    pOption->scale = scale;
    pOption->defaultValue = defaultValue;
    pOption->minValue = minValue;
    pOption->maxValue = maxValue;
    mCompilerOptions.insert(key,pOption);
    mCompilerOptionList.append(pOption);
    return pOption;
}

void CompilerInfo::init()
{
    prepareCompilerOptions();
}

void CompilerInfo::prepareCompilerOptions()
{
    CompileOptionChoiceList sl;
    QString groupName;
//    // C options
//    groupName = QObject::tr("C options");
//    addOption(CC_CMD_OPT_ANSI, QObject::tr("Support all ANSI standard C programs (-ansi)"), groupName, true, true, false, "-ansi");
//    addOption(CC_CMD_OPT_NO_ASM, QObject::tr("Do not recognize asm,inline or typeof as a keyword (-fno-asm)"), groupName, true, true, false, "-fno-asm");
//    addOption(CC_CMD_OPT_TRADITIONAL_CPP, QObject::tr("Imitate traditional C preprocessors (-traditional-cpp)"), groupName, true, true, false, "-traditional-cpp");

    groupName = QObject::tr("Code Generation");
    // Optimization
    sl.clear();
    sl.append(CompilerOptionChoice("Low (-O1)","1"));
    sl.append(CompilerOptionChoice("Med (-O2)","2"));
    sl.append(CompilerOptionChoice("High (-O3)","3"));
    sl.append(CompilerOptionChoice("Highest (-Ofast)","fast"));
    sl.append(CompilerOptionChoice("Size (-Os)","s"));
    sl.append(CompilerOptionChoice("Debug (-Og)","g"));
    addOption(CC_CMD_OPT_OPTIMIZE, QObject::tr("Optimization level (-Ox)"), groupName, true, true, false, "-O", CompilerOptionType::Choice, sl);

    // C++ Language Standards
    sl.clear();
    sl.append(CompilerOptionChoice("ISO C++","c++98"));
    sl.append(CompilerOptionChoice("ISO C++11","c++11"));
    sl.append(CompilerOptionChoice("ISO C++14","c++14"));
    sl.append(CompilerOptionChoice("ISO C++17","c++17"));
    sl.append(CompilerOptionChoice("ISO C++20","c++2a"));
    sl.append(CompilerOptionChoice("ISO C++23","c++2b"));
    sl.append(CompilerOptionChoice("ISO C++26","c++2c"));
    sl.append(CompilerOptionChoice("GNU C++","gnu++98"));
    sl.append(CompilerOptionChoice("GNU C++11","gnu++11"));
    sl.append(CompilerOptionChoice("GNU C++14","gnu++14"));
    sl.append(CompilerOptionChoice("GNU C++17","gnu++17"));
    sl.append(CompilerOptionChoice("GNU C++20","gnu++2a"));
    sl.append(CompilerOptionChoice("GNU C++23","gnu++2b"));
    sl.append(CompilerOptionChoice("GNU C++26","gnu++2c"));
    addOption(CC_CMD_OPT_STD, QObject::tr("C++ Language standard (-std)"), groupName, false, true, false, "-std=",CompilerOptionType::Choice, sl)
        ->level = CompilerOptionLevel::Toolchain;

    sl.clear();
    sl.append(CompilerOptionChoice("ISO C90","c90"));
    sl.append(CompilerOptionChoice("ISO C99","c99"));
    sl.append(CompilerOptionChoice("ISO C11","c11"));
    sl.append(CompilerOptionChoice("ISO C17","c17"));
    sl.append(CompilerOptionChoice("ISO C23","c2x"));
    sl.append(CompilerOptionChoice("ISO C2Y","c2y"));
    sl.append(CompilerOptionChoice("GNU C90","gnu90"));
    sl.append(CompilerOptionChoice("GNU C99","gnu99"));
    sl.append(CompilerOptionChoice("GNU C11","gnu11"));
    sl.append(CompilerOptionChoice("GNU C17","gnu17"));
    sl.append(CompilerOptionChoice("GNU C23","gnu2x"));
    sl.append(CompilerOptionChoice("GNU C2Y","gnu2y"));
    addOption(C_CMD_OPT_STD, QObject::tr("C Language standard (-std)"), groupName, true, false, false, "-std=", CompilerOptionType::Choice, sl)
        ->level = CompilerOptionLevel::Toolchain;

    // Optimization for cpu type
//    sl.clear();
//    sl.append(CompilerOptionChoice(QObject::tr("This CPU"),"native"));
//    sl.append(CompilerOptionChoice("i386","i386"));
//    sl.append(CompilerOptionChoice("i486","i486"));
//    sl.append(CompilerOptionChoice("i586","i586"));
//    sl.append(CompilerOptionChoice("i686","i686"));
//    sl.append(CompilerOptionChoice("Pentium","pentium"));
//    sl.append(CompilerOptionChoice("Pentium MMX","pentium-mmx"));
//    sl.append(CompilerOptionChoice("Pentium Pro","pentiumpro"));
//    sl.append(CompilerOptionChoice("Pentium 2","pentium2"));
//    sl.append(CompilerOptionChoice("Pentium 3","pentium3"));
//    sl.append(CompilerOptionChoice("Pentium 4","pentium4"));
//    sl.append(CompilerOptionChoice("Conroe","core2"));
//    sl.append(CompilerOptionChoice("Nehalem","corei7"));
//    sl.append(CompilerOptionChoice("Sandy","corei7-avx"));
//    sl.append(CompilerOptionChoice("K6","k6"));
//    sl.append(CompilerOptionChoice("K6-2","k6-2"));
//    sl.append(CompilerOptionChoice("K6-3","k6-3"));
//    sl.append(CompilerOptionChoice("Athlon","athlon"));
//    sl.append(CompilerOptionChoice("Athlon Tbird","athlon-tbird"));
//    sl.append(CompilerOptionChoice("Athlon 4","athlon-4"));
//    sl.append(CompilerOptionChoice("Athlon XP","athlon-xp"));
//    sl.append(CompilerOptionChoice("Athlon MP","athlon-mp"));
//    sl.append(CompilerOptionChoice("K8","k8"));
//    sl.append(CompilerOptionChoice("K8 Rev.E","k8-sse3"));
//    sl.append(CompilerOptionChoice("K10","barcelona"));
//    sl.append(CompilerOptionChoice("Bulldozer","bdver1"));
//    addOption(CC_CMD_OPT_ARCH, QObject::tr("Optimize for the following machine (-march)"), groupName, true, true, false, "-march=", sl);
//    addOption(CC_CMD_OPT_TUNE, QObject::tr("Optimize less, while maintaining full compatibility (-tune)"), groupName, true, true, false, "-mtune=", sl);

    // Enable use of the specific instructions
    sl.clear();
    sl.append(CompilerOptionChoice("MMX","mmx"));
    sl.append(CompilerOptionChoice("3D Now","3dnow"));
    sl.append(CompilerOptionChoice("SSE","sse"));
    sl.append(CompilerOptionChoice("SSE2","sse2"));
    sl.append(CompilerOptionChoice("SSE3","sse3"));
    sl.append(CompilerOptionChoice("SSSE3","ssse3"));
    sl.append(CompilerOptionChoice("SSE4","sse4"));
    sl.append(CompilerOptionChoice("SSE4A","sse4a"));
    sl.append(CompilerOptionChoice("SSE4.1","sse4.1"));
    sl.append(CompilerOptionChoice("SSE4.2","sse4.2"));
    sl.append(CompilerOptionChoice("AVX","avx"));
    sl.append(CompilerOptionChoice("AVX2","avx2"));
    sl.append(CompilerOptionChoice("FMA4","fma4"));
    sl.append(CompilerOptionChoice("XOP","xop"));
    sl.append(CompilerOptionChoice("AES","aes"));
    addOption(CC_CMD_OPT_INSTRUCTION,QObject::tr("Enable use of specific instructions (-mx)"), groupName, true, true, false, "-m", CompilerOptionType::Choice, sl)
        ->level = CompilerOptionLevel::Toolchain;

    // 32bit/64bit
    sl.clear();
    sl.append(CompilerOptionChoice(QObject::tr("32-bit pointer, 32-bit instruction (-m32)"), "32"));
    sl.append(CompilerOptionChoice(QObject::tr("32-bit pointer, 64-bit instruction (-mx32)"), "x32"));
    sl.append(CompilerOptionChoice(QObject::tr("64-bit pointer, 64-bit instruction (-m64)"), "64"));
    addOption(CC_CMD_OPT_POINTER_SIZE, QObject::tr("x86 multilib (-mx)"), groupName, true, true, true, "-m", CompilerOptionType::Choice, sl)
        ->level = CompilerOptionLevel::Toolchain;

    addOption(CC_CMD_OPT_DEBUG_INFO, QObject::tr("Generate debugging information (-g3)"), groupName, true, true, false, "-g3");
    addOption(CC_CMD_OPT_PROFILE_INFO, QObject::tr("Generate profiling info for analysis (-pg)"), groupName, true, true, true, "-pg");
    addOption(CC_CMD_OPT_SYNTAX_ONLY, QObject::tr("Only check the code for syntax errors (-fsyntax-only)"), groupName, true, true, false, "-fsyntax-only");
    addOption(CC_CMD_OPT_ENABLE_GCC_IMPORT_STD, QObject::tr("Enable experimental support for GCC standard library modules (-fmodules)"), groupName, false, true, false, "-fmodules")
        ->level = CompilerOptionLevel::Toolchain;

    // Warnings
    groupName = QObject::tr("Warnings");
    addOption(CC_CMD_OPT_INHIBIT_ALL_WARNING, QObject::tr("Inhibit all warning messages (-w)"), groupName, true, true, false, "-w");
    addOption(CC_CMD_OPT_WARNING_ALL,QObject::tr("Show most warnings (-Wall)"), groupName, true, true, false, "-Wall");
    addOption(CC_CMD_OPT_WARNING_EXTRA,QObject::tr("Show some more warnings (-Wextra)"), groupName, true, true, false, "-Wextra");
    addOption(CC_CMD_OPT_CHECK_ISO_CONFORMANCE, QObject::tr("Check ISO C/C++ conformance (-pedantic)"), groupName, true, true, false, "-pedantic");
    addOption(CC_CMD_OPT_NO_MS_EXTENSIONS, QObject::tr("Don't accept MSVC non-standard syntax (-fno-ms-extentions)"), groupName, true, true, false, "-fno-ms-extensions");
    addOption(CC_CMD_OPT_WARNING_AS_ERROR, QObject::tr("Make all warnings into errors (-Werror)"), groupName, true, true, false, "-Werror");
    addOption(CC_CMD_OPT_ABORT_ON_ERROR , QObject::tr("Abort compilation on first error (-Wfatal-errors)"), groupName, true, true, false, "-Wfatal-errors");
    sl.clear();
    sl.append(CompilerOptionChoice("Normal","protector"));
    sl.append(CompilerOptionChoice("Explicit","protector-explicit"));
    sl.append(CompilerOptionChoice("Strong","protector-strong"));
    sl.append(CompilerOptionChoice("All","protector-all"));
    addOption(CC_CMD_OPT_STACK_PROTECTOR , QObject::tr("Check for stack smashing attacks (-fstack-protector)"), groupName, false, false, true, "-fstack-", CompilerOptionType::Choice, sl);
    sl.clear();
    sl.append(CompilerOptionChoice("Address","address"));
    sl.append(CompilerOptionChoice("Hwaddress","hwaddress"));
    sl.append(CompilerOptionChoice("Thread","thread"));
    sl.append(CompilerOptionChoice("Leak","leak"));
    sl.append(CompilerOptionChoice("Undefined","undefined"));
    addOption(CC_CMD_OPT_ADDRESS_SANITIZER , QObject::tr("Enable Sanitizer (-fsanitize=)"), groupName, true, true, true, "-fsanitize=",CompilerOptionType::Choice,sl);

    // Errors
    groupName = QObject::tr("Errors");
    addOption(CC_CMD_OPT_ERROR_RETURN_TYPE, QObject::tr("Function without return type (return-type)"), groupName, true, true, false, "-Werror=return-type");
    addOption(CC_CMD_OPT_ERROR_IMPLICIT_INT,QObject::tr("Declaration does not specify a type (implicit-int)"), groupName, true, false, false, "-Werror=implicit-int");
    addOption(CC_CMD_OPT_ERROR_VLA,QObject::tr("Variable lenght array (vla)"), groupName, true, true, false, "-Werror=vla");

    // Output
    //groupName = QObject::tr("Output");
    //addOption(CC_CMD_OPT_VERBOSE_ASM, QObject::tr("Put comments in generated assembly code (-fverbose-asm)"), groupName, true, true, false, "-fverbose-asm");
    //addOption(CC_CMD_OPT_ONLY_GEN_ASM_CODE, QObject::tr("Do not assemble, compile and generate the assemble code (-S)"), groupName, true, true, false, "-S");
    //addOption(CC_CMD_OPT_STOP_AFTER_PREPROCESSING, QObject::tr("Do not compile, stop after the preprocessing stage (-E)"), groupName, true, true, false, "-E");

    // Linker
    groupName = QObject::tr("Linker");
    addNumberOption(LINK_CMD_OPT_STACK_SIZE, QObject::tr("PE Stack Size"), groupName, false, false, true, "-Wl,--stack,","MB",1024*1024,0,0,99999)
        ->level = CompilerOptionLevel::Both;

    addOption(CC_CMD_OPT_USE_PIPE, QObject::tr("Use pipes instead of temporary files during compilation (-pipe)"), groupName, true, true, false, "-pipe")
        ->level = CompilerOptionLevel::Toolchain;
    //addOption(LINK_CMD_OPT_LINK_OBJC, QObject::tr("Link an Objective C program (-lobjc)"), groupName, false, false, true, "-lobjc");
    addOption(LINK_CMD_OPT_NO_LINK_STDLIB,QObject::tr("Do not use standard system libraries (-nostdlib)"), groupName, false, false, true, "-nostdlib")
        ->level = CompilerOptionLevel::Both;
    addOption(LINK_CMD_OPT_NO_CONSOLE, QObject::tr("Do not create a console window (-mwindows)"), groupName,false, false, true, "-mwindows")
        ->level = CompilerOptionLevel::Both;
    addOption(LINK_CMD_OPT_STRIP_EXE, QObject::tr("Strip executable (-s)"), groupName, false, false, true, "-s");
}

CompilerInfoManager::CompilerInfoManager()
{
    PCompilerInfo compilerInfo = std::make_shared<ClangCompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerType::Clang, compilerInfo);

    compilerInfo = std::make_shared<GCCCompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerType::GCC, compilerInfo);

#ifdef ENABLE_SDCC
    compilerInfo = std::make_shared<SDCCCompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerType::SDCC, compilerInfo);
#endif
}

PCompilerInfo CompilerInfoManager::getInfo(CompilerType compilerType)
{
    return getInstance()->mInfos.value(compilerType,PCompilerInfo());
}

bool CompilerInfoManager::hasCompilerOption(CompilerType compilerType, const QString &optKey)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->hasCompilerOption(optKey);
}

PCompilerOption CompilerInfoManager::getCompilerOption(CompilerType compilerType, const QString &optKey)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return PCompilerOption();
    return pInfo->getCompilerOption(optKey);
}

const QList<PCompilerOption> &CompilerInfoManager::getCompilerOptions(CompilerType compilerType)
{
    static QList<PCompilerOption> EmptyList;
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return EmptyList;
    return pInfo->compilerOptions();
}

bool CompilerInfoManager::supportStaticLink(CompilerType compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->supportStaticLink();
}

bool CompilerInfoManager::supportSyntaxCheck(CompilerType compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->supportSyntaxCheck();
}

PCompilerInfoManager CompilerInfoManager::instance;

PCompilerInfoManager CompilerInfoManager::getInstance()
{
    if (!instance) {
        instance = std::make_shared<CompilerInfoManager>();
    }
    return instance;
}

void CompilerInfoManager::addInfo(CompilerType compilerType, PCompilerInfo info)
{
    getInstance()->mInfos.insert(compilerType,info);
}

ClangCompilerInfo::ClangCompilerInfo():CompilerInfo(COMPILER_CLANG)
{
}

bool ClangCompilerInfo::supportStaticLink()
{
    return true;
}

GCCCompilerInfo::GCCCompilerInfo():CompilerInfo(COMPILER_GCC)
{
}

bool GCCCompilerInfo::supportStaticLink()
{
    return true;
}

#ifdef ENABLE_SDCC
SDCCCompilerInfo::SDCCCompilerInfo():CompilerInfo(COMPILER_SDCC)
{

}

bool SDCCCompilerInfo::supportStaticLink()
{
    return false;
}

bool SDCCCompilerInfo::supportSyntaxCheck()
{
    return false;
}

void SDCCCompilerInfo::prepareCompilerOptions()
{
    CompileOptionChoiceList sl;
    QString groupName;
//    // C options
//    groupName = QObject::tr("C options");
//    addOption(CC_CMD_OPT_ANSI, QObject::tr("Support all ANSI standard C programs (-ansi)"), groupName, true, true, false, "-ansi");
//    addOption(CC_CMD_OPT_NO_ASM, QObject::tr("Do not recognize asm,inline or typeof as a keyword (-fno-asm)"), groupName, true, true, false, "-fno-asm");
//    addOption(CC_CMD_OPT_TRADITIONAL_CPP, QObject::tr("Imitate traditional C preprocessors (-traditional-cpp)"), groupName, true, true, false, "-traditional-cpp");

    groupName = QObject::tr("Code Generation");
    // Optimization
    sl.clear();
    sl.append(CompilerOptionChoice("Intel MCS51","mcs51"));
    sl.append(CompilerOptionChoice("Dallas DS80C390","ds390"));
    sl.append(CompilerOptionChoice("Dallas DS80C400","ds400"));
    sl.append(CompilerOptionChoice("Freescale/Motorola HC08","hc08"));
    sl.append(CompilerOptionChoice("Freescale/Motorola S08","s08"));
    sl.append(CompilerOptionChoice("Zilog Z80","z80"));
    sl.append(CompilerOptionChoice("Zilog Z180","z180"));
    sl.append(CompilerOptionChoice("Rabbit 2000/3000","r2k"));
    sl.append(CompilerOptionChoice("Rabbit 3000","r3ka"));
    sl.append(CompilerOptionChoice("Sharp SM83","sm83"));
    sl.append(CompilerOptionChoice("Toshiba TLCS-90","tlcs90"));
    sl.append(CompilerOptionChoice("Zilog eZ80","ez80_z80"));
    sl.append(CompilerOptionChoice("STM8","stm8"));
    sl.append(CompilerOptionChoice("Padauk processors-13bit width memory","pdk13"));
    sl.append(CompilerOptionChoice("Padauk processors-14bit width memory","pdk14"));
    sl.append(CompilerOptionChoice("Padauk processors-15bit width memory","pdk15"));
    addOption(SDCC_CMD_OPT_PROCESSOR, QObject::tr("Processor (-m)"), groupName, true, false, false, "-m", CompilerOptionType::Choice,sl);

    // C++ Language Standards
    sl.clear();
    sl.append(CompilerOptionChoice("ANSI C89/ISO C90","c89"));
    sl.append(CompilerOptionChoice("ISO C99","c99"));
    sl.append(CompilerOptionChoice("ISO C11","c11"));
    sl.append(CompilerOptionChoice("ISO C17","c17"));
    sl.append(CompilerOptionChoice("ISO C2x","c2x"));
    sl.append(CompilerOptionChoice("SDCC C89","sdcc89"));
    sl.append(CompilerOptionChoice("SDCC C99","sdcc99"));
    sl.append(CompilerOptionChoice("SDCC C11","sdcc11"));
    sl.append(CompilerOptionChoice("SDCC C17","sdcc17"));
    sl.append(CompilerOptionChoice("SDCC C2x","sdcc2x"));
    addOption(SDCC_CMD_OPT_STD, QObject::tr("Language standard (--std)"), groupName, true, false, false, "--std-", CompilerOptionType::Choice,sl);

    // Memory Model
    sl.clear();
    sl.append(CompilerOptionChoice("Small","-small"));
    sl.append(CompilerOptionChoice("Medium","-medium"));
    sl.append(CompilerOptionChoice("Large","-large"));
    sl.append(CompilerOptionChoice("Huge","-huge"));
    addOption(SDCC_OPT_MEMORY_MODEL, QObject::tr("Memory model (--model)"), groupName, true, false, false, "--model", CompilerOptionType::Choice,sl);

    addOption(SDCC_OPT_XSTACK, QObject::tr("Use external stack"),groupName,true,false,false,"--xstack");
    addOption(SDCC_OPT_XRAM_MOVC, QObject::tr("Use movc instead of movx to read from external ram"),groupName,true,false,false,"--xram-movc");
    addOption(SDCC_OPT_ACALL_AJMP, QObject::tr("Replaces lcall/ljmp with acall/ajmp"),groupName,true,false,false,"--acall-ajmp");
    addOption(SDCC_OPT_NO_XINIT_OPT, QObject::tr("Don't memcpy initialized xram from code"),groupName,true,false,false,"--no-xinit-opt");
    addOption(SDCC_OPT_NOSTARTUP, QObject::tr("Don't generate startup code"),groupName,false,false,false,"nostartup");

    groupName = QObject::tr("MCU Specification");

    addOption(SDCC_OPT_IRAM_SIZE, QObject::tr("Internal ram size"), groupName, false, false, true, "--iram-size",CompilerOptionType::Input);
    addOption(SDCC_OPT_XRAM_LOC, QObject::tr("External ram start location"), groupName, false, false, true, "--xram-loc",CompilerOptionType::Input);
    addOption(SDCC_OPT_XRAM_SIZE, QObject::tr("External ram size"), groupName, false, false, true, "--xram-size",CompilerOptionType::Input);
    addOption(SDCC_OPT_STACK_LOC, QObject::tr("Stack pointer initial value"), groupName, false, false, true, "--stack-loc",CompilerOptionType::Input);
    addOption(SDCC_OPT_XSTACK_LOC, QObject::tr("External stack start location"), groupName, false, false, true, "--xstack-loc",CompilerOptionType::Input);
    addOption(SDCC_OPT_DATA_LOC, QObject::tr("Direct data start location"), groupName, false, false, true, "--data-loc",CompilerOptionType::Input);
    addOption(SDCC_OPT_CODE_LOC, QObject::tr("Code segment location"), groupName, false, false, true, "--code-loc",CompilerOptionType::Input);
    addOption(SDCC_OPT_CODE_SIZE, QObject::tr("Code segment size"), groupName, false, false, true, "--code-size",CompilerOptionType::Input);
}
#endif
