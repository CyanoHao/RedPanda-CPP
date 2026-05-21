// Ported from LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "llvm-triple.h"

#include <QString>
#include <cctype>

#include "llvm-util.hpp"

// LLVM 22.1.6: llvm/lib/TargetParser/Triple.cpp
namespace llvm
{
Triple::ArchType parseArch(QString ArchName)
{
    auto AT =
        StringSwitch<Triple::ArchType>(ArchName)
            .Cases({"i386", "i486", "i586", "i686"}, Triple::x86)
            // FIXME: Do we need to support these?
            .Cases({"i786", "i886", "i986"}, Triple::x86)
            .Cases({"amd64", "x86_64", "x86_64h"}, Triple::x86_64)
            .Cases({"powerpc", "powerpcspe", "ppc", "ppc32"}, Triple::ppc)
            .Cases({"powerpcle", "ppcle", "ppc32le"}, Triple::ppcle)
            .Cases({"powerpc64", "ppu", "ppc64"}, Triple::ppc64)
            .Cases({"powerpc64le", "ppc64le"}, Triple::ppc64le)
            .Case("xscale", Triple::arm)
            .Case("xscaleeb", Triple::armeb)
            .Case("aarch64", Triple::aarch64)
            .Case("aarch64_be", Triple::aarch64_be)
            .Case("aarch64_32", Triple::aarch64_32)
            .Case("aarch64_lfi", Triple::aarch64)
            .Case("arc", Triple::arc)
            .Case("arm64", Triple::aarch64)
            .Case("arm64_32", Triple::aarch64_32)
            .Case("arm64e", Triple::aarch64)
            .Case("arm64ec", Triple::aarch64)
            .Case("arm", Triple::arm)
            .Case("armeb", Triple::armeb)
            .Case("thumb", Triple::thumb)
            .Case("thumbeb", Triple::thumbeb)
            .Case("avr", Triple::avr)
            .Case("m68k", Triple::m68k)
            .Case("msp430", Triple::msp430)
            .Cases({"mips", "mipseb", "mipsallegrex", "mipsisa32r6", "mipsr6"},
                    Triple::mips)
            .Cases({"mipsel", "mipsallegrexel", "mipsisa32r6el", "mipsr6el"},
                    Triple::mipsel)
            .Cases({"mips64", "mips64eb", "mipsn32", "mipsisa64r6", "mips64r6",
                    "mipsn32r6"},
                    Triple::mips64)
            .Cases({"mips64el", "mipsn32el", "mipsisa64r6el", "mips64r6el",
                    "mipsn32r6el"},
                    Triple::mips64el)
            .Case("r600", Triple::r600)
            .Case("amdgcn", Triple::amdgcn)
            .Case("riscv32", Triple::riscv32)
            .Case("riscv64", Triple::riscv64)
            .Case("riscv32be", Triple::riscv32be)
            .Case("riscv64be", Triple::riscv64be)
            .Case("hexagon", Triple::hexagon)
            .Cases({"s390x", "systemz"}, Triple::systemz)
            .Case("sparc", Triple::sparc)
            .Case("sparcel", Triple::sparcel)
            .Cases({"sparcv9", "sparc64"}, Triple::sparcv9)
            .Case("tce", Triple::tce)
            .Case("tcele", Triple::tcele)
            .Case("xcore", Triple::xcore)
            .Case("nvptx", Triple::nvptx)
            .Case("nvptx64", Triple::nvptx64)
            .Case("amdil", Triple::amdil)
            .Case("amdil64", Triple::amdil64)
            .Case("hsail", Triple::hsail)
            .Case("hsail64", Triple::hsail64)
            .Case("spir", Triple::spir)
            .Case("spir64", Triple::spir64)
            .Cases({"spirv", "spirv1.5", "spirv1.6"}, Triple::spirv)
            .Cases({"spirv32", "spirv32v1.0", "spirv32v1.1", "spirv32v1.2",
                    "spirv32v1.3", "spirv32v1.4", "spirv32v1.5", "spirv32v1.6"},
                    Triple::spirv32)
            .Cases({"spirv64", "spirv64v1.0", "spirv64v1.1", "spirv64v1.2",
                    "spirv64v1.3", "spirv64v1.4", "spirv64v1.5", "spirv64v1.6"},
                    Triple::spirv64)
            .StartsWith("kalimba", Triple::kalimba)
            .Case("lanai", Triple::lanai)
            .Case("renderscript32", Triple::renderscript32)
            .Case("renderscript64", Triple::renderscript64)
            .Case("shave", Triple::shave)
            .Case("ve", Triple::ve)
            .Case("wasm32", Triple::wasm32)
            .Case("wasm64", Triple::wasm64)
            .Case("csky", Triple::csky)
            .Case("loongarch32", Triple::loongarch32)
            .Case("loongarch64", Triple::loongarch64)
            .Cases({"dxil", "dxilv1.0", "dxilv1.1", "dxilv1.2", "dxilv1.3",
                    "dxilv1.4", "dxilv1.5", "dxilv1.6", "dxilv1.7", "dxilv1.8",
                    "dxilv1.9"},
                    Triple::dxil)
            .Case("xtensa", Triple::xtensa)
            .Default(Triple::UnknownArch);

    // Some architectures require special parsing logic just to compute the
    // ArchType result.
    if (AT == Triple::UnknownArch) {
        // LLVM parseARMArch is too complicated.
        // Here we only check for some popular patterns:
        //   - Arch Linux: arm, armv7l
        //   - Arm GNU Toolchain: arm
        //   - Debian: arm
        //   - Fedora: arm
        //   - LLVM MinGW: armv7
        //   - openSUSE: arm
        if (ArchName.startsWith("armv5") || ArchName.startsWith("armv6") ||
            ArchName.startsWith("armv7"))
            return Triple::arm;
        if (ArchName.startsWith("bpf"))
            return parseBPFArch(ArchName);
    }

    return AT;
}

Triple::ArchType parseBPFArch(QString ArchName) {
    if (ArchName == "bpf") {
        if (sys::IsLittleEndianHost)
            return Triple::bpfel;
        else
            return Triple::bpfeb;
    } else if (ArchName == "bpf_be" || ArchName == "bpfeb") {
        return Triple::bpfeb;
    } else if (ArchName == "bpf_le" || ArchName == "bpfel") {
        return Triple::bpfel;
    } else {
        return Triple::UnknownArch;
    }
}
} // namespace llvm
