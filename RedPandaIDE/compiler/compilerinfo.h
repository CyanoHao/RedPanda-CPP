#ifndef COMPILERINFO_H
#define COMPILERINFO_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <memory>

#include <qt_utils/enum.h>

#define COMPILER_CLANG "Clang"
#define COMPILER_GCC "GCC"
#define COMPILER_GCC_UTF8 "GCC_UTF8"
#define COMPILER_SDCC "SDCC"

#define C_CMD_OPT_STD "c_cmd_opt_std"

#define CC_CMD_OPT_ANSI "cc_cmd_opt_ansi"
#define CC_CMD_OPT_NO_ASM "cc_cmd_opt_no_asm"
#define CC_CMD_OPT_TRADITIONAL_CPP "cc_cmd_opt_traditional_cpp"

#define CC_CMD_OPT_ARCH "cc_cmd_opt_arch"
#define CC_CMD_OPT_TUNE "cc_cmd_opt_tune"
#define CC_CMD_OPT_INSTRUCTION "cc_cmd_opt_instruction"
#define CC_CMD_OPT_OPTIMIZE "cc_cmd_opt_optimize"
#define CC_CMD_OPT_POINTER_SIZE "cc_cmd_opt_pointer_size"
#define CC_CMD_OPT_STD "cc_cmd_opt_std"

#define CC_CMD_OPT_INHIBIT_ALL_WARNING "cc_cmd_opt_inhibit_all_warning"
#define CC_CMD_OPT_WARNING_ALL "cc_cmd_opt_warning_all"
#define CC_CMD_OPT_WARNING_EXTRA "cc_cmd_opt_warning_extra"
#define CC_CMD_OPT_CHECK_ISO_CONFORMANCE "cc_cmd_opt_check_iso_conformance"
#define CC_CMD_OPT_SYNTAX_ONLY "cc_cmd_opt_syntax_only"
#define CC_CMD_OPT_WARNING_AS_ERROR "cc_cmd_opt_warning_as_error"
#define CC_CMD_OPT_ABORT_ON_ERROR "cc_cmd_opt_abort_on_error"

#define CC_CMD_OPT_PROFILE_INFO "cc_cmd_opt_profile_info"

#define LINK_CMD_OPT_LINK_OBJC "link_cmd_opt_link_objc"
#define LINK_CMD_OPT_NO_LINK_STDLIB "link_cmd_opt_no_link_stdlib"
#define LINK_CMD_OPT_NO_CONSOLE "link_cmd_opt_no_console"
#define LINK_CMD_OPT_STRIP_EXE "link_cmd_opt_strip_exe"
#define LINK_CMD_OPT_STACK_SIZE "link_cmd_opt_stack_size"
#define CC_CMD_OPT_DEBUG_INFO "cc_cmd_opt_debug_info"
#define CC_CMD_OPT_ADDRESS_SANITIZER "cc_cmd_opt_address_sanitizer"
#define CC_CMD_OPT_STACK_PROTECTOR "cc_cmd_opt_stack_protector"

#define CC_CMD_OPT_VERBOSE_ASM "cc_cmd_opt_verbose_asm"
#define CC_CMD_OPT_ONLY_GEN_ASM_CODE "cc_cmd_opt_only_gen_asm_code"
#define CC_CMD_OPT_STOP_AFTER_PREPROCESSING "cc_cmd_opt_stop_after_preprocessing"
#define CC_CMD_OPT_USE_PIPE "cc_cmd_opt_use_pipe"

#define SDCC_CMD_OPT_PROCESSOR "sdcc_cmd_opt_processor"
#define SDCC_CMD_OPT_STD "sdcc_cmd_opt_std"
#define SDCC_OPT_MEMORY_MODEL "sdcc_opt_memory_model"
#define SDCC_OPT_XSTACK "sdcc_opt_xstack"
#define SDCC_OPT_XRAM_MOVC "sdcc_opt_xram_movc"
#define SDCC_OPT_ACALL_AJMP "sdcc_opt_acall_ajmp"
#define SDCC_OPT_NO_XINIT_OPT "sdcc_opt_no_xinit_opt"

#define SDCC_OPT_NOSTARTUP "sdcc_opt_nostartup"
#define SDCC_OPT_IRAM_SIZE "sdcc_opt_iram_size"
#define SDCC_OPT_XRAM_SIZE "sdcc_opt_xram_size"
#define SDCC_OPT_XRAM_LOC "sdcc_opt_xram_loc"
#define SDCC_OPT_XSTACK_LOC "sdcc_opt_xstack_loc"
#define SDCC_OPT_CODE_LOC "sdcc_opt_code_loc"
#define SDCC_OPT_CODE_SIZE "sdcc_opt_code_size"
#define SDCC_OPT_STACK_LOC "sdcc_opt_stack_loc"
#define SDCC_OPT_DATA_LOC "sdcc_opt_data_loc"
#define SDCC_OPT_NOSTARTUP "sdcc_opt_nostartup"

#define COMPILER_OPTION_ON "on"
#define COMPILER_OPTION_OFF ""

BETTER_ENUM(CompilerDriver, uint16_t,
    Unknown = 0,
    GCC = 0x0100,
    Clang = 0x0101,
    SDCC = 0x0200,
    MSVC = 0x0300,
    ClangCl = 0x0301)

