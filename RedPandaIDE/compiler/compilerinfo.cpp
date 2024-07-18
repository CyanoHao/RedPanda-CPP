#include "compilerinfo.h"

#include <QObject>
#include <QDebug>
#include <QVersionNumber>

namespace CompilerOptionFilter
{
    QString asIs(const CompilerOptionChoice &self, const CompilerInfo &)
    {
        return self.value;
    }

    bool always(const CompilerInfo &)
    {
        return true;
    }

    template <int Major, int Minor = 0, int Patch = 0>
    bool gccAtLeast(const CompilerInfo &info)
    {
        return info.driver() == CompilerDriver::GCC &&
               info.version() >= QVersionNumber(Major, Minor, Patch);
    }

    template <int Major, int Minor, int Patch>
    bool clangAtLeast(const CompilerInfo &info)
    {
        return info.driver() == CompilerDriver::Clang &&
               info.version() >= QVersionNumber(Major, Minor, Patch);
    }

    template <int Major, int Minor = 0, int Patch = 0>
    bool msvcAtLeast(const CompilerInfo &info)
    {
        return info.driver() == CompilerDriver::MSVC &&
               info.version() >= QVersionNumber(Major, Minor, Patch);
    }

    template <int Major, int Minor = 0, int Patch = 0>
    bool clangClAtLeast(const CompilerInfo &info)
    {
        return info.driver() == CompilerDriver::ClangCl &&
               info.version() >= QVersionNumber(Major, Minor, Patch);
    }
}

CompilerOptionChoice::CompilerOptionChoice(const QString &display, const QString &value):
    display(display),
    value(value),
    availability(CompilerOptionFilter::always),
    filter(CompilerOptionFilter::asIs)
{
}

CompilerOptionChoice::CompilerOptionChoice(
    const QString &display,
    const QString &value,
    const std::function<bool (const CompilerInfo &info)> &availability
):
    display(display),
    value(value),
    availability(availability),
    filter(CompilerOptionFilter::asIs)
{
}

CompilerOptionChoice::CompilerOptionChoice(
    const QString &display,
    const QString &value,
    const std::function<bool (const CompilerInfo &info)> &availability,
    const std::function<QString (const CompilerOptionChoice &self, const CompilerInfo &info)> &filter
):
    display(display),
    value(value),
    availability(availability),
    filter(filter)
{
}

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

