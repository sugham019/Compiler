#include "Compiler.hpp"
#include "IRGenerator.hpp"
#include <iostream>

constexpr Platform compilerTargetPlatform = Platform::LINUX;

int main(int argc, char* argv[]) {
    if(argc != 3){
        std::cerr << "Invalid number of arguments" << std::endl;
        return -1;
    }
    const std::string srcFilePath = argv[1];
    const std::string outputFilePath = argv[2];
    const std::string irPath = outputFilePath+".ll";

    LlvmIRGenerator llvmIRGenerator("main");
    Compiler compiler(llvmIRGenerator);

    compiler.compileToIR(srcFilePath, irPath);
    compiler.buildExec(irPath, outputFilePath, compilerTargetPlatform);
    return 0;
}
