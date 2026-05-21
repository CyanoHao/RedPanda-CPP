#ifndef COMPILATIONSTAGE_H
#define COMPILATIONSTAGE_H

enum class CompilationStage {
    PreprocessingOnly,
    CompilationProperOnly,
    AssemblingOnly,
    GenerateExecutable,
    GenerateGimple,
};

#endif