void gccCompilerOption()
{
    using Option = CompilerOptionWithFilter<CompilerInfoFamilyGcc>;
    using namespace CompilerOptionFilter;
    QList<Option> options = {
        // code generation
        Option::ChoiceOption(
            CC_CMD_OPT_OPTIMIZE,
            QObject::tr("Optimization level (-Ox)"),
            "-O",
            {
                {"-Og for debugging", "g", Or<GccAtLeast<4, 8>, ClangAtLeast<4>>{}},
                {"-O1 for optimizing", "1"},
                {"-Oz aggressively for size", "z", Or<GccAtLeast<12>, Clang>{}},
                {"-Os for size", "s"},
                {"-O2 for even more", "2"},
                {"-O3 for yet more", "3"},
                {"-Ofast against compliance", "fast", Or<GccAtLeast<4, 6>, Clang>{}},
            }
            // TODO: minimal clang version for `-Ofast` and `-Oz` are unsure (works in 3.3)
        ),
        Option::ChoiceOption(
            CC_CMD_OPT_STD,
            QObject::tr("C++ Language standard (-std)"),
            "-std=",
            {
                {"ISO C++98", "c++98"},
                {"ISO C++11", "c++0x", And<GccAtLeast<4, 3>, GccLessThan<4, 7>>{}},
                {"ISO C++11", "c++11", Or<GccAtLeast<4, 7>, Clang>{}},
                {"ISO C++14", "c++1y", And<GccAtLeast<4, 8>, GccLessThan<5>>{}},
                {"ISO C++14", "c++14", Or<GccAtLeast<5>, ClangAtLeast<3, 5>>{}},
                {"ISO C++17", "c++1z", Or<And<GccAtLeast<5>, GccLessThan<8>>, And<ClangAtLeast<3>, ClangLessThan<5>>>{}},
                {"ISO C++17", "c++17", Or<GccAtLeast<8>, ClangAtLeast<5>>{}},
                {"ISO C++20", "c++2a", Or<And<GccAtLeast<8>, GccLessThan<10>>, And<ClangAtLeast<5>, ClangLessThan<10>>>{}},
                {"ISO C++20", "c++20", Or<GccAtLeast<10>, ClangAtLeast<10>>{}},
                {"ISO C++23", "c++2b", And<ClangAtLeast<12>, ClangLessThan<17>>{}},
                {"ISO C++23", "c++23", Or<GccAtLeast<11>, ClangAtLeast<17>>{}},
                {"ISO C++26", "c++26", Or<GccAtLeast<14>, ClangAtLeast<17>>{}},
                {"GNU C++98", "gnu++98"},
                {"GNU C++11", "gnu++0x"},
                {"GNU C++11", "gnu++11"},
                {"GNU C++14", "gnu++1y"},
                {"GNU C++14", "gnu++14"},
                {"GNU C++17", "gnu++1z"},
                {"GNU C++17", "gnu++17"},
                {"GNU C++20", "gnu++2a"},
                {"GNU C++20", "gnu++20"},
                {"GNU C++23", "gnu++2b"},
                {"GNU C++23", "gnu++23"},
                {"GNU C++26", "gnu++2c"},
            },
            Or<GccAtLeast<3, 3>, Clang>{}
        ),
        Option::ChoiceOption(
            C_CMD_OPT_STD,
            QObject::tr("C Language standard (-std)"),
            "-std=",
            {
                // GCC option for C standard changed from `-fstd=c??` to `-std=c??` in 3.0.
                // We do not meant to support pre-3.0 versions so C90/C95/C99 are always truthy.
                {"ISO C90", "c90"},
                {"ISO C95", "iso9899:199409"},
                {"ISO C99", "c99"},
                {"ISO C11", "c1x", And<GccAtLeast<4, 6>, GccLessThan<4, 7>>{}},
                {"ISO C11", "c11", Or<GccAtLeast<4, 7>, Clang>{}},
                {"ISO C17", "c17", GccAtLeast<8>{}},
                {"ISO C23", "c2x", And<GccAtLeast<9>, GccLessThan<14>>{}},
                {"ISO C23", "c23", Or<GccAtLeast<14>, ClangAtLeast<18>>{}},
                {"GNU C90", "gnu90"},
                {"GNU C99", "gnu99"},
                {"GNU C11", "gnu1x"},
                {"GNU C11", "gnu11"},
                {"GNU C17", "gnu17"},
                {"GNU C23", "gnu2x"},
                {"GNU C23", "gnu23"},
            }
        ),
        Option::ChoiceOption(
            CC_CMD_OPT_ARCH,
            QObject::tr("µ-arch level (-march=)"),
            "-march=",
            {
                {"x86-64", "x86-64"},
                {"x86-64-v2 (SSE4.2)", "x86-64-v2"},
                {"x86-64-v3 (AVX2)", "x86-64-v3"},
                {"x86-64-v4 (AVX-512)", "x86-64-v4"},
                {"armv8-a", "armv8-a"},
                {"rv64g", "rv64g"},
                {"rv64gc", "rv64gc"},
                {"rv64gcv", "rv64gcv"},
            }
        ),
        Option::ChoiceOption(
            CC_CMD_OPT_POINTER_SIZE,
            QObject::tr("x86 multilib (-mx)"),
            "-m",
            {
                {"-m32", "32"},
                {"-m64", "64"},
                {"-mx32", "x32"},
            }
        ),
        Option::ChoiceOption(
            CC_CMD_OPT_POINTER_SIZE,
            QObject::tr("MIPS ABI (-mabi=)"),
            "-mabi=",
            {
                {"32", "32"},
                {"n32", "n32"},
                {"64", "64"},
            }
            // there are also abi options for aarch64, loongarch and riscv, but not actually used.
            // TODO: investigate ilp32 on apple OSes (usage is reported on watchOS)
        ),
    };
}