enum class CompilerOptionType {
    Args,
    Boolean,
    Choice,
    Number,
    String,
};

class CompilerInfo;

struct CompilerOptionChoice {
    QString display;
    QString value;

    std::function<bool (const CompilerInfo &info)> availability;
    std::function<QString (const CompilerOptionChoice &self, const CompilerInfo &info)> filter;
};

struct CompilerOption {
    QString key;
    QString name; // "Generate debugging info"
    QString section; // "C options"

    CompilerOptionType type;

    // boolean option, e.g. "-g3"
    QString setting;

    // choice option, e.g. "-O1", "-O2", "-O3"
    QList<CompilerOptionChoice> choices;

    /* for spin control */
    int scale; //Scale
    QString suffix;  //suffix
    int defaultValue;
    int minValue;
    int maxValue;

    std::function<bool (const CompilerInfo &info)> availability;

public:
    void applyCFlags(const CompilerInfo &info, QStringList &args) const;
    void applyCxxFlags(const CompilerInfo &info, QStringList &args) const;
    void applyLdFlags(const CompilerInfo &info, QStringList &args) const;
};

using PCompilerOption = std::shared_ptr<CompilerOption>;

using CompilerOptionMap = QMap<QString, PCompilerOption>;

class CompilerInfo
{
public:
    CompilerInfo(const QString& name);
    CompilerInfo(const CompilerInfo&)=delete;
    CompilerInfo& operator=(const CompilerInfo&)=delete;

    const QList<PCompilerOption> &compilerOptions() const;
    const QString &name() const;
    PCompilerOption getCompilerOption(const QString& key) const;
    bool hasCompilerOption(const QString& key) const;
    void init();

    virtual bool supportConvertingCharset()=0;
#ifdef Q_OS_WIN
    virtual bool forceUTF8InDebugger()=0;
    virtual bool forceUTF8InMakefile()=0;
#else
    constexpr bool forceUTF8InDebugger() { return true; }
    constexpr bool forceUTF8InMakefile() { return true; }
#endif
    virtual bool supportStaticLink()=0;
    virtual bool supportSyntaxCheck();
protected:
    PCompilerOption addOption(const QString& key,
                   const QString& name,
                   const QString section,
                   bool isC,
                   bool isCpp,
                   bool isLinker,
                   const QString& setting,
                   CompilerOptionType type = CompilerOptionType::Checkbox,
                   const CompileOptionChoiceList& choices = CompileOptionChoiceList());
    PCompilerOption addNumberOption(const QString& key,
                   const QString& name,
                   const QString section,
                   bool isC,
                   bool isCpp,
                   bool isLinker,
                   const QString& setting,
                   const QString& suffix,
                   int scale,
                   int defaultValue,
                   int minValue,
                   int maxValue
                    );
    virtual void prepareCompilerOptions();
    void invalidateCompilerOptions();
protected:
    CompilerOptionMap mCompilerOptions;
    QList<PCompilerOption> mCompilerOptionList;
    QString mName;
private:
    bool mCompilerOptionValid;
};

using PCompilerInfo = std::shared_ptr<CompilerInfo>;

class CompilerInfoManager;
using PCompilerInfoManager = std::shared_ptr<CompilerInfoManager>;

class CompilerInfoManager {
public:
    CompilerInfoManager();
    static PCompilerInfo getInfo(CompilerDriver compilerType);
    static bool hasCompilerOption(CompilerDriver compilerType, const QString& optKey);
    static PCompilerOption getCompilerOption(CompilerDriver compilerType, const QString& optKey);
    static QList<PCompilerOption> getCompilerOptions(CompilerDriver compilerType);
    static bool supportCovertingCharset(CompilerDriver compilerType);
    static bool supportStaticLink(CompilerDriver compilerType);
    static bool supportSyntaxCheck(CompilerDriver compilerType);
    static bool forceUTF8InDebugger(CompilerDriver compilerType);
    static PCompilerInfoManager getInstance();
    static void addInfo(CompilerDriver compilerType, PCompilerInfo info);
private:
    static PCompilerInfoManager instance;
    QMap<CompilerDriver,PCompilerInfo> mInfos;
};

class CompilerInfoFamilyGcc: public CompilerInfo {
public:
    CompilerInfoFamilyGcc();
    bool supportConvertingCharset() override;
#ifdef Q_OS_WIN
    bool forceUTF8InDebugger() override;
    bool forceUTF8InMakefile() override;
#endif
    bool supportStaticLink() override;
};

class CompilerInfoFamilySdcc : public CompilerInfo{
public:
    CompilerInfoFamilySdcc();
    bool supportConvertingCharset() override;
#ifdef Q_OS_WIN
    bool forceUTF8InDebugger() override;
    bool forceUTF8InMakefile() override;
#endif
    bool supportStaticLink() override;
    bool supportSyntaxCheck() override;
protected:
    void prepareCompilerOptions() override;
};

class CompilerInfoFamilyMsvc : public CompilerInfo{
public:
    CompilerInfoFamilyMsvc();
    bool supportConvertingCharset() override;
#ifdef Q_OS_WIN
    bool forceUTF8InDebugger() override;
    bool forceUTF8InMakefile() override;
#endif
    bool supportStaticLink() override;
};

#endif // COMPILERINFO_H
