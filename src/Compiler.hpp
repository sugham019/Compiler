#pragma once

#include "IRGenerator.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <list>

enum class Platform{
    WIN,
    LINUX
};

class Compiler{
    
public:
    Compiler(IRGenerator& irGenerator);
    void compileToIR(const std::filesystem::path& srcFilepath, const std::filesystem::path& outputIrFilepath);
    void buildExec(const std::string& irFilePath, const std::string& outputfile,  Platform platform);

private:
    void performIRGeneration(const ast::File& file, const std::string& filename);
    ast::File generateAST(const std::filesystem::path& srcFilepath);

    const std::string commonLibs = "";
    const std::string linuxLibs = "libstdlinux.a";
    const std::string winLibs = "";

    IRGenerator& m_irGenerator;
    std::list<std::string> m_files;
};