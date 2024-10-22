#include "Analyzer.hpp"
#include <Token.hpp>
#include <Tokenizer.hpp>
#include <gtest/gtest.h>
#include <Parser.hpp>
#include <AST.hpp>
#include <sys/types.h>
#include <IRGenerator.hpp>
#include <Compiler.hpp>

TEST(CompilerTest, compilerTest){
    LlvmIRGenerator llvmIRGenerator("test");
    Compiler compiler(llvmIRGenerator);

    compiler.compileToIR("testfile", "outputfile");
    compiler.buildExec("outputfile", "myprogram", Platform::LINUX);
}