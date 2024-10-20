#pragma once

#include "IRGenerator.hpp"
#include <filesystem>
#include <string>
#include <list>

class Compiler{
    
public:
    Compiler(IRGenerator& irGenerator);
    void compile(const std::filesystem::path& srcFilepath, const std::filesystem::path& outputFilepath);

private:
    void performIRGeneration(const ast::File& file, const std::string& filename);
    ast::File generateAST(const std::filesystem::path& srcFilepath);

    IRGenerator& m_irGenerator;
    std::list<std::string> m_files;
};