void msvcCompilerOption()
{
    using Option = CompilerOptionWithFilter<CompilerInfoFamilyMsvc>;
    QList<Option> options = {};
}

void CompilerInfo::prepareCompilerOptions()
{
    QList<QPair<QString,QString>> sl;
    QString groupName;
//    // C options
//    groupName = QObject::tr("C options");
//    addOption(CC_CMD_OPT_ANSI, QObject::tr("Support all ANSI standard C programs (-ansi)"), groupName, true, true, false, "-ansi");
//    addOption(CC_CMD_OPT_NO_ASM, QObject::tr("Do not recognize asm,inline or typeof as a keyword (-fno-asm)"), groupName, true, true, false, "-fno-asm");
//    addOption(CC_CMD_OPT_TRADITIONAL_CPP, QObject::tr("Imitate traditional C preprocessors (-traditional-cpp)"), groupName, true, true, false, "-traditional-cpp");

    groupName = QObject::tr("Code Generation");
    // Optimization
    sl.clear();
    sl.append(QPair<QString,QString>("Low (-O1)","1"));
    sl.append(QPair<QString,QString>("Med (-O2)","2"));
    sl.append(QPair<QString,QString>("High (-O3)","3"));
    sl.append(QPair<QString,QString>("Highest (-Ofast)","fast"));
    sl.append(QPair<QString,QString>("Size (-Os)","s"));
    sl.append(QPair<QString,QString>("Debug (-Og)","g"));
    addOption(CC_CMD_OPT_OPTIMIZE, QObject::tr("Optimization level (-Ox)"), groupName, true, true, false, "-O", CompilerOptionType::Choice, sl);

    // C++ Language Standards
    sl.clear();
    sl.append(QPair<QString,QString>("ISO C++","c++98"));
    sl.append(QPair<QString,QString>("ISO C++11","c++11"));
    sl.append(QPair<QString,QString>("ISO C++14","c++14"));
    sl.append(QPair<QString,QString>("ISO C++17","c++17"));
    sl.append(QPair<QString,QString>("ISO C++20","c++2a"));
    sl.append(QPair<QString,QString>("ISO C++23","c++2b"));
    sl.append(QPair<QString,QString>("GNU C++","gnu++98"));
    sl.append(QPair<QString,QString>("GNU C++11","gnu++11"));
    sl.append(QPair<QString,QString>("GNU C++14","gnu++14"));
    sl.append(QPair<QString,QString>("GNU C++17","gnu++17"));
    sl.append(QPair<QString,QString>("GNU C++20","gnu++2a"));
    sl.append(QPair<QString,QString>("GNU C++23","gnu++2b"));
    addOption(CC_CMD_OPT_STD, QObject::tr("C++ Language standard (-std)"), groupName, false, true, false, "-std=",CompilerOptionType::Choice, sl);

    sl.clear();
    sl.append(QPair<QString,QString>("ISO C90","c90"));
    sl.append(QPair<QString,QString>("ISO C99","c99"));
    sl.append(QPair<QString,QString>("ISO C11","c11"));
    sl.append(QPair<QString,QString>("ISO C17","c17"));
    sl.append(QPair<QString,QString>("GNU C90","gnu90"));
    sl.append(QPair<QString,QString>("GNU C99","gnu99"));
    sl.append(QPair<QString,QString>("GNU C11","gnu11"));
    sl.append(QPair<QString,QString>("GNU C17","gnu17"));
    addOption(C_CMD_OPT_STD, QObject::tr("C Language standard (-std)"), groupName, true, false, false, "-std=", CompilerOptionType::Choice, sl);

    // Optimization for cpu type
//    sl.clear();
//    sl.append(QPair<QString,QString>(QObject::tr("This CPU"),"native"));
//    sl.append(QPair<QString,QString>("i386","i386"));
//    sl.append(QPair<QString,QString>("i486","i486"));
//    sl.append(QPair<QString,QString>("i586","i586"));
//    sl.append(QPair<QString,QString>("i686","i686"));
//    sl.append(QPair<QString,QString>("Pentium","pentium"));
//    sl.append(QPair<QString,QString>("Pentium MMX","pentium-mmx"));
//    sl.append(QPair<QString,QString>("Pentium Pro","pentiumpro"));
//    sl.append(QPair<QString,QString>("Pentium 2","pentium2"));
//    sl.append(QPair<QString,QString>("Pentium 3","pentium3"));
//    sl.append(QPair<QString,QString>("Pentium 4","pentium4"));
//    sl.append(QPair<QString,QString>("Conroe","core2"));
//    sl.append(QPair<QString,QString>("Nehalem","corei7"));
//    sl.append(QPair<QString,QString>("Sandy","corei7-avx"));
//    sl.append(QPair<QString,QString>("K6","k6"));
//    sl.append(QPair<QString,QString>("K6-2","k6-2"));
//    sl.append(QPair<QString,QString>("K6-3","k6-3"));
//    sl.append(QPair<QString,QString>("Athlon","athlon"));
//    sl.append(QPair<QString,QString>("Athlon Tbird","athlon-tbird"));
//    sl.append(QPair<QString,QString>("Athlon 4","athlon-4"));
//    sl.append(QPair<QString,QString>("Athlon XP","athlon-xp"));
//    sl.append(QPair<QString,QString>("Athlon MP","athlon-mp"));
//    sl.append(QPair<QString,QString>("K8","k8"));
//    sl.append(QPair<QString,QString>("K8 Rev.E","k8-sse3"));
//    sl.append(QPair<QString,QString>("K10","barcelona"));
//    sl.append(QPair<QString,QString>("Bulldozer","bdver1"));
//    addOption(CC_CMD_OPT_ARCH, QObject::tr("Optimize for the following machine (-march)"), groupName, true, true, false, "-march=", sl);
//    addOption(CC_CMD_OPT_TUNE, QObject::tr("Optimize less, while maintaining full compatibility (-tune)"), groupName, true, true, false, "-mtune=", sl);

    // Enable use of the specific instructions
    sl.clear();
    sl.append(QPair<QString,QString>("MMX","mmx"));
    sl.append(QPair<QString,QString>("3D Now","3dnow"));
    sl.append(QPair<QString,QString>("SSE","sse"));
    sl.append(QPair<QString,QString>("SSE2","sse2"));
    sl.append(QPair<QString,QString>("SSE3","sse3"));
    sl.append(QPair<QString,QString>("SSSE3","ssse3"));
    sl.append(QPair<QString,QString>("SSE4","sse4"));
    sl.append(QPair<QString,QString>("SSE4A","sse4a"));
    sl.append(QPair<QString,QString>("SSE4.1","sse4.1"));
    sl.append(QPair<QString,QString>("SSE4.2","sse4.2"));
    sl.append(QPair<QString,QString>("AVX","avx"));
    sl.append(QPair<QString,QString>("AVX2","avx2"));
    sl.append(QPair<QString,QString>("FMA4","fma4"));
    sl.append(QPair<QString,QString>("XOP","xop"));
    sl.append(QPair<QString,QString>("AES","aes"));
    addOption(CC_CMD_OPT_INSTRUCTION,QObject::tr("Enable use of specific instructions (-mx)"), groupName, true, true, false, "-m", CompilerOptionType::Choice, sl);

    // 32bit/64bit
    sl.clear();
    sl.append(QPair<QString,QString>("32bit","32"));
    sl.append(QPair<QString,QString>("64bit","64"));
    addOption(CC_CMD_OPT_POINTER_SIZE, QObject::tr("Compile with the following pointer size (-mx)"), groupName, true, true, true, "-m", CompilerOptionType::Choice, sl);

    addOption(CC_CMD_OPT_DEBUG_INFO, QObject::tr("Generate debugging information (-g3)"), groupName, true, true, false, "-g3");
    addOption(CC_CMD_OPT_PROFILE_INFO, QObject::tr("Generate profiling info for analysis (-pg)"), groupName, true, true, true, "-pg");
    addOption(CC_CMD_OPT_SYNTAX_ONLY, QObject::tr("Only check the code for syntax errors (-fsyntax-only)"), groupName, true, true, false, "-fsyntax-only");

    // Warnings
    groupName = QObject::tr("Warnings");
    addOption(CC_CMD_OPT_INHIBIT_ALL_WARNING, QObject::tr("Inhibit all warning messages (-w)"), groupName, true, true, false, "-w");
    addOption(CC_CMD_OPT_WARNING_ALL,QObject::tr("Show most warnings (-Wall)"), groupName, true, true, false, "-Wall");
    addOption(CC_CMD_OPT_WARNING_EXTRA,QObject::tr("Show some more warnings (-Wextra)"), groupName, true, true, false, "-Wextra");
    addOption(CC_CMD_OPT_CHECK_ISO_CONFORMANCE, QObject::tr("Check ISO C/C++ conformance (-pedantic)"), groupName, true, true, false, "-pedantic");
    addOption(CC_CMD_OPT_WARNING_AS_ERROR, QObject::tr("Make all warnings into errors (-Werror)"), groupName, true, true, false, "-Werror");
    addOption(CC_CMD_OPT_ABORT_ON_ERROR , QObject::tr("Abort compilation on first error (-Wfatal-errors)"), groupName, true, true, false, "-Wfatal-errors");
    sl.clear();
    sl.append(QPair<QString,QString>("Normal","protector"));
    sl.append(QPair<QString,QString>("Explicit","protector-explicit"));
    sl.append(QPair<QString,QString>("Strong","protector-strong"));
    sl.append(QPair<QString,QString>("All","protector-all"));
    addOption(CC_CMD_OPT_STACK_PROTECTOR , QObject::tr("Check for stack smashing attacks (-fstack-protector)"), groupName, false, false, true, "-fstack-", CompilerOptionType::Choice, sl);
    sl.clear();
    sl.append(QPair<QString,QString>("Address","address"));
    sl.append(QPair<QString,QString>("Hwaddress","hwaddress"));
    sl.append(QPair<QString,QString>("Thread","thread"));
    sl.append(QPair<QString,QString>("Leak","leak"));
    sl.append(QPair<QString,QString>("Undefined","undefined"));
    addOption(CC_CMD_OPT_ADDRESS_SANITIZER , QObject::tr("Enable Sanitizer (-fsanitize=)"), groupName, true, true, true, "-fsanitize=",CompilerOptionType::Choice,sl);

    // Output
    //groupName = QObject::tr("Output");
    //addOption(CC_CMD_OPT_VERBOSE_ASM, QObject::tr("Put comments in generated assembly code (-fverbose-asm)"), groupName, true, true, false, "-fverbose-asm");
    //addOption(CC_CMD_OPT_ONLY_GEN_ASM_CODE, QObject::tr("Do not assemble, compile and generate the assemble code (-S)"), groupName, true, true, false, "-S");
    //addOption(CC_CMD_OPT_STOP_AFTER_PREPROCESSING, QObject::tr("Do not compile, stop after the preprocessing stage (-E)"), groupName, true, true, false, "-E");

    // Linker
    groupName = QObject::tr("Linker");
#ifdef Q_OS_WIN
    addNumberOption(LINK_CMD_OPT_STACK_SIZE, QObject::tr("Stack Size"), groupName, false, false, true, "-Wl,--stack,","MB",1024*1024,12,0,99999);
#endif

    addOption(CC_CMD_OPT_USE_PIPE, QObject::tr("Use pipes instead of temporary files during compilation (-pipe)"), groupName, true, true, false, "-pipe");
    //addOption(LINK_CMD_OPT_LINK_OBJC, QObject::tr("Link an Objective C program (-lobjc)"), groupName, false, false, true, "-lobjc");
    addOption(LINK_CMD_OPT_NO_LINK_STDLIB,QObject::tr("Do not use standard system libraries (-nostdlib)"), groupName, false, false, true, "-nostdlib");
    addOption(LINK_CMD_OPT_NO_CONSOLE, QObject::tr("Do not create a console window (-mwindows)"), groupName,false, false, true, "-mwindows");
    addOption(LINK_CMD_OPT_STRIP_EXE, QObject::tr("Strip executable (-s)"), groupName, false, false, true, "-s");
}

