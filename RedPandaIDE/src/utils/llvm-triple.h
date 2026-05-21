// Ported from LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef UTILS_LLVM_TRIPLE_H
#define UTILS_LLVM_TRIPLE_H

#include <QString>

// LLVM 22.1.6: llvm/include/llvm/TargetParser/Triple.h
namespace llvm
{
class Triple
{
public:
    enum ArchType
    {
        UnknownArch,

        arm,         // ARM (little endian): arm, armv.*, xscale
        armeb,       // ARM (big endian): armeb
        aarch64,     // AArch64 (little endian): aarch64
        aarch64_be,  // AArch64 (big endian): aarch64_be
        aarch64_32,  // AArch64 (little endian) ILP32: aarch64_32
        arc,         // ARC: Synopsys ARC
        avr,         // AVR: Atmel AVR microcontroller
        bpfel,       // eBPF or extended BPF or 64-bit BPF (little endian)
        bpfeb,       // eBPF or extended BPF or 64-bit BPF (big endian)
        csky,        // CSKY: csky
        dxil,        // DXIL 32-bit DirectX bytecode
        hexagon,     // Hexagon: hexagon
        loongarch32, // LoongArch (32-bit): loongarch32
        loongarch64, // LoongArch (64-bit): loongarch64
        m68k,        // M68k: Motorola 680x0 family
        mips,        // MIPS: mips, mipsallegrex, mipsr6
        mipsel,      // MIPSEL: mipsel, mipsallegrexe, mipsr6el
        mips64,      // MIPS64: mips64, mips64r6, mipsn32, mipsn32r6
        mips64el,    // MIPS64EL: mips64el, mips64r6el, mipsn32el, mipsn32r6el
        msp430,      // MSP430: msp430
        ppc,         // PPC: powerpc
        ppcle,       // PPCLE: powerpc (little endian)
        ppc64,       // PPC64: powerpc64, ppu
        ppc64le,     // PPC64LE: powerpc64le
        r600,        // R600: AMD GPUs HD2XXX - HD6XXX
        amdgcn,      // AMDGCN: AMD GCN GPUs
        riscv32,     // RISC-V (32-bit, little endian): riscv32
        riscv64,     // RISC-V (64-bit, little endian): riscv64
        riscv32be,   // RISC-V (32-bit, big endian): riscv32be
        riscv64be,   // RISC-V (64-bit, big endian): riscv64be
        sparc,       // Sparc: sparc
        sparcv9,     // Sparcv9: Sparcv9
        sparcel,     // Sparc: (endianness = little). NB: 'Sparcle' is a CPU variant
        systemz,     // SystemZ: s390x
        tce,         // TCE (http://tce.cs.tut.fi/): tce
        tcele,       // TCE little endian (http://tce.cs.tut.fi/): tcele
        thumb,       // Thumb (little endian): thumb, thumbv.*
        thumbeb,     // Thumb (big endian): thumbeb
        x86,         // X86: i[3-9]86
        x86_64,      // X86-64: amd64, x86_64
        xcore,       // XCore: xcore
        xtensa,      // Tensilica: Xtensa
        nvptx,       // NVPTX: 32-bit
        nvptx64,     // NVPTX: 64-bit
        amdil,       // AMDIL
        amdil64,     // AMDIL with 64-bit pointers
        hsail,       // AMD HSAIL
        hsail64,     // AMD HSAIL with 64-bit pointers
        spir,        // SPIR: standard portable IR for OpenCL 32-bit version
        spir64,      // SPIR: standard portable IR for OpenCL 64-bit version
        spirv,       // SPIR-V with logical memory layout.
        spirv32,     // SPIR-V with 32-bit pointers
        spirv64,     // SPIR-V with 64-bit pointers
        kalimba,     // Kalimba: generic kalimba
        shave,       // SHAVE: Movidius vector VLIW processors
        lanai,       // Lanai: Lanai 32-bit
        wasm32,      // WebAssembly with 32-bit pointers
        wasm64,      // WebAssembly with 64-bit pointers
        renderscript32, // 32-bit RenderScript
        renderscript64, // 64-bit RenderScript
        ve,             // NEC SX-Aurora Vector Engine
        LastArchType = ve
    };
};

Triple::ArchType parseArch(QString ArchName);
Triple::ArchType parseBPFArch(QString ArchName);
} // namespace llvm

#endif