CompilerInfoManager::CompilerInfoManager()
{
    PCompilerInfo compilerInfo = std::make_shared<ClangCompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerDriver::Clang, compilerInfo);

    compilerInfo = std::make_shared<GCCCompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerDriver::GCC, compilerInfo);

    compilerInfo = std::make_shared<GCCUTF8CompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerType::GCC_UTF8, compilerInfo);

    compilerInfo = std::make_shared<SDCCCompilerInfo>();
    compilerInfo->init();
    mInfos.insert(CompilerType::SDCC, compilerInfo);
}

PCompilerInfo CompilerInfoManager::getInfo(CompilerDriver compilerType)
{
    return getInstance()->mInfos.value(compilerType,PCompilerInfo());
}

bool CompilerInfoManager::hasCompilerOption(CompilerDriver compilerType, const QString &optKey)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->hasCompilerOption(optKey);
}

PCompilerOption CompilerInfoManager::getCompilerOption(CompilerDriver compilerType, const QString &optKey)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return PCompilerOption();
    return pInfo->getCompilerOption(optKey);
}

QList<PCompilerOption> CompilerInfoManager::getCompilerOptions(CompilerDriver compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return QList<PCompilerOption>();
    return pInfo->compilerOptions();
}

bool CompilerInfoManager::supportCovertingCharset(CompilerDriver compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->supportConvertingCharset();
}

bool CompilerInfoManager::supportStaticLink(CompilerDriver compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->supportStaticLink();
}

bool CompilerInfoManager::supportSyntaxCheck(CompilerDriver compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->supportSyntaxCheck();
}

bool CompilerInfoManager::forceUTF8InDebugger(CompilerDriver compilerType)
{
    PCompilerInfo pInfo = getInfo(compilerType);
    if (!pInfo)
        return false;
    return pInfo->forceUTF8InDebugger();
}

PCompilerInfoManager CompilerInfoManager::instance;

PCompilerInfoManager CompilerInfoManager::getInstance()
{
    if (!instance) {
        instance = std::make_shared<CompilerInfoManager>();
    }
    return instance;
}

void CompilerInfoManager::addInfo(CompilerDriver compilerType, PCompilerInfo info)
{
    getInstance()->mInfos.insert(compilerType,info);
}

ClangCompilerInfo::ClangCompilerInfo():CompilerInfo(COMPILER_CLANG)
{
}

bool ClangCompilerInfo::supportConvertingCharset()
{
    return false;
}

bool ClangCompilerInfo::forceUTF8InDebugger()
{
    return true;
}

bool ClangCompilerInfo::forceUTF8InMakefile()
{
    return false;
}

bool ClangCompilerInfo::supportStaticLink()
{
    return true;
}

GCCCompilerInfo::GCCCompilerInfo():CompilerInfo(COMPILER_GCC)
{
}

bool GCCCompilerInfo::supportConvertingCharset()
{
    return true;
}

bool GCCCompilerInfo::forceUTF8InDebugger()
{
    return false;
}

bool GCCCompilerInfo::forceUTF8InMakefile()
{
    return false;
}

bool GCCCompilerInfo::supportStaticLink()
{
    return true;
}

GCCUTF8CompilerInfo::GCCUTF8CompilerInfo():CompilerInfo(COMPILER_GCC_UTF8)
{
}

bool GCCUTF8CompilerInfo::supportConvertingCharset()
{
    return true;
}

bool GCCUTF8CompilerInfo::forceUTF8InDebugger()
{
    return true;
}

bool GCCUTF8CompilerInfo::forceUTF8InMakefile()
{
    return true;
}

bool GCCUTF8CompilerInfo::supportStaticLink()
{
    return true;
}

SDCCCompilerInfo::SDCCCompilerInfo():CompilerInfo(COMPILER_SDCC)
{

}

bool SDCCCompilerInfo::supportConvertingCharset()
{
    return false;
}

bool SDCCCompilerInfo::forceUTF8InDebugger()
{
    return false;
}

bool SDCCCompilerInfo::forceUTF8InMakefile()
{
    return false;
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
    QList<QPair<QString,QString>> sl;
    QString groupName;
//    // C options
//    groupName = QObject::tr("C options");
//    addOption(CC_CMD_OPT_ANSI, QObject::tr("Support all ANSI standard C programs (-ansi)"), groupName, true, true, false, "-ansi");
//    addOption(CC_CMD_OPT_NO_ASM, QObject::tr("Do not recognize asm,inline or typeof as a keyword (-fno-asm)"), groupName, true, true, false, "-fno-asm");
//    addOption(CC_CMD_OPT_TRADITIONAL_CPP, QObject::tr("Imitate traditional C preprocessors (-traditional-cpp)"), groupName, true, true, false, "-traditional-cpp");

    groupName = QObject::tr("Code Generation");
    // Optimization
    sl.clear();
    sl.append(QPair<QString,QString>("Intel MCS51","mcs51"));
    sl.append(QPair<QString,QString>("Dallas DS80C390","ds390"));
    sl.append(QPair<QString,QString>("Dallas DS80C400","ds400"));
    sl.append(QPair<QString,QString>("Freescale/Motorola HC08","hc08"));
    sl.append(QPair<QString,QString>("Freescale/Motorola S08","s08"));
    sl.append(QPair<QString,QString>("Zilog Z80","z80"));
    sl.append(QPair<QString,QString>("Zilog Z180","z180"));
    sl.append(QPair<QString,QString>("Rabbit 2000/3000","r2k"));
    sl.append(QPair<QString,QString>("Rabbit 3000","r3ka"));
    sl.append(QPair<QString,QString>("Sharp SM83","sm83"));
    sl.append(QPair<QString,QString>("Toshiba TLCS-90","tlcs90"));
    sl.append(QPair<QString,QString>("Zilog eZ80","ez80_z80"));
    sl.append(QPair<QString,QString>("STM8","stm8"));
    sl.append(QPair<QString,QString>("Padauk processors-13bit width memory","pdk13"));
    sl.append(QPair<QString,QString>("Padauk processors-14bit width memory","pdk14"));
    sl.append(QPair<QString,QString>("Padauk processors-15bit width memory","pdk15"));
    addOption(SDCC_CMD_OPT_PROCESSOR, QObject::tr("Processor (-m)"), groupName, true, false, false, "-m", CompilerOptionType::Choice,sl);

    // C++ Language Standards
    sl.clear();
    sl.append(QPair<QString,QString>("ANSI C89/ISO C90","c89"));
    sl.append(QPair<QString,QString>("ISO C99","c99"));
    sl.append(QPair<QString,QString>("ISO C11","c11"));
    sl.append(QPair<QString,QString>("ISO C17","c17"));
    sl.append(QPair<QString,QString>("ISO C2x","c2x"));
    sl.append(QPair<QString,QString>("SDCC C89","sdcc89"));
    sl.append(QPair<QString,QString>("SDCC C99","sdcc99"));
    sl.append(QPair<QString,QString>("SDCC C11","sdcc11"));
    sl.append(QPair<QString,QString>("SDCC C17","sdcc17"));
    sl.append(QPair<QString,QString>("SDCC C2x","sdcc2x"));
    addOption(SDCC_CMD_OPT_STD, QObject::tr("Language standard (--std)"), groupName, true, false, false, "--std-", CompilerOptionType::Choice,sl);

    // Memory Model
    sl.clear();
    sl.append(QPair<QString,QString>("Small","-small"));
    sl.append(QPair<QString,QString>("Medium","-medium"));
    sl.append(QPair<QString,QString>("Large","-large"));
    sl.append(QPair<QString,QString>("Huge","-huge"));
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